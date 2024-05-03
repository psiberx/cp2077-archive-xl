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
            LogError("|{}| Material template \"{}\" entry not found.", ModuleName, templateName.ToString());
            continue;
        }

        Red::Handle<Red::CMaterialInstance> sourceInstance;
        uint16_t sourceIndex;

        {
            auto& sourceEntry = aMesh->materialEntries[templateIt.value()];
            sourceInstance = Red::Cast<Red::CMaterialInstance>(sourceEntry.materialWeak).Lock();
            sourceIndex = sourceEntry.index;
        }

        if (!sourceInstance)
        {
            Red::SharedPtr<Red::ResourceToken<Red::IMaterial>> token;
            Raw::MeshMaterialBuffer::LoadMaterialAsync(&aMesh->localMaterialBuffer, token, Red::AsHandle(aMesh),
                                                       sourceIndex, 0, 0);

            if (!token)
            {
                LogError("|{}| Material template \"{}\" instance not found.", ModuleName, templateName.ToString());
                continue;
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

        auto materialInstance = Red::MakeHandle<Red::CMaterialInstance>();
        materialInstance->baseMaterial = sourceInstance->baseMaterial;
        materialInstance->enableMask = sourceInstance->enableMask;
        materialInstance->resourceVersion = sourceInstance->resourceVersion;
        materialInstance->audioTag = sourceInstance->audioTag;

        if (ProcessResourceReference(materialInstance->baseMaterial, materialName))
        {
            aLoadingJobs.push_back(materialInstance->baseMaterial.token->job);
        }

        for (const auto& sourceParam : sourceInstance->params)
        {
            materialInstance->params.PushBack(sourceParam);

            if (sourceParam.data.GetType()->GetType() == Red::ERTTIType::ResourceReference)
            {
                auto& materialParam = materialInstance->params.Back();
                auto materialParamData = materialParam.data.GetDataPtr();
                auto& materialReference = *reinterpret_cast<Red::ResourceReference<>*>(materialParamData);

                if (ProcessResourceReference(materialReference, materialName))
                {
                    aLoadingJobs.push_back(materialReference.token->job);
                }
            }
        }

        aMesh->materialEntries.EmplaceBack();

        auto& materialEntry = aMesh->materialEntries.Back();
        materialEntry.name = chunkMaterialName;
        materialEntry.material = materialInstance;
        materialEntry.materialWeak = materialInstance;
        materialEntry.isLocalInstance = true;
    }

    return true;
}

template<typename T>
bool App::MeshTemplateModule::ProcessResourceReference(Red::ResourceReference<T>& aInstance, Red::CName aMaterialName)
{
    if (aInstance.token)
        return false;

    auto& controller = GarmentOverrideModule::GetDynamicAppearanceController();
    auto& pathStr = controller->GetPathStr(aInstance.path);

    if (pathStr.empty())
    {
        return false;
    }

    auto result = controller->ProcessString({{MaterialAttr, aMaterialName}}, {}, pathStr.data());
    if (!result.valid)
    {

        LogError("|{}| Dynamic path \"{}\" is invalid and cannot be processed.", ModuleName, pathStr);
        return false;
    }

    aInstance.path = result.value.data();

    // if (!Red::ResourceDepot::Get()->ResourceExists(aInstance.path))
    // {
    //     LogError("|{}| Dynamic path was resolved to \"{}\", but resource doesn't exist.", ModuleName, result.value);
    //     return false;
    // }

    aInstance.LoadAsync();
    return true;
}
