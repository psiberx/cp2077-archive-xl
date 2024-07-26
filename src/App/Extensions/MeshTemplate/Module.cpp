#include "Module.hpp"
#include "App/Extensions/GarmentOverride/Module.hpp"
#include "App/Utils/Str.hpp"

namespace
{
constexpr auto ModuleName = "MeshTemplate";

constexpr auto SpecialMaterialMarker = '@';
constexpr auto ContextMaterialName = Red::CName("@context");
constexpr auto DefaultTemplateName = Red::CName("@material");
constexpr auto DefaultAppearanceName = Red::CName("default");
constexpr auto MaterialAttr = Red::CName("material");
}

std::string_view App::MeshTemplateModule::GetName()
{
    return ModuleName;
}

bool App::MeshTemplateModule::Load()
{
    HookAfter<Raw::CMesh::FindAppearance>(&OnFindAppearance).OrThrow();
    HookAfter<Raw::CMesh::LoadMaterialsAsync>(&OnLoadMaterials).OrThrow();
    Hook<Raw::CMesh::AddStubAppearance>(&OnAddStubAppearance).OrThrow();
    HookAfter<Raw::CMesh::ShouldPreloadAppearances>(&OnPreloadAppearances).OrThrow();

    s_dummyMaterial = Red::MakeHandle<Red::CMaterialInstance>();

    return true;
}

bool App::MeshTemplateModule::Unload()
{
    {
        std::unique_lock _(s_stateLock);
        s_states.clear();
    }

    Unhook<Raw::CMesh::FindAppearance>();
    Unhook<Raw::CMesh::LoadMaterialsAsync>();
    Unhook<Raw::CMesh::AddStubAppearance>();
    Unhook<Raw::CMesh::ShouldPreloadAppearances>();

    return true;
}

void App::MeshTemplateModule::OnFindAppearance(Red::Handle<Red::meshMeshAppearance>& aOut, Red::CMesh* aMesh,
                                               Red::CName aName)
{
    if (!aOut)
        return;

    const auto ownerMesh = Raw::MeshAppearance::Owner::Ptr(aOut);

    if (!ownerMesh)
    {
        auto& controller = GarmentOverrideModule::GetDynamicAppearanceController();
        auto pathStr = controller->GetPathString(aMesh->path);

        if (!pathStr.empty())
        {
            LogWarning(R"(|{}| Mesh \"{}\" doesn't have any appearances.)", ModuleName, pathStr);
        }
        else
        {
            LogWarning(R"(|{}| Mesh {} doesn't have any appearances.)", ModuleName, aMesh->path.hash);
        }

        aOut = Red::MakeHandle<Red::meshMeshAppearance>();
        Raw::MeshAppearance::Owner::Set(aOut, aMesh);
        aMesh->appearances.PushBack(aOut);
        return;
    }

    if (!aOut->name || aMesh->appearances.size == 0)
        return;

    if (aOut->chunkMaterials.size == 0)
    {
        auto sourceAppearance = aMesh->appearances[0];

        if (sourceAppearance && sourceAppearance != aOut)
        {
            const auto appearanceNameStr = std::string{aOut->name.ToString()};
            for (auto chunkMaterialName : sourceAppearance->chunkMaterials)
            {
                auto chunkMaterialNameStr = std::string_view{chunkMaterialName.ToString()};
                auto templateNamePos = chunkMaterialNameStr.find(SpecialMaterialMarker);

                if (templateNamePos != std::string_view::npos)
                {
                    chunkMaterialNameStr.remove_prefix(templateNamePos);

                    auto generatedMaterialNameStr = appearanceNameStr;
                    generatedMaterialNameStr.append(chunkMaterialNameStr);

                    chunkMaterialName = Red::CNamePool::Add(generatedMaterialNameStr.data());
                }
                else if (chunkMaterialName == sourceAppearance->name)
                {
                    chunkMaterialName = aOut->name;
                }

                aOut->chunkMaterials.PushBack(chunkMaterialName);
            }
        }
    }

    if (ownerMesh != aMesh)
    {
        auto meshState = AcquireMeshState(aMesh);

        {
            std::unique_lock _(meshState->meshMutex);

            auto patchName = meshState->RegisterSource(ownerMesh);
            aOut->chunkMaterials.PushBack(patchName);
        }

        Raw::MeshAppearance::Owner::Set(aOut, aMesh);
    }
}

