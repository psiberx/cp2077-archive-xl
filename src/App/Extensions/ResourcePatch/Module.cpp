#include "Module.hpp"
#include "App/Extensions/ResourceAlias/Module.hpp"
#include "Red/EntityBuilder.hpp"
#include "Red/Mesh.hpp"
#include "Red/ResourceDepot.hpp"

namespace
{
constexpr auto ModuleName = "ResourcePatch";
}

std::string_view App::ResourcePatchModule::GetName()
{
    return ModuleName;
}

bool App::ResourcePatchModule::Load()
{
    if (!HookBefore<Raw::ResourceDepot::RequestResource>(&OnResourceRequest))
        throw std::runtime_error("Failed to hook [ResourceDepot::RequestResource].");

    if (!HookBefore<Raw::ResourceSerializer::Deserialize>(&OnResourceDeserialize))
        throw std::runtime_error("Failed to hook [ResourceSerializer::Deserialize].");

    if (!HookAfter<Raw::EntityTemplate::OnLoad>(&OnEntityTemplateLoad))
        throw std::runtime_error("Failed to hook [EntityTemplate::OnLoad].");

    if (!HookBefore<Raw::AppearanceResource::OnLoad>(&OnAppearanceResourceLoad))
        throw std::runtime_error("Failed to hook [AppearanceResource::OnLoad].");

    if (!HookAfter<Raw::CMesh::OnLoad>(&OnMeshResourceLoad))
        throw std::runtime_error("Failed to hook [EntityBuilder::ExtractComponentsJob].");

    if (!HookBefore<Raw::EntityBuilder::ExtractComponentsJob>(&OnEntityPackageExtract))
        throw std::runtime_error("Failed to hook [EntityBuilder::ExtractComponentsJob].");

    if (!HookAfter<Raw::AppearanceDefinition::ExtractPartComponents>(&OnPartPackageExtract))
        throw std::runtime_error("Failed to hook [AppearanceDefinition::ExtractPartComponents].");

    if (!HookAfter<Raw::GarmentAssembler::ExtractComponentsJob>(&OnGarmentPackageExtract))
        throw std::runtime_error("Failed to hook [GarmentAssembler::ProcessComponentsJob].");

    PreparePatches();

    return true;
}

void App::ResourcePatchModule::Reload()
{
    PreparePatches();
}

bool App::ResourcePatchModule::Unload()
{
    Unhook<Raw::ResourceDepot::RequestResource>();
    Unhook<Raw::ResourceSerializer::Deserialize>();
    Unhook<Raw::EntityTemplate::OnLoad>();
    Unhook<Raw::AppearanceResource::OnLoad>();
    Unhook<Raw::CMesh::OnLoad>();
    Unhook<Raw::EntityBuilder::ExtractComponentsJob>();
    Unhook<Raw::AppearanceDefinition::ExtractPartComponents>();
    Unhook<Raw::GarmentAssembler::ExtractComponentsJob>();

    return true;
}

void App::ResourcePatchModule::PreparePatches()
{
    s_patches.clear();

    auto depot = Red::ResourceDepot::Get();
    Core::Set<Red::ResourcePath> invalidPaths;

    for (auto& unit : m_units)
    {
        for (const auto& [targetPathOrAlias, patchList] : unit.patches)
        {
            auto targetList = ResourceAliasModule::ResolveAlias(targetPathOrAlias);
            if (targetList.empty())
            {
                targetList.insert(targetPathOrAlias);
            }

            for (const auto& targetPath : targetList)
            {
                s_paths[targetPath] = unit.paths[targetPath];

                for (const auto& patchPath : patchList)
                {
                    if (patchPath == targetPath)
                        continue;

                    if (!depot->ResourceExists(patchPath))
                    {
                        if (!invalidPaths.contains(patchPath))
                        {
                            LogWarning("|{}| Resource \"{}\" doesn't exist. Skipped.", ModuleName, unit.paths[patchPath]);
                            invalidPaths.insert(patchPath);
                        }
                        continue;
                    }

                    s_patches[targetPath].insert(patchPath);
                    s_paths[patchPath] = unit.paths[patchPath];
                }
            }
        }
    }
}

void App::ResourcePatchModule::OnResourceRequest(Red::ResourceDepot*, const uintptr_t* aOut, Red::ResourcePath aPath,
                                                 const int32_t*)
{
    const auto& patchList = GetPatchList(aPath);

    if (!patchList.empty())
    {
        std::unique_lock _(s_tokenLock);
        for (const auto& patchPath : patchList)
        {
            if (!s_tokens.contains(patchPath))
            {
                s_tokens[patchPath] = Red::ResourceLoader::Get()->LoadAsync(patchPath);
            }
        }
    }
}

