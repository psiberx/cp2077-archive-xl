#include "Extension.hpp"
#include "App/Extensions/Garment/Extension.hpp"
#include "App/Utils/Str.hpp"

namespace
{
constexpr auto ExtensionName = "Mesh";

constexpr auto SpecialMaterialMarker = '@';
constexpr auto ContextMaterialName = Red::CName("@context");
constexpr auto DefaultTemplateName = Red::CName("@material");
constexpr auto DefaultAppearanceName = Red::CName("default");
constexpr auto MaterialAttr = Red::CName("material");
constexpr auto ExpansionSourceAttr = Red::CName("appearance_expansion_source");
}

std::string_view App::MeshExtension::GetName()
{
    return ExtensionName;
}

bool App::MeshExtension::Load()
{
    HookAfter<Raw::CMesh::FindAppearance>(&OnFindAppearance).OrThrow();
    Hook<Raw::CMesh::LoadMaterialsAsync>(&OnLoadMaterials).OrThrow();
    Hook<Raw::CMesh::AddStubAppearance>(&OnAddStubAppearance).OrThrow();
    Hook<Raw::CMesh::ShouldPreloadAppearances>(&OnPreloadAppearances).OrThrow();

    s_dummyMesh = Red::MakeHandle<Red::CMesh>();
    s_dummyAppearance = Red::MakeHandle<Red::meshMeshAppearance>();
    s_dummyMaterial = Red::MakeHandle<Red::CMaterialInstance>();

    Raw::MeshAppearance::Owner::Set(s_dummyAppearance.instance, s_dummyMesh.instance);

    return true;
}

bool App::MeshExtension::Unload()
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