void App::MeshTemplateModule::OnLoadMaterials(Red::CMesh* aMesh, Red::MeshMaterialsToken& aToken,
                                              const Red::DynArray<Red::CName>& aMaterialNames, uint8_t a4)
{
    if (aToken.materials->size != aMaterialNames.size || !ContainsUnresolvedMaterials(*aToken.materials))
        return;

    Red::JobQueue jobQueue;
    jobQueue.Wait(aToken.job);
    jobQueue.Dispatch([aMesh, aMaterialNames, finalMaterials = aToken.materials](const Red::JobGroup& aJobGroup) {
        if (!ContainsUnresolvedMaterials(*finalMaterials))
            return;

        auto meshState = AcquireMeshState(aMesh);

        Red::CMesh* sourceMesh;
        MeshState* sourceState;

        {
            std::shared_lock _(meshState->meshMutex);
            sourceMesh = meshState->ResolveSource(aMaterialNames.Back());
        }

        if (sourceMesh)
        {
            sourceState = AcquireMeshState(sourceMesh);
        }
        else
        {
            sourceMesh = aMesh;
            sourceState = meshState;
        }

        if (sourceState == meshState && meshState->IsStatic())
            return;

        std::unique_lock _(meshState->meshMutex);

        Red::JobQueue jobQueue(aJobGroup);
        jobQueue.Wait(meshState->lastJob);
        jobQueue.Dispatch([meshState, aMesh, sourceState, sourceMesh, aMaterialNames, finalMaterials](const Red::JobGroup& aJobGroup) {
            ProcessMeshResource(meshState, aMesh, sourceState, sourceMesh, aMaterialNames, finalMaterials, aJobGroup);
        });

        meshState->lastJob = std::move(jobQueue.Capture());
    });

    aToken.job = std::move(jobQueue.Capture());
}

App::MeshTemplateModule::MeshState* App::MeshTemplateModule::AcquireMeshState(Red::CMesh* aMesh)
{
    std::unique_lock _(s_stateLock);

    auto it = s_states.find(aMesh->path);
    if (it == s_states.end())
    {
        it = s_states.emplace(aMesh->path, Core::MakeUnique<MeshState>(aMesh)).first;
    }

    return it.value().get();
}