void App::ResourcePatchModule::OnResourceDeserialize(void* aSerializer, uint64_t, uint64_t, Red::JobHandle& aJob,
                                                     Red::ResourceSerializerRequest& aRequest, uint64_t,
                                                     Red::DynArray<Red::Handle<Red::ISerializable>>&, uint64_t)
{
    const auto& patchList = GetPatchList(aRequest.path);

    if (!patchList.empty())
    {
        std::shared_lock _(s_tokenLock);
        for (const auto& patchPath : patchList)
        {
            auto patchToken = GetPatchToken(patchPath);
            if (patchToken)
            {
                aJob.Join(patchToken->job);
            }
        }
    }
}

void App::ResourcePatchModule::OnEntityTemplateLoad(Red::EntityTemplate* aTemplate, void*)
{
    const auto& patchList = GetPatchList(aTemplate->path);

    if (patchList.empty())
        return;

    for (const auto& patchPath : patchList)
    {
        auto patchTemplate = GetPatchResource<Red::EntityTemplate>(patchPath);

        if (!patchTemplate)
            continue;

        for (const auto& patchAppearance : patchTemplate->appearances)
        {
            auto isNewAppearance = true;

            for (auto& existingAppearance : aTemplate->appearances)
            {
                if (existingAppearance.name == patchAppearance.name)
                {
                    existingAppearance = patchAppearance;
                    isNewAppearance = false;
                    break;
                }
            }

            if (isNewAppearance)
            {
                aTemplate->appearances.EmplaceBack(patchAppearance);
            }
        }

        if (patchTemplate->visualTagsSchema)
        {
            if (!aTemplate->visualTagsSchema)
            {
                aTemplate->visualTagsSchema = {};
            }

            aTemplate->visualTagsSchema->visualTags.Add(patchTemplate->visualTagsSchema->visualTags);
        }
    }
}

void App::ResourcePatchModule::OnAppearanceResourceLoad(Red::AppearanceResource* aResource)
{
    const auto& patchList = GetPatchList(aResource->path);

    if (patchList.empty())
        return;

    for (const auto& patchPath : patchList)
    {
        auto patchResource = GetPatchResource<Red::AppearanceResource>(patchPath);

        if (!patchResource)
            continue;

        for (const auto& patchDefinition : patchResource->appearances)
        {
            auto isNewAppearance = true;

            for (auto& existingDefinition : aResource->appearances)
            {
                if (existingDefinition->name == patchDefinition->name)
                {
                    {
                        std::unique_lock _(s_definitionLock);
                        s_definitions[patchPath][patchDefinition->name] = patchDefinition;
                    }

                    for (const auto& partValue : patchDefinition->partsValues)
                    {
                        existingDefinition->partsValues.PushBack(partValue);
                    }

                    for (const auto& partOverride : patchDefinition->partsOverrides)
                    {
                        existingDefinition->partsOverrides.PushBack(partOverride);
                    }

                    existingDefinition->visualTags.Add(patchDefinition->visualTags);

                    isNewAppearance = false;
                    break;
                }
            }

            if (isNewAppearance)
            {
                aResource->appearances.EmplaceBack(patchDefinition);
            }
        }
    }
}

void App::ResourcePatchModule::OnMeshResourceLoad(Red::CMesh* aMesh, void*)
{
    const auto& patchList = GetPatchList(aMesh->path);

    if (patchList.empty())
        return;

    for (const auto& patchPath : patchList)
    {
        auto patchMesh = GetPatchResource<Red::CMesh>(patchPath);

        if (!patchMesh)
            continue;

        for (const auto& patchAppearance : patchMesh->appearances)
        {
            auto isNewAppearance = true;

            for (auto& existingAppearance : aMesh->appearances)
            {
                if (existingAppearance->name == patchAppearance->name)
                {
                    existingAppearance = patchAppearance;
                    isNewAppearance = false;
                    break;
                }
            }

            if (isNewAppearance)
            {
                aMesh->appearances.EmplaceBack(patchAppearance);
            }
        }
    }
}