void App::MeshExtension::OnFindAppearance(Red::Handle<Red::meshMeshAppearance>& aOut, Red::CMesh* aMesh,
                                               Red::CName aName)
{
    if (!aOut)
        return;

    if (!Raw::MeshAppearance::Owner::Ptr(aOut))
    {
        aOut = s_dummyAppearance;
        return;
    }

    if (!aOut->name || aMesh->appearances.size == 0)
        return;

    if (aOut->chunkMaterials.size > 0 && aOut->tags.size == 0)
        return;

    auto meshState = AcquireMeshState(aMesh);

    std::unique_lock _(meshState->meshMutex);

    if (aOut->chunkMaterials.size == 0)
    {
        auto sourceIndex = meshState->GetExpansionSourceIndex(aMesh);
        auto sourceAppearance = aMesh->appearances[sourceIndex];

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

    if (aOut->chunkMaterials.size > 0 && aOut->tags.size == 1)
    {
        aOut->chunkMaterials.PushBack(aOut->tags[0]);
        aOut->tags.Clear();
    }
}

void* App::MeshExtension::OnLoadMaterials(Red::CMesh* aTargetMesh, Red::MeshMaterialsToken& aToken,
                                               const Red::DynArray<Red::CName>& aMaterialNames, uint8_t a4)
{
    Raw::CMesh::LoadMaterialsAsync(aTargetMesh, aToken, aMaterialNames, a4);

    if (!aTargetMesh->path || aMaterialNames.size == 0 || aToken.materials->size != aMaterialNames.size ||
        !ContainsUnresolvedMaterials(*aToken.materials))
        return &aToken;

    Red::JobQueue jobQueue;
    jobQueue.Wait(aToken.job);
    jobQueue.Dispatch([targetMeshWeak = Red::AsWeakHandle(aTargetMesh), materialNames = aMaterialNames,
                       finalMaterials = aToken.materials](const Red::JobGroup& aJobGroup) {
        if (!ContainsUnresolvedMaterials(*finalMaterials))
            return;

        auto targetMesh = targetMeshWeak.Lock();

        if (!targetMesh)
            return;

        auto targetMeshState = AcquireMeshState(targetMesh);

        Red::Handle<Red::CMesh> sourceMesh;
        Core::SharedPtr<MeshState> sourceMeshState;

        {
            std::shared_lock _(targetMeshState->meshMutex);
            sourceMesh = targetMeshState->ResolveSource(materialNames.Back());
        }

        if (sourceMesh)
        {
            sourceMeshState = AcquireMeshState(sourceMesh);
        }
        else
        {
            sourceMesh = targetMesh;
            sourceMeshState = targetMeshState;
        }

        if (sourceMeshState == targetMeshState && targetMeshState->IsStatic())
            return;

        std::unique_lock _(targetMeshState->meshMutex);

        Red::JobQueue jobQueue(aJobGroup);

        if (targetMeshState->contextToken)
        {
            jobQueue.Wait(targetMeshState->contextToken->job);
        }

        jobQueue.Wait(targetMeshState->lastJob);

        jobQueue.Dispatch([targetMeshState, targetMeshWeak = Red::ToWeakHandle(targetMesh),
                           sourceMeshState, sourceMeshWeak = Red::ToWeakHandle(sourceMesh),
                           materialNames, finalMaterials](const Red::JobGroup& aJobGroup) {
            auto targetMesh = targetMeshWeak.Lock();
            auto sourceMesh = sourceMeshWeak.Lock();
            if (targetMesh && sourceMesh)
            {
                ProcessMeshResource(targetMeshState, targetMesh, sourceMeshState, sourceMesh, materialNames,
                                    finalMaterials, aJobGroup);
            }
        });

        targetMeshState->lastJob = std::move(jobQueue.Capture());
    });

    aToken.job = std::move(jobQueue.Capture());

    return &aToken;
}

Core::SharedPtr<App::MeshExtension::MeshState> App::MeshExtension::AcquireMeshState(Red::CMesh* aMesh)
{
    std::unique_lock _(s_stateLock);

    auto it = s_states.find(aMesh->path);
    if (it == s_states.end())
    {
        it = s_states.emplace(aMesh->path, Core::MakeShared<MeshState>(aMesh)).first;
    }

    return it.value();
}

void App::MeshExtension::ProcessMeshResource(const Core::SharedPtr<MeshState>& aMeshState,
                                             const Red::Handle<Red::CMesh>& aMesh,
                                             const Core::SharedPtr<MeshState>& aSourceState,
                                             const Red::Handle<Red::CMesh>& aSourceMesh,
                                             const Red::DynArray<Red::CName>& aMaterialNames,
                                             const Red::SharedPtr<Red::DynArray<Red::Handle<Red::IMaterial>>>& aFinalMaterials,
                                             const Red::JobGroup& aJobGroup)
{
    Red::JobQueue jobQueue(aJobGroup);
    Core::Vector<DeferredMaterial> deferredMaterials(0);

    std::scoped_lock _(aMeshState->meshMutex, aSourceState->sourceMutex);

    aMeshState->FillMaterials(aMesh);
    if (aSourceState != aMeshState)
    {
        aSourceState->FillMaterials(aSourceMesh);
    }

    for (int32_t chunkIndex = 0; chunkIndex < aMaterialNames.size; ++chunkIndex)
    {
        const auto& chunkName = aMaterialNames[chunkIndex];

        if (aMeshState->HasMaterialEntry(chunkName))
            continue;

        if (chunkName.hash == aSourceMesh->path.hash)
        {
            aMeshState->materials[chunkName] = static_cast<int32_t>(aMesh->materialEntries.size);
            aMesh->materialEntries.EmplaceBack();

            auto& materialEntry = aMesh->materialEntries.Back();
            materialEntry.name = chunkName;
            materialEntry.material = s_dummyMaterial;
            materialEntry.materialWeak = s_dummyMaterial;
            materialEntry.isLocalInstance = true;

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
                LogError(R"(|{}| Material for entry "{}" cannot be resoled.)", ExtensionName, chunkName.ToString());
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
                                                           aSourceMesh, sourceEntry.index, 0, 0);
                if (!token)
                {
                    LogError("|{}| Material \"{}\" instance not found.", ExtensionName, templateName.ToString());
                    continue;
                }
            }
            else
            {
                auto& externalPath = aSourceMesh->externalMaterials[sourceEntry.index].path;
                auto materialPath = ExpandResourcePath(externalPath, aMeshState, materialName);

                if (!materialPath)
                {
                    //LogError("|{}| Material \"{}\" path cannot be resolved.", ExtensionName, templateName.ToString());
                    continue;
                }

                token = Red::ResourceLoader::Get()->LoadAsync<Red::IMaterial>(materialPath);
            }

            if (token->IsFailed())
            {
                LogError("|{}| Material \"{}\" instance failed to load.", ExtensionName, templateName.ToString());
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
                         ExtensionName, templateName.ToString(), Red::CMaterialInstance::NAME);
                continue;
            }

            sourceEntry.material = sourceInstance;
            sourceEntry.materialWeak = sourceInstance;
        }

        aMeshState->materials[chunkName] = static_cast<int32_t>(aMesh->materialEntries.size);
        aMesh->materialEntries.EmplaceBack();

        auto materialInstance = materialName != templateName
            ? CloneMaterialInstance(sourceInstance, aMeshState, materialName, jobQueue, true)
            : sourceInstance;

        auto& materialEntry = aMesh->materialEntries.Back();
        materialEntry.name = chunkName;
        materialEntry.material = materialInstance;
        materialEntry.materialWeak = materialInstance;
        materialEntry.isLocalInstance = true;
    }

    if (deferredMaterials.empty())
    {
        for (int32_t chunkIndex = 0; chunkIndex < aMaterialNames.size; ++chunkIndex)
        {
            const auto& chunkName = aMaterialNames[chunkIndex];
            auto materialIndex = aMeshState->GetMaterialEntryIndex(chunkName);
            if (materialIndex >= 0)
            {
                (*aFinalMaterials)[chunkIndex] = aMesh->materialEntries[materialIndex].materialWeak;
            }
            else
            {
                (*aFinalMaterials)[chunkIndex] = {};
            }
        }
        return;
    }

    for (auto& deferred : deferredMaterials)
    {
        jobQueue.Wait(deferred.sourceToken->job);
    }

    jobQueue.Dispatch([aMesh, aMeshState, aSourceMesh, aSourceState, aMaterialNames, aFinalMaterials,
                       deferredMaterials = std::move(deferredMaterials)](const Red::JobGroup& aJobGroup) {
        std::scoped_lock _(aMeshState->meshMutex, aSourceState->sourceMutex);

        Red::JobQueue jobQueue(aJobGroup);

        for (auto& deferred : deferredMaterials)
        {
            if (deferred.sourceToken->IsFailed())
            {
                LogError("|{}| Material \"{}\" instance failed to load.", ExtensionName, deferred.templateName.ToString());
                continue;
            }

            auto sourceInstance = Red::Cast<Red::CMaterialInstance>(deferred.sourceToken->resource);

            if (!sourceInstance)
            {
                LogError("|{}| Material \"{}\" must be instance of {}.",
                         ExtensionName, deferred.templateName.ToString(), Red::CMaterialInstance::NAME);
                continue;
            }

            auto& sourceEntry = aSourceMesh->materialEntries[deferred.sourceIndex];
            sourceEntry.material = sourceInstance;
            sourceEntry.materialWeak = sourceInstance;

            aMeshState->materials[deferred.chunkName] = static_cast<int32_t>(aMesh->materialEntries.size);
            aMesh->materialEntries.EmplaceBack();

            auto materialInstance = deferred.materialName != deferred.templateName
                ? CloneMaterialInstance(sourceInstance, aMeshState, deferred.materialName, jobQueue, true)
                : sourceInstance;

            auto& materialEntry = aMesh->materialEntries.Back();
            materialEntry.name = deferred.chunkName;
            materialEntry.material = materialInstance;
            materialEntry.materialWeak = materialInstance;
            materialEntry.isLocalInstance = true;
        }

        for (int32_t chunkIndex = 0; chunkIndex < aMaterialNames.size; ++chunkIndex)
        {
            const auto& chunkName = aMaterialNames[chunkIndex];
            auto materialIndex = aMeshState->GetMaterialEntryIndex(chunkName);
            if (materialIndex >= 0)
            {
                (*aFinalMaterials)[chunkIndex] = aMesh->materialEntries[materialIndex].materialWeak;
            }
            else
            {
                (*aFinalMaterials)[chunkIndex] = {};
            }
        }
    });
}