void App::MeshTemplateModule::ProcessMeshResource(MeshState* aMeshState, Red::CMesh* aMesh,
                                                  MeshState* aSourceState, Red::CMesh* aSourceMesh,
                                                  const Red::DynArray<Red::CName>& aMaterialNames,
                                                  const Red::SharedPtr<Red::DynArray<Red::Handle<Red::IMaterial>>>& aFinalMaterials,
                                                  const Red::JobGroup& aJobGroup)
{
    Red::JobQueue jobQueue(aJobGroup);
    Core::Vector<DeferredMaterial> deferredMaterials(0);

    std::scoped_lock _(aMeshState->meshMutex, aSourceState->sourceMutex);

    aMeshState->FillMaterials(aMesh);

    for (int32_t chunkIndex = 0; chunkIndex < aMaterialNames.size; ++chunkIndex)
    {
        const auto& chunkName = aMaterialNames[chunkIndex];

        if (aMeshState->HasMaterialEntry(chunkName))
            continue;

        if (chunkName.hash == aSourceMesh->path.hash)
        {
            aMesh->materialEntries.EmplaceBack();

            auto& materialEntry = aMesh->materialEntries.Back();
            materialEntry.name = chunkName;
            materialEntry.material = s_dummyMaterial;
            materialEntry.materialWeak = s_dummyMaterial;
            materialEntry.isLocalInstance = true;

            (*aFinalMaterials)[chunkIndex] = s_dummyMaterial;
            continue;
        }

        auto materialName = chunkName;
        auto templateName = chunkName;
        auto sourceIndex = aSourceState->GetMaterialEntryIndex(chunkName);

        if (sourceIndex < 0)
        {
            auto chunkMaterialNameStr = std::string_view(chunkName.ToString());
            auto templateNamePos = chunkMaterialNameStr.find(SpecialMaterialMarker);
            if (templateNamePos != std::string_view::npos)
            {
                std::string templateNameStr{chunkMaterialNameStr.data() + templateNamePos,
                                            chunkMaterialNameStr.size() - templateNamePos};
                std::string materialNameStr{chunkMaterialNameStr.data(), templateNamePos};

                templateName = Red::CNamePool::Add(templateNameStr.data());
                materialName = Red::CNamePool::Add(materialNameStr.data());
            }
            else
            {
                templateName = DefaultTemplateName;
            }

            sourceIndex = aSourceState->GetTemplateEntryIndex(templateName);

            if (sourceIndex < 0)
            {
                LogError(R"(|{}| Material template "{}" for entry "{}" not found.)",
                         ModuleName, templateName.ToString(), chunkName.ToString());
                continue;
            }
        }

        auto& sourceEntry = aSourceMesh->materialEntries[sourceIndex];
        auto sourceInstance = Red::Cast<Red::CMaterialInstance>(sourceEntry.materialWeak.Lock());

        if (!sourceInstance)
        {
            Red::SharedPtr<Red::ResourceToken<Red::IMaterial>> token;

            if (sourceEntry.isLocalInstance)
            {
                Raw::MeshMaterialBuffer::LoadMaterialAsync(&aSourceMesh->localMaterialBuffer, token,
                                                           Red::AsHandle(aSourceMesh), sourceEntry.index, 0, 0);
                if (!token)
                {
                    LogError("|{}| Material \"{}\" instance not found.", ModuleName, templateName.ToString());
                    continue;
                }
            }
            else
            {
                auto& externalPath = aSourceMesh->externalMaterials[sourceEntry.index].path;
                auto materialPath = ExpandResourcePath(externalPath, aMeshState, materialName);

                token = Red::ResourceLoader::Get()->LoadAsync<Red::IMaterial>(materialPath);
            }

            if (token->IsFailed())
            {
                LogError("|{}| Material \"{}\" instance failed to load.", ModuleName, templateName.ToString());
                continue;
            }

            if (!token->IsLoaded())
            {
                deferredMaterials.push_back({chunkIndex, chunkName, materialName, templateName, sourceIndex, token});
                continue;
            }

            sourceInstance = Red::Cast<Red::CMaterialInstance>(token->resource);

            if (!sourceInstance)
            {
                LogError("|{}| Material \"{}\" must be instance of {}.",
                         ModuleName, templateName.ToString(), Red::CMaterialInstance::NAME);
                continue;
            }

            sourceEntry.material = sourceInstance;
            sourceEntry.materialWeak = sourceInstance;
        }

        aMesh->materialEntries.EmplaceBack();

        auto materialInstance = materialName != templateName
            ? CloneMaterialInstance(sourceInstance, aMeshState, materialName, jobQueue)
            : sourceInstance;

        auto& materialEntry = aMesh->materialEntries.Back();
        materialEntry.name = chunkName;
        materialEntry.material = materialInstance;
        materialEntry.materialWeak = materialInstance;
        materialEntry.isLocalInstance = true;

        (*aFinalMaterials)[chunkIndex] = materialInstance;
    }

    if (deferredMaterials.empty())
        return;

    for (auto& deferred : deferredMaterials)
    {
        jobQueue.Wait(deferred.sourceToken->job);
    }

    jobQueue.Dispatch([aMesh, aMeshState, aSourceMesh, aSourceState, aFinalMaterials, deferredMaterials = std::move(deferredMaterials)](const Red::JobGroup& aJobGroup) {
        std::scoped_lock _(aMeshState->meshMutex, aSourceState->sourceMutex);

        Red::JobQueue jobQueue(aJobGroup);

        for (auto& deferred : deferredMaterials)
        {
            if (deferred.sourceToken->IsFailed())
            {
                LogError("|{}| Material \"{}\" instance failed to load.", ModuleName, deferred.templateName.ToString());
                continue;
            }

            auto sourceInstance = Red::Cast<Red::CMaterialInstance>(deferred.sourceToken->resource);

            if (!sourceInstance)
            {
                LogError("|{}| Material \"{}\" must be instance of {}.",
                         ModuleName, deferred.templateName.ToString(), Red::CMaterialInstance::NAME);
                continue;
            }

            auto& sourceEntry = aSourceMesh->materialEntries[deferred.sourceIndex];
            sourceEntry.material = sourceInstance;
            sourceEntry.materialWeak = sourceInstance;

            aMesh->materialEntries.EmplaceBack();

            auto materialInstance = deferred.materialName != deferred.templateName
                ? CloneMaterialInstance(sourceInstance, aMeshState, deferred.materialName, jobQueue)
                : sourceInstance;

            auto& materialEntry = aMesh->materialEntries.Back();
            materialEntry.name = deferred.chunkName;
            materialEntry.material = materialInstance;
            materialEntry.materialWeak = materialInstance;
            materialEntry.isLocalInstance = true;

            (*aFinalMaterials)[deferred.chunkIndex] = materialInstance;
        }
    });
}

