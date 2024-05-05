#include "Module.hpp"
#include "App/Extensions/GarmentOverride/Module.hpp"

namespace
{
constexpr auto ModuleName = "MeshTemplate";

constexpr auto TemplateMarker = '@';
constexpr auto DefaultTemplateName = Red::CName("@material");
constexpr auto MaterialAttr = Red::CName("material");
}

std::string_view App::MeshTemplateModule::GetName()
{
    return ModuleName;
}

bool App::MeshTemplateModule::Load()
{
    if (!Hook<Raw::CMesh::LoadMaterialsAsync>(&OnLoadMaterials))
        throw std::runtime_error("Failed to hook [CMesh::LoadMaterialsAsync].");

    return true;
}

bool App::MeshTemplateModule::Unload()
{
    {
        std::unique_lock _(s_stateLock);
        s_states.clear();
    }

    Unhook<Raw::CMesh::LoadMaterialsAsync>();

    return true;
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

    if (!meshState->IsDynamic())
        return false;

    std::unique_lock _(meshState->mutex);

    Core::Map<Red::CName, std::size_t> templateIndexes;
    Core::Set<Red::CName> existingMaterialNames;

    for (const auto& materialEntry : aMesh->materialEntries)
    {
        if (materialEntry.name.ToString()[0] == TemplateMarker)
        {
            templateIndexes.insert_or_assign(materialEntry.name, &materialEntry - aMesh->materialEntries.begin());
        }

        for (const auto& chunkMaterialName : aMaterialNames)
        {
            if (materialEntry.name == chunkMaterialName)
            {
                existingMaterialNames.insert(chunkMaterialName);
                break;
            }
        }
    }

    if (existingMaterialNames.size() == aMaterialNames.size)
        return false;

    if (templateIndexes.empty())
    {
        meshState->MarkStatic();
        return false;
    }

    for (const auto& chunkMaterialName : aMaterialNames)
    {
        if (existingMaterialNames.contains(chunkMaterialName))
            continue;

        auto chunkMaterialNameStr = std::string_view(chunkMaterialName.ToString());
        auto materialName = chunkMaterialName;

        auto templateName = DefaultTemplateName;
        auto templateNamePos = chunkMaterialNameStr.find(TemplateMarker);
        if (templateNamePos != std::string_view::npos)
        {
            std::string templateNameStr{chunkMaterialNameStr.data() + templateNamePos,
                                        chunkMaterialNameStr.size() - templateNamePos};
            std::string materialNameStr{chunkMaterialNameStr.data(), templateNamePos};

            templateName = Red::CNamePool::Add(templateNameStr.data());
            materialName = Red::CNamePool::Add(materialNameStr.data());
        }


        auto templateIt = templateIndexes.find(templateName);
        if (templateIt == templateIndexes.end())
        {
            LogError(R"(|{}| Material template "{}" for "{}" entry not found.)",
                     ModuleName, templateName.ToString(), chunkMaterialName.ToString());
            continue;
        }

        auto& sourceEntry = aMesh->materialEntries[templateIt.value()];
        auto sourceInstance = Red::Cast<Red::CMaterialInstance>(sourceEntry.materialWeak).Lock();

        if (!sourceInstance)
        {
            Red::SharedPtr<Red::ResourceToken<Red::IMaterial>> token;

            if (sourceEntry.isLocalInstance)
            {
                Raw::MeshMaterialBuffer::LoadMaterialAsync(&aMesh->localMaterialBuffer, token, Red::AsHandle(aMesh),
                                                           sourceEntry.index, 0, 0);
                if (!token)
                {
                    LogError("|{}| Material template \"{}\" instance not found.", ModuleName, templateName.ToString());
                    continue;
                }
            }
            else
            {
                auto materialPath = ProcessResourcePath(aMesh->externalMaterials[sourceEntry.index].path, materialName);
                token = Red::ResourceLoader::Get()->LoadAsync<Red::IMaterial>(materialPath);
            }

            Red::WaitForResource(token, std::chrono::milliseconds(1000));

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
        }

        auto materialInstance = CloneMaterialInstance(sourceInstance, materialName, aLoadingJobs);

        aMesh->materialEntries.EmplaceBack();

        auto& materialEntry = aMesh->materialEntries.Back();
        materialEntry.name = chunkMaterialName;
        materialEntry.material = materialInstance;
        materialEntry.materialWeak = materialInstance;
        materialEntry.isLocalInstance = true;
    }

    return true;
}

Red::Handle<Red::CMaterialInstance> App::MeshTemplateModule::CloneMaterialInstance(
    const Red::Handle<Red::CMaterialInstance>& aSourceInstance, Red::CName aMaterialName,
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

    if (ProcessResourceReference(materialInstance->baseMaterial, aMaterialName))
    {
        Red::WaitForResource(materialInstance->baseMaterial.token, std::chrono::milliseconds(1000));
    }
    else if (!materialInstance->baseMaterial.token)
    {
        auto& aReference = materialInstance->baseMaterial;

        aReference.LoadAsync();
        Red::WaitForResource(aReference.token, std::chrono::milliseconds(1000));

        if (auto baseInstance = Red::Cast<Red::CMaterialInstance>(aReference.token->resource))
        {
            auto cloneToken = Red::MakeShared<Red::ResourceToken<Red::IMaterial>>();
            cloneToken->self = cloneToken;
            cloneToken->resource = CloneMaterialInstance(baseInstance, aMaterialName, aLoadingJobs);
            cloneToken->path = baseInstance->path;
            cloneToken->finished = 1;

            aReference.token = std::move(cloneToken);
        }
    }

    ProcessMaterialInstanceParams(materialInstance, aMaterialName, aLoadingJobs);

    if (auto baseInstance = Red::Cast<Red::CMaterialInstance>(materialInstance->baseMaterial.token->resource))
    {
        ProcessMaterialInstanceParams(baseInstance, aMaterialName, aLoadingJobs);
    }

    return materialInstance;
}

void App::MeshTemplateModule::ProcessMaterialInstanceParams(Red::Handle<Red::CMaterialInstance>& aMaterialInstance,
                                                            Red::CName aMaterialName,
                                                            Core::Vector<Red::JobHandle>& aLoadingJobs)
{
    for (const auto& materialParam : aMaterialInstance->params)
    {
        if (materialParam.data.GetType()->GetType() == Red::ERTTIType::ResourceReference)
        {
            auto materialParamData = materialParam.data.GetDataPtr();
            auto& materialReference = *reinterpret_cast<Red::ResourceReference<>*>(materialParamData);

            if (ProcessResourceReference(materialReference, aMaterialName))
            {
                aLoadingJobs.push_back(materialReference.token->job);
            }
        }
    }
}

template<typename T>
bool App::MeshTemplateModule::ProcessResourceReference(Red::ResourceReference<T>& aReference, Red::CName aMaterialName)
{
    if (aReference.token)
        return false;

    auto path = ProcessResourcePath(aReference.path, aMaterialName);

    if (path == aReference.path)
        return false;

    aReference.path = path;
    aReference.LoadAsync();

    return true;
}

Red::ResourcePath App::MeshTemplateModule::ProcessResourcePath(Red::ResourcePath aPath, Red::CName aMaterialName)
{
    auto& controller = GarmentOverrideModule::GetDynamicAppearanceController();
    auto& pathStr = controller->GetPathStr(aPath);

    if (pathStr.empty())
    {
        return aPath;
    }

    auto result = controller->ProcessString({{MaterialAttr, aMaterialName}}, {}, pathStr.data());
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