bool App::MeshExtension::ContainsUnresolvedMaterials(const Red::DynArray<Red::Handle<Red::IMaterial>>& aMaterials)
{
    return std::ranges::any_of(aMaterials, [](const auto& aMaterial) { return !aMaterial; });
}

Red::Handle<Red::CMaterialInstance> App::MeshExtension::CloneMaterialInstance(
    const Red::Handle<Red::CMaterialInstance>& aSourceInstance, const Core::SharedPtr<MeshState>& aMeshState,
    Red::CName aMaterialName, Red::JobQueue& aJobQueue, bool aAppendExtraContextPatams)
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

    if (aAppendExtraContextPatams)
    {
        for (const auto& contextParam : aMeshState->GetContextParams())
        {
            for (auto& materialParam : materialInstance->params)
            {
                if (materialParam.name == contextParam.name)
                {
                    if (materialParam.data.GetType() == contextParam.data.GetType())
                    {
                        materialParam.data = contextParam.data;
                    }
                    break;
                }
            }
        }
    }

    ExpandMaterialInstanceParams(materialInstance, aMeshState, aMaterialName, aJobQueue);

    if (ExpandResourceReference(materialInstance->baseMaterial, aMeshState, aMaterialName))
    {
        aJobQueue.Wait(materialInstance->baseMaterial.token->job);
        aJobQueue.Dispatch([materialInstance, aMeshState, aMaterialName](const Red::JobGroup& aJobGroup) {
            auto& baseReference = materialInstance->baseMaterial;
            if (auto baseInstance = Red::Cast<Red::CMaterialInstance>(baseReference.token->resource))
            {
                Red::JobQueue jobQueue(aJobGroup);
                ExpandMaterialInstanceParams(baseInstance, aMeshState, aMaterialName, jobQueue);
            }
        });
    }
    else if (materialInstance->baseMaterial.path && !materialInstance->baseMaterial.token)
    {
        auto templateToken = Red::ResourceLoader::Get()->LoadAsync<Red::CMaterialInstance>(materialInstance->baseMaterial.path);

        aJobQueue.Wait(templateToken->job);
        aJobQueue.Dispatch([materialInstance, templateToken, aMeshState, aMaterialName](const Red::JobGroup& aJobGroup) {
            if (!templateToken->IsLoaded())
                return;

            Red::JobQueue jobQueue(aJobGroup);

            auto cloneToken = Red::MakeShared<Red::ResourceToken<Red::IMaterial>>();
            cloneToken->self = cloneToken;
            cloneToken->resource = CloneMaterialInstance(templateToken->resource, aMeshState, aMaterialName, jobQueue);
            cloneToken->path = templateToken->path;
            cloneToken->finished = 1;

            materialInstance->baseMaterial.token = std::move(cloneToken);
        });
    }

    return materialInstance;
}