bool App::MeshTemplateModule::ContainsUnresolvedMaterials(const Red::DynArray<Red::Handle<Red::IMaterial>>& aMaterials)
{
    return std::ranges::any_of(aMaterials, [](const auto& aMaterial) { return !aMaterial; });
}

Red::Handle<Red::CMaterialInstance> App::MeshTemplateModule::CloneMaterialInstance(
    const Red::Handle<Red::CMaterialInstance>& aSourceInstance, MeshState* aState, Red::CName aMaterialName,
    Red::JobQueue& aJobQueue)
{
    auto materialInstance = Red::MakeHandle<Red::CMaterialInstance>();
    materialInstance->baseMaterial = aSourceInstance->baseMaterial;
    materialInstance->enableMask = aSourceInstance->enableMask;
    materialInstance->resourceVersion = aSourceInstance->resourceVersion;
    materialInstance->audioTag = aSourceInstance->audioTag;

    for (const auto& sourceParam : aSourceInstance->params)
    {
        materialInstance->params.PushBack(sourceParam);
    }

    ExpandMaterialInstanceParams(materialInstance, aState, aMaterialName, aJobQueue);

    auto& baseReference = materialInstance->baseMaterial;

    if (ExpandResourceReference(materialInstance->baseMaterial, aState, aMaterialName))
    {
        aJobQueue.Wait(materialInstance->baseMaterial.token->job);
        aJobQueue.Dispatch([materialInstance, aState, aMaterialName](const Red::JobGroup& aJobGroup) {
            auto& baseReference = materialInstance->baseMaterial;
            if (auto baseInstance = Red::Cast<Red::CMaterialInstance>(baseReference.token->resource))
            {
                Red::JobQueue jobQueue(aJobGroup);
                ExpandMaterialInstanceParams(baseInstance, aState, aMaterialName, jobQueue);
            }
        });
    }
    else if (materialInstance->baseMaterial.path && !materialInstance->baseMaterial.token)
    {
        materialInstance->baseMaterial.LoadAsync();

        aJobQueue.Wait(baseReference.token->job);
        aJobQueue.Dispatch([materialInstance, aState, aMaterialName](const Red::JobGroup& aJobGroup) {
            auto& baseReference = materialInstance->baseMaterial;
            if (auto baseInstance = Red::Cast<Red::CMaterialInstance>(baseReference.token->resource))
            {
                Red::JobQueue jobQueue(aJobGroup);

                auto cloneToken = Red::MakeShared<Red::ResourceToken<Red::IMaterial>>();
                cloneToken->self = cloneToken;
                cloneToken->resource = CloneMaterialInstance(baseInstance, aState, aMaterialName, jobQueue);
                cloneToken->path = baseInstance->path;
                cloneToken->finished = 1;

                baseReference.token = std::move(cloneToken);

                ExpandMaterialInstanceParams(baseInstance, aState, aMaterialName, jobQueue);
            }
        });
    }

    return materialInstance;
}