void App::ResourcePatchModule::OnEntityPackageExtract(Red::EntityBuilderJobParams* aParams, void* a2)
{
    if (aParams->entityBuilderWeak.Expired())
        return;

    auto& entityBuilder = aParams->entityBuilder;

    if (entityBuilder->flags.ExtractEntity)
    {
        PatchPackageExtractorResults(entityBuilder->entityTemplate,
                                     entityBuilder->entityExtractor->results);
    }

    if (entityBuilder->flags.ExtractAppearance)
    {
        PatchPackageExtractorResults(entityBuilder->appearance.resource,
                                     entityBuilder->appearance.definition,
                                     entityBuilder->appearance.extractor->results);
    }

    if (entityBuilder->flags.ExtractAppearances)
    {
        for (auto& appearance : entityBuilder->appearances)
        {
            PatchPackageExtractorResults(appearance.resource,
                                         appearance.definition,
                                         appearance.extractor->results);
        }
    }
}

void App::ResourcePatchModule::OnPartPackageExtract(
    Red::DynArray<Red::Handle<Red::ISerializable>>& aResultObjects,
    const Red::SharedPtr<Red::ResourceToken<Red::EntityTemplate>>& aPartToken)
{
    PatchPackageExtractorResults(aPartToken->resource, aResultObjects);
}

void App::ResourcePatchModule::OnGarmentPackageExtract(Red::GarmentComponentParams* aParams,
                                                       const Red::JobGroup& aJobGroup)
{
    const auto& patchList = GetPatchList(aParams->entityTemplate->path);

    if (patchList.empty())
        return;

    auto originalEntityTemplate = aParams->entityTemplate;

    for (const auto& patchPath : patchList)
    {
        auto patchTemplate = GetPatchResource<Red::EntityTemplate>(patchPath);
        if (patchTemplate)
        {
            aParams->entityTemplate = patchTemplate;
            Raw::GarmentAssembler::ExtractComponentsJob(aParams, aJobGroup);
        }
    }

    aParams->entityTemplate = originalEntityTemplate;
}

void App::ResourcePatchModule::PatchPackageExtractorResults(
    const Red::Handle<Red::EntityTemplate>& aTemplate,
    Red::DynArray<Red::Handle<Red::ISerializable>>& aResultObjects)
{
    if (!aTemplate)
        return;

    const auto& patchList = GetPatchList(aTemplate->path);

    if (patchList.empty())
        return;

    for (const auto& patchPath : patchList)
    {
        auto patchTemplate = GetPatchResource<Red::EntityTemplate>(patchPath);

        if (!patchTemplate)
            continue;

        auto& patchHeader = patchTemplate->compiledDataHeader;

        if (patchHeader.IsEmpty())
            continue;

        auto patchExtractor = Red::ObjectPackageExtractor(patchHeader);
        patchExtractor.ExtractSync();

        if (patchExtractor.results.size > 0)
        {
            PatchResultEntity(aResultObjects, patchExtractor.results, patchHeader.rootIndex);
            PatchResultComponents(aResultObjects, patchExtractor.results);
        }
    }
}

void App::ResourcePatchModule::PatchPackageExtractorResults(
    const Red::Handle<Red::AppearanceResource>& aResource,
    const Red::Handle<Red::AppearanceDefinition>& aDefinition,
    Red::DynArray<Red::Handle<Red::ISerializable>>& aResultObjects)
{
    if (!aResource)
        return;

    if (ResourceAliasModule::IsAliased(ResourceAliasModule::CustomizationAlias, aResource->path) &&
        aDefinition->partsOverrides.size == 1  && aDefinition->partsOverrides[0].componentsOverrides.size == 1 &&
        !aDefinition->partsOverrides[0].componentsOverrides[0].componentName)
    {
        for (auto& resultObject : aResultObjects)
        {
            if (auto meshComponent = Red::Cast<Red::entSkinnedMeshComponent>(resultObject))
            {
                if (meshComponent->meshAppearance && meshComponent->meshAppearance != "default")
                {
                    meshComponent->meshAppearance = aDefinition->partsOverrides[0].componentsOverrides[0].meshAppearance;
                }
            }
        }
    }

    const auto& patchList = GetPatchList(aResource->path);

    if (patchList.empty())
        return;

    for (const auto& patchPath : patchList)
    {
        auto patchDefinition = GetPatchDefinition(patchPath, aDefinition->name);

        if (!patchDefinition)
            continue;

        auto& patchHeader = patchDefinition->compiledDataHeader;

        if (patchHeader.IsEmpty())
        {
            auto& patchBuffer = patchDefinition->compiledData;

            if (patchBuffer.state != Red::DeferredDataBufferState::Loaded)
            {
                auto bufferToken = patchBuffer.LoadAsync();
                Red::WaitForJob(bufferToken->job, std::chrono::milliseconds(500));
            }

            auto packageLoader = Red::ObjectPackageReader(patchBuffer);
            packageLoader.ReadHeader(patchHeader);
        }

        auto patchExtractor = Red::ObjectPackageExtractor(patchHeader);
        patchExtractor.ExtractSync();

        if (patchExtractor.results.size > 0)
        {
            PatchResultComponents(aResultObjects, patchExtractor.results);
        }
    }
}

