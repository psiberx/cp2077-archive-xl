#include "Module.hpp"
#include "App/Extensions/GarmentOverride/Module.hpp"
#include "App/Utils/Str.hpp"

namespace
{
constexpr auto ModuleName = "MeshTemplate";

constexpr auto SpecialMaterialMarker = '@';
constexpr auto ContextMaterialName = Red::CName("@context");
constexpr auto DefaultTemplateName = Red::CName("@material");
constexpr auto MaterialAttr = Red::CName("material");
}

std::string_view App::MeshTemplateModule::GetName()
{
    return ModuleName;
}

bool App::MeshTemplateModule::Load()
{
    HookAfter<Raw::CMesh::FindAppearance>(&OnFindAppearance).OrThrow();
    Hook<Raw::CMesh::LoadMaterialsAsync>(&OnLoadMaterials).OrThrow();

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

    return true;
}

void App::MeshTemplateModule::OnFindAppearance(Red::Handle<Red::meshMeshAppearance>& aOut, Red::CMesh* aMesh,
                                               Red::CName aName)
{
    if (aOut)
    {
        if (aOut->chunkMaterials.size == 0 && aMesh->appearances.size > 0)
        {
            const auto& sourceAppearance = aMesh->appearances.Front();
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

        const auto ownerMesh = Raw::MeshAppearance::Owner::Ptr(aOut);
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
}

void* App::MeshTemplateModule::OnLoadMaterials(Red::CMesh* aMesh, Red::MeshMaterialsToken& aToken,
                                               const Red::DynArray<Red::CName>& aMaterialNames, uint8_t a4)
{
    Core::Vector<Red::JobHandle> loadingJobs(0);
    ProcessMeshResource(aMesh, aMaterialNames, loadingJobs);

    auto ret = Raw::CMesh::LoadMaterialsAsync(aMesh, aToken, aMaterialNames, a4);

    if (!loadingJobs.empty())
    {
        for (const auto& loadingJob : loadingJobs)
        {
            aToken.job.Join(loadingJob);
        }
    }

    return ret;
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

bool App::MeshTemplateModule::ProcessMeshResource(Red::CMesh* aMesh, const Red::DynArray<Red::CName>& aMaterialNames,
                                                  Core::Vector<Red::JobHandle>& aLoadingJobs)
{
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

    if (sourceState->IsStatic())
        return false;

    std::scoped_lock _(meshState->meshMutex, sourceState->sourceMutex);

    meshState->FillMaterials(aMesh);

    for (const auto& chunkMaterialName : aMaterialNames)
    {
        if (meshState->HasMaterialEntry(chunkMaterialName))
            continue;

        if (chunkMaterialName.hash == sourceMesh->path.hash)
        {
            aMesh->materialEntries.EmplaceBack();

            auto materialInstance = Red::MakeHandle<Red::CMaterialInstance>();
            auto& materialEntry = aMesh->materialEntries.Back();
            materialEntry.name = chunkMaterialName;
            materialEntry.material = materialInstance;
            materialEntry.materialWeak = materialInstance;
            materialEntry.isLocalInstance = true;

            continue;
        }

        auto chunkMaterialNameStr = std::string_view(chunkMaterialName.ToString());
        auto materialName = chunkMaterialName;

        auto templateName = DefaultTemplateName;
        auto templateNamePos = chunkMaterialNameStr.find(SpecialMaterialMarker);
        if (templateNamePos != std::string_view::npos)
        {
            std::string templateNameStr{chunkMaterialNameStr.data() + templateNamePos,
                                        chunkMaterialNameStr.size() - templateNamePos};
            std::string materialNameStr{chunkMaterialNameStr.data(), templateNamePos};

            templateName = Red::CNamePool::Add(templateNameStr.data());
            materialName = Red::CNamePool::Add(materialNameStr.data());
        }

        auto templateIndex = sourceState->GetTemplateEntryIndex(templateName);
        if (templateIndex < 0)
        {
            LogError(R"(|{}| Material template "{}" for "{}" entry not found.)",
                     ModuleName, templateName.ToString(), chunkMaterialName.ToString());
            continue;
        }

        auto& sourceEntry = sourceMesh->materialEntries[templateIndex];
        auto sourceInstance = Red::Cast<Red::CMaterialInstance>(sourceEntry.materialWeak.Lock());

        if (!sourceInstance)
        {
            Red::SharedPtr<Red::ResourceToken<Red::IMaterial>> token;

            if (sourceEntry.isLocalInstance)
            {
                Raw::MeshMaterialBuffer::LoadMaterialAsync(&sourceMesh->localMaterialBuffer, token,
                                                           Red::AsHandle(sourceMesh), sourceEntry.index, 0, 0);
                if (!token)
                {
                    LogError("|{}| Material template \"{}\" instance not found.", ModuleName, templateName.ToString());
                    continue;
                }
            }
            else
            {
                auto& externalPath = sourceMesh->externalMaterials[sourceEntry.index].path;
                auto materialPath = ExpandResourcePath(externalPath, meshState, materialName);

                token = Red::ResourceLoader::Get()->LoadAsync<Red::IMaterial>(materialPath);
            }

            EnsureResourceLoaded(token);

            if (token->IsFailed())
            {
                LogError("|{}| Material template \"{}\" instance failed to load.", ModuleName, templateName.ToString());
                continue;
            }

            sourceInstance = Red::Cast<Red::CMaterialInstance>(token->resource);

            if (!sourceInstance)
            {
                LogError("|{}| Material template \"{}\" must be instance of {}.",
                         ModuleName, templateName.ToString(), Red::CMaterialInstance::NAME);
                continue;
            }

            sourceEntry.material = sourceInstance;
            sourceEntry.materialWeak = sourceInstance;
        }

        aMesh->materialEntries.EmplaceBack();

        auto materialInstance = CloneMaterialInstance(sourceInstance, meshState, materialName, aLoadingJobs);
        auto& materialEntry = aMesh->materialEntries.Back();
        materialEntry.name = chunkMaterialName;
        materialEntry.material = materialInstance;
        materialEntry.materialWeak = materialInstance;
        materialEntry.isLocalInstance = true;
    }

    return true;
}

Red::Handle<Red::CMaterialInstance> App::MeshTemplateModule::CloneMaterialInstance(
    const Red::Handle<Red::CMaterialInstance>& aSourceInstance, MeshState* aState, Red::CName aMaterialName,
    Core::Vector<Red::JobHandle>& aLoadingJobs)
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

    auto& baseReference = materialInstance->baseMaterial;

    if (ExpandResourceReference(baseReference, aState, aMaterialName))
    {
        EnsureResourceLoaded(baseReference);
    }
    else if (materialInstance->baseMaterial.path && !materialInstance->baseMaterial.token)
    {
        EnsureResourceLoaded(baseReference);

        if (auto baseInstance = Red::Cast<Red::CMaterialInstance>(baseReference.token->resource))
        {
            auto cloneToken = Red::MakeShared<Red::ResourceToken<Red::IMaterial>>();
            cloneToken->self = cloneToken;
            cloneToken->resource = CloneMaterialInstance(baseInstance, aState, aMaterialName, aLoadingJobs);
            cloneToken->path = baseInstance->path;
            cloneToken->finished = 1;

            baseReference.token = std::move(cloneToken);
        }
    }

    if (auto baseInstance = Red::Cast<Red::CMaterialInstance>(materialInstance->baseMaterial.token->resource))
    {
        ExpandMaterialInstanceParams(baseInstance, aState, aMaterialName, aLoadingJobs);
    }

    ExpandMaterialInstanceParams(materialInstance, aState, aMaterialName, aLoadingJobs);

    return materialInstance;
}

void App::MeshTemplateModule::ExpandMaterialInstanceParams(Red::Handle<Red::CMaterialInstance>& aMaterialInstance,
                                                           MeshState* aState, Red::CName aMaterialName,
                                                           Core::Vector<Red::JobHandle>& aLoadingJobs)
{
    for (const auto& materialParam : aMaterialInstance->params)
    {
        if (materialParam.data.GetType()->GetType() == Red::ERTTIType::ResourceReference)
        {
            auto materialParamData = materialParam.data.GetDataPtr();
            auto& materialReference = *reinterpret_cast<Red::ResourceReference<>*>(materialParamData);

            if (ExpandResourceReference(materialReference, aState, aMaterialName))
            {
                aLoadingJobs.push_back(materialReference.token->job);
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
    auto& pathStr = controller->GetPathStr(aPath);

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
    materials.clear();
    sources.clear();
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

void App::MeshTemplateModule::MeshState::RegisterMaterialEntry(Red::CName aMaterialName, uint32_t aEntryIndex)
{
    materials[aMaterialName] = aEntryIndex;
}

uint32_t App::MeshTemplateModule::MeshState::GetTemplateEntryIndex(Red::CName aMaterialName)
{
    auto templateEntry = templates.find(aMaterialName);

    if (templateEntry == templates.end())
        return -1;

    return templateEntry.value();
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