void App::MeshTemplateModule::ExpandMaterialInstanceParams(Red::Handle<Red::CMaterialInstance>& aMaterialInstance,
                                                           MeshState* aState, Red::CName aMaterialName,
                                                           Red::JobQueue& aJobQueue)
{
    for (const auto& materialParam : aMaterialInstance->params)
    {
        if (materialParam.data.GetType()->GetType() == Red::ERTTIType::ResourceReference)
        {
            auto materialParamData = materialParam.data.GetDataPtr();
            auto& materialReference = *reinterpret_cast<Red::ResourceReference<>*>(materialParamData);

            if (ExpandResourceReference(materialReference, aState, aMaterialName))
            {
                aJobQueue.Wait(materialReference.token->job);
            }
        }
    }
}

template<typename T>
bool App::MeshTemplateModule::ExpandResourceReference(Red::ResourceReference<T>& aReference, MeshState* aState,
                                                      Red::CName aMaterialName)
{
    if (aReference.token)
        return false;

    auto path = ExpandResourcePath(aReference.path, aState, aMaterialName);

    if (path == aReference.path)
        return false;

    aReference.path = path;

    if (!path)
        return false;

    aReference.LoadAsync();
    return true;
}

Red::ResourcePath App::MeshTemplateModule::ExpandResourcePath(Red::ResourcePath aPath, MeshState* aState,
                                                              Red::CName aMaterialName)
{
    auto& controller = GarmentOverrideModule::GetDynamicAppearanceController();
    auto pathStr = controller->GetPathString(aPath);

    if (!controller->IsDynamicValue(pathStr))
    {
        return aPath;
    }

    auto result = controller->ProcessString(aState->GetContext(), {{MaterialAttr, aMaterialName}}, pathStr.data());

    if (!result.valid)
    {
        LogError("|{}| Dynamic path \"{}\" is invalid and cannot be processed.", ModuleName, pathStr);
        return aPath;
    }

#ifndef NDEBUG
    LogDebug("|{}| Dynamic path resolved to \"{}\".", ModuleName, result.value);
#endif

    return result.value.data();
}

template<typename T>
void App::MeshTemplateModule::EnsureResourceLoaded(Red::ResourceReference<T>& aRef)
{
    if (!aRef.token)
    {
        aRef.LoadAsync();
    }

    EnsureResourceLoaded(aRef.token);
}

template<typename T>
void App::MeshTemplateModule::EnsureResourceLoaded(Red::SharedPtr<Red::ResourceToken<T>>& aToken)
{
    Red::WaitForResource(aToken, std::chrono::milliseconds(1000));
}

bool App::MeshTemplateModule::IsSpecialMaterial(Red::CName aMaterialName)
{
    return aMaterialName.ToString()[0] == SpecialMaterialMarker;
}

bool App::MeshTemplateModule::IsContextualMesh(Red::CMesh* aMesh)
{
    return aMesh->materialEntries.size > 0 &&
           aMesh->materialEntries.Front().isLocalInstance &&
           aMesh->materialEntries.Front().name == ContextMaterialName;
}

void App::MeshTemplateModule::PrefetchMeshState(Red::CMesh* aMesh, const Core::Map<Red::CName, std::string>& aContext)
{
    if (!aContext.empty())
    {
        auto meshState = AcquireMeshState(aMesh);
        meshState->FillContext(aContext);
    }
    else if (IsContextualMesh(aMesh))
    {
        AcquireMeshState(aMesh);
    }
}

void App::MeshTemplateModule::OnAddStubAppearance(Red::CMesh* aMesh)
{
    if (!aMesh->renderResourceBlob || aMesh->surfaceAreaPerAxis.X < 0.0)
    {
        Raw::CMesh::AddStubAppearance(aMesh);
    }
}

void App::MeshTemplateModule::OnPreloadAppearances(bool& aResult, Red::CMesh* aMesh)
{
    if (aResult && !aMesh->forceLoadAllAppearances && aMesh->appearances.size == 1)
    {
        if (aMesh->appearances[0]->chunkMaterials.size == 0)
        {
            aResult = false;
        }
        else if (aMesh->appearances[0]->name != DefaultAppearanceName)
        {
            if (aMesh->materialEntries.size == 0 || IsSpecialMaterial(aMesh->materialEntries[0].name))
            {
                aResult = false;
            }
        }
    }
}