void App::ResourcePatchModule::PatchResultEntity(Red::DynArray<Red::Handle<Red::ISerializable>>& aResultObjects,
                                                 Red::DynArray<Red::Handle<Red::ISerializable>>& aPatchObjects,
                                                 int16_t aEntityIndex)
{
    if (aEntityIndex == -1)
        return;

    if (aResultObjects.size > aEntityIndex && aPatchObjects.size > aEntityIndex)
    {
        if (auto patchEntity = Red::Cast<Red::Entity>(aPatchObjects[aEntityIndex]))
        {
            if (patchEntity->GetNativeType() != Red::GetClass<Red::Entity>())
            {
                aResultObjects[aEntityIndex] = std::move(patchEntity);
            }
        }
    }
}

void App::ResourcePatchModule::PatchResultComponents(Red::DynArray<Red::Handle<Red::ISerializable>>& aResultObjects,
                                                     Red::DynArray<Red::Handle<Red::ISerializable>>& aPatchObjects)
{
    for (auto& patchObject : aPatchObjects)
    {
        if (auto patchComponent = Red::Cast<Red::IComponent>(patchObject))
        {
            auto isNewComponent = true;

            for (auto& resultObject : aResultObjects)
            {
                if (auto resultComponent = Red::Cast<Red::IComponent>(resultObject))
                {
                    if (resultComponent->name == patchComponent->name &&
                        resultComponent->id.unk00 == patchComponent->id.unk00)
                    {
                        resultComponent = patchComponent;
                        isNewComponent = false;
                        break;
                    }
                }
            }

            if (isNewComponent)
            {
                aResultObjects.PushBack(std::move(patchComponent));
            }
        }
    }
}

const Core::Set<Red::ResourcePath>& App::ResourcePatchModule::GetPatchList(Red::ResourcePath aTargetPath)
{
    static const Core::Set<Red::ResourcePath> s_null;

    const auto& patchIt = s_patches.find(aTargetPath);

    if (patchIt == s_patches.end())
        return s_null;

    return patchIt.value();
}

template<typename T>
Red::SharedPtr<Red::ResourceToken<T>> App::ResourcePatchModule::GetPatchToken(Red::ResourcePath aPatchPath)
{
    std::shared_lock _(s_tokenLock);
    auto& token = s_tokens[aPatchPath];

    if constexpr (!std::is_same_v<T, Red::CResource>)
    {
        if (!token->IsFinished())
        {
            LogWarning("|{}| Patch resource \"{}\" is not ready.", ModuleName, s_paths[token->path]);

            Red::WaitForResource(token, std::chrono::milliseconds(250));

            if (!token->IsFinished())
            {
                Red::WaitForResource(token, std::chrono::milliseconds(250));
            }
        }

        if (token->IsFailed())
        {
            LogError("|{}| Patch resource \"{}\" is failed to load.", ModuleName, s_paths[token->path]);
            return {};
        }
    }

    return *reinterpret_cast<Red::SharedPtr<Red::ResourceToken<T>>*>(&token);
}

template<typename T>
Red::Handle<T> App::ResourcePatchModule::GetPatchResource(Red::ResourcePath aPatchPath)
{
    auto token = GetPatchToken<T>(aPatchPath);

    if (!token)
        return {};

    return token->resource;
}

Red::Handle<Red::AppearanceDefinition> App::ResourcePatchModule::GetPatchDefinition(Red::ResourcePath aResourcePath,
                                                                                    Red::CName aDefinitionName)
{
    std::shared_lock _(s_definitionLock);

    const auto& resourceIt = s_definitions.find(aResourcePath);
    if (resourceIt == s_definitions.end())
        return {};

    const auto& definitionIt = resourceIt.value().find(aDefinitionName);
    if (definitionIt == resourceIt.value().end())
        return {};

    return definitionIt.value().Lock();
}