void App::MeshExtension::ExpandMaterialInstanceParams(Red::Handle<Red::CMaterialInstance>& aMaterialInstance,
                                                      const Core::SharedPtr<MeshState>& aMeshState,
                                                      Red::CName aMaterialName, Red::JobQueue& aJobQueue)
{
    for (auto i = static_cast<int32_t>(aMaterialInstance->params.size) - 1; i >= 0; --i)
    {
        const auto& materialParam = aMaterialInstance->params[i];

        if (materialParam.data.GetType()->GetType() == Red::ERTTIType::ResourceReference)
        {
            auto materialParamData = materialParam.data.GetDataPtr();
            auto& materialReference = *reinterpret_cast<Red::ResourceReference<>*>(materialParamData);

            if (ExpandResourceReference(materialReference, aMeshState, aMaterialName))
            {
                aJobQueue.Wait(materialReference.token->job);
            }
            else if (!materialReference.path)
            {
                aMaterialInstance->params.RemoveAt(i);
            }
        }
    }
}

template<typename T>
bool App::MeshExtension::ExpandResourceReference(Red::ResourceReference<T>& aReference,
                                                      const Core::SharedPtr<MeshState>& aState,
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

Red::ResourcePath App::MeshExtension::ExpandResourcePath(Red::ResourcePath aPath,
                                                         const Core::SharedPtr<MeshState>& aState,
                                                         Red::CName aMaterialName)
{
    auto& controller = GarmentExtension::GetDynamicAppearanceController();
    auto pathStr = controller->GetPathString(aPath);

    if (!controller->IsDynamicValue(pathStr))
    {
        return aPath;
    }

    auto result = controller->ProcessString(aState->GetContextAttrs(), {{MaterialAttr, aMaterialName}}, pathStr.data());

    if (!result.valid)
    {
        LogError("|{}| Dynamic path \"{}\" cannot be resolved.", ExtensionName, pathStr);
        return {};
    }

#ifndef NDEBUG
    if (!result.value.empty() && result.value != pathStr)
    {
        LogDebug("|{}| Dynamic path resolved to \"{}\".", ExtensionName, result.value);
    }
#endif

    return result.value.data();
}

template<typename T>
void App::MeshExtension::EnsureResourceLoaded(Red::ResourceReference<T>& aRef)
{
    if (!aRef.token)
    {
        aRef.LoadAsync();
    }

    EnsureResourceLoaded(aRef.token);
}

template<typename T>
void App::MeshExtension::EnsureResourceLoaded(Red::SharedPtr<Red::ResourceToken<T>>& aToken)
{
    Red::WaitForResource(aToken, std::chrono::milliseconds(1000));
}

bool App::MeshExtension::IsSpecialMaterial(Red::CName aMaterialName)
{
    return aMaterialName.ToString()[0] == SpecialMaterialMarker;
}

bool App::MeshExtension::IsContextualMesh(Red::CMesh* aMesh)
{
    return aMesh->materialEntries.size > 0 &&
           aMesh->materialEntries.Front().isLocalInstance &&
           aMesh->materialEntries.Front().name == ContextMaterialName;
}

void App::MeshExtension::PrefetchMeshState(Red::CMesh* aMesh, const Core::Map<Red::CName, std::string>& aContext)
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

Red::CName App::MeshExtension::RegisterMeshSource(Red::CMesh* aMesh, Red::CMesh* aSourceMesh)
{
    if (!aSourceMesh || !aSourceMesh->materialEntries.size)
        return {};

    auto meshState = AcquireMeshState(aMesh);
    return meshState->RegisterSource(aSourceMesh);
}

void App::MeshExtension::OnAddStubAppearance(Red::CMesh* aMesh)
{
    if (!aMesh->renderResourceBlob || aMesh->surfaceAreaPerAxis.X < 0.0)
    {
        Raw::CMesh::AddStubAppearance(aMesh);
    }
}

bool App::MeshExtension::OnPreloadAppearances(Red::CMesh* aMesh)
{
    if (!aMesh || !aMesh->path || aMesh->appearances.size == 0)
        return false;

    auto result = Raw::CMesh::ShouldPreloadAppearances(aMesh);

    if (result && !aMesh->forceLoadAllAppearances && aMesh->appearances.size == 1)
    {
        if (aMesh->appearances[0]->chunkMaterials.size == 0)
        {
            result = false;
        }
        else if (aMesh->appearances[0]->name != DefaultAppearanceName)
        {
            if (aMesh->materialEntries.size == 0 || IsSpecialMaterial(aMesh->materialEntries[0].name))
            {
                result = false;
            }
        }
    }

    return result;
}

App::MeshExtension::MeshState::MeshState(Red::CMesh* aMesh)
    : dynamic(true)
    , meshPath(aMesh->path)
    , expansionSourceIndex(-1)
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

void App::MeshExtension::MeshState::MarkStatic()
{
    dynamic = false;
    contextParams.Clear();
    contextAttrs.clear();
    templates.clear();
}

bool App::MeshExtension::MeshState::IsStatic() const
{
    return !dynamic;
}

void App::MeshExtension::MeshState::PrefetchContext(Red::CMesh* aMesh)
{
    auto contextIndex = GetTemplateEntryIndex(ContextMaterialName);
    if (contextIndex == 0)
    {
        Raw::MeshMaterialBuffer::LoadMaterialAsync(&aMesh->localMaterialBuffer, contextToken, Red::AsHandle(aMesh),
                                                   aMesh->materialEntries[contextIndex].index, 0, 0);
    }
}

void App::MeshExtension::MeshState::FillContext(const Core::Map<Red::CName, std::string>& aContext)
{
    contextToken.Reset();

    for (const auto& [attrName, attrValue] : aContext)
    {
        contextAttrs.emplace(Red::CNamePool::Add(Str::SnakeCase(attrName.ToString()).data()), attrValue);
    }

    ResolveContextProperties();
}

void App::MeshExtension::MeshState::ResolveContextProperties()
{
    auto expansionSourceStr = GetContextAttr(ExpansionSourceAttr);
    if (!expansionSourceStr.empty())
    {
        expansionSource = {expansionSourceStr.data(), expansionSourceStr.size()};
    }
}

void App::MeshExtension::MeshState::EnsureContextFilled()
{
    if (!contextToken)
        return;

    EnsureResourceLoaded(contextToken);

    if (!contextToken->IsLoaded())
    {
        auto& controller = GarmentExtension::GetDynamicAppearanceController();
        auto pathStr = controller->GetPathString(meshPath);

        if (!pathStr.empty())
        {
            LogWarning(R"(|{}| Can't load context for mesh \"{}\".)", ExtensionName, pathStr);
        }
        else
        {
            LogWarning(R"(|{}| Can't load context for mesh {}.)", ExtensionName, meshPath.hash);
        }

        contextToken.Reset();
        return;
    }

    auto& contextInstance = Red::Cast<Red::CMaterialInstance>(contextToken->resource);

    if (!contextInstance)
    {
        contextToken.Reset();
        return;
    }

    contextParams = std::move(contextInstance->params);

    for (const auto& contextParam : contextParams)
    {
        if (contextParam.data.GetType()->GetType() == Red::ERTTIType::Name)
        {
            auto attrName = Red::CNamePool::Add(Str::SnakeCase(contextParam.name.ToString()).data());
            auto attrValue = *reinterpret_cast<Red::CName*>(contextParam.data.GetDataPtr());

            contextAttrs.emplace(attrName, attrValue);
        }
    }

    contextToken.Reset();

    ResolveContextProperties();
}

const Red::DynArray<Red::MaterialParameterInstance>& App::MeshExtension::MeshState::GetContextParams()
{
    EnsureContextFilled();

    return contextParams;
}

const App::DynamicAttributeList& App::MeshExtension::MeshState::GetContextAttrs()
{
    EnsureContextFilled();

    return contextAttrs;
}

std::string_view App::MeshExtension::MeshState::GetContextAttr(Red::CName aAttr)
{
    EnsureContextFilled();

    const auto& it = contextAttrs.find(aAttr);

    if (it == contextAttrs.end())
        return {};

    return it.value().value;
}

Red::CName App::MeshExtension::MeshState::GetExpansionSource()
{
    EnsureContextFilled();

    return expansionSource;
}

int32_t App::MeshExtension::MeshState::GetExpansionSourceIndex(Red::CMesh* aMesh)
{
    if (expansionSourceIndex < 0)
    {
        expansionSourceIndex = 0;

        auto sourceName = GetExpansionSource();
        if (sourceName)
        {
            for (auto i = 0; i < aMesh->appearances.size; ++i)
            {
                if (aMesh->appearances[i]->name == sourceName)
                {
                    expansionSourceIndex = i;
                    break;
                }
            }
        }
    }

    return expansionSourceIndex;
}

void App::MeshExtension::MeshState::FillMaterials(Red::CMesh* aMesh)
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

void App::MeshExtension::MeshState::RegisterMaterialEntry(Red::CName aMaterialName, int32_t aEntryIndex)
{
    materials[aMaterialName] = aEntryIndex;
}

int32_t App::MeshExtension::MeshState::GetTemplateEntryIndex(Red::CName aMaterialName)
{
    auto templateEntry = templates.find(aMaterialName);

    if (templateEntry == templates.end())
        return -1;

    return templateEntry.value();
}

int32_t App::MeshExtension::MeshState::GetMaterialEntryIndex(Red::CName aMaterialName)
{
    auto materialEntry = materials.find(aMaterialName);

    if (materialEntry == materials.end())
        return -1;

    return materialEntry.value();
}

bool App::MeshExtension::MeshState::HasMaterialEntry(Red::CName aMaterialName) const
{
    return materials.contains(aMaterialName);
}

Red::CName App::MeshExtension::MeshState::RegisterSource(Red::CMesh* aSourceMesh)
{
    Red::CName sourceName = aSourceMesh->path.hash;
    sources[sourceName] = Red::AsWeakHandle(aSourceMesh);

    return sourceName;
}

Red::Handle<Red::CMesh> App::MeshExtension::MeshState::ResolveSource(Red::CName aSourceName)
{
    auto sourceMesh = sources.find(aSourceName);

    if (sourceMesh == sources.end())
        return {};

    return sourceMesh.value().Lock();
}