App::MeshTemplateModule::MeshState::MeshState(Red::CMesh* aMesh)
    : dynamic(true)
{
    FillMaterials(aMesh);

    if (!templates.empty())
    {
        PrefetchContext(aMesh);
    }
    else
    {
        MarkStatic();
    }
}

void App::MeshTemplateModule::MeshState::MarkStatic()
{
    dynamic = false;
    context.clear();
    templates.clear();
}

bool App::MeshTemplateModule::MeshState::IsStatic() const
{
    return !dynamic;
}

void App::MeshTemplateModule::MeshState::PrefetchContext(Red::CMesh* aMesh)
{
    auto contextIndex = GetTemplateEntryIndex(ContextMaterialName);
    if (contextIndex == 0)
    {
        Raw::MeshMaterialBuffer::LoadMaterialAsync(&aMesh->localMaterialBuffer, contextToken, Red::AsHandle(aMesh),
                                                   aMesh->materialEntries[contextIndex].index, 0, 0);
    }
}

void App::MeshTemplateModule::MeshState::FillContext(const Core::Map<Red::CName, std::string>& aContext)
{
    contextToken.Reset();

    for (const auto& [attrName, attrValue] : aContext)
    {
        context.emplace(Red::CNamePool::Add(Str::SnakeCase(attrName.ToString()).data()), attrValue);
    }
}

const App::DynamicAttributeList& App::MeshTemplateModule::MeshState::GetContext()
{
    if (contextToken)
    {
        EnsureResourceLoaded(contextToken);

        if (!contextToken->IsLoaded())
        {
            contextToken.Reset();
            return context;
        }

        auto& metaInstance = Red::Cast<Red::CMaterialInstance>(contextToken->resource);

        if (!metaInstance)
        {
            contextToken.Reset();
            return context;
        }

        for (auto& param : metaInstance->params)
        {
            if (param.data.GetType()->GetType() == Red::ERTTIType::Name)
            {
                auto attrName = Red::CNamePool::Add(Str::SnakeCase(param.name.ToString()).data());
                auto attrValue = *reinterpret_cast<Red::CName*>(param.data.GetDataPtr());

                context.emplace(attrName, attrValue);
            }
        }

        contextToken.Reset();
    }

    return context;
}

void App::MeshTemplateModule::MeshState::FillMaterials(Red::CMesh* aMesh)
{
    materials.clear();

    for (auto i = 0; i < aMesh->materialEntries.size; ++i)
    {
        const auto& materialName = aMesh->materialEntries[i].name;

        if (IsSpecialMaterial(materialName))
        {
            templates[materialName] = i;
        }
        else
        {
            materials[materialName] = i;
        }
    }
}

void App::MeshTemplateModule::MeshState::RegisterMaterialEntry(Red::CName aMaterialName, int32_t aEntryIndex)
{
    materials[aMaterialName] = aEntryIndex;
}

int32_t App::MeshTemplateModule::MeshState::GetTemplateEntryIndex(Red::CName aMaterialName)
{
    auto templateEntry = templates.find(aMaterialName);

    if (templateEntry == templates.end())
        return -1;

    return templateEntry.value();
}

int32_t App::MeshTemplateModule::MeshState::GetMaterialEntryIndex(Red::CName aMaterialName)
{
    auto materialEntry = materials.find(aMaterialName);

    if (materialEntry == materials.end())
        return -1;

    return materialEntry.value();
}

bool App::MeshTemplateModule::MeshState::HasMaterialEntry(Red::CName aMaterialName) const
{
    return materials.contains(aMaterialName);
}

Red::CName App::MeshTemplateModule::MeshState::RegisterSource(Red::CMesh* aSourceMesh)
{
    Red::CName sourceName = aSourceMesh->path.hash;
    sources[sourceName] = aSourceMesh;

    return sourceName;
}

Red::CMesh* App::MeshTemplateModule::MeshState::ResolveSource(Red::CName aSourceName)
{
    auto sourceMesh = sources.find(aSourceName);

    if (sourceMesh == sources.end())
        return nullptr;

    return sourceMesh.value();
}
