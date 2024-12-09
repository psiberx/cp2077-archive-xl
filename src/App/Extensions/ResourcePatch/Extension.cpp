#include "Extension.hpp"
#include "App/Extensions/Customization/Extension.hpp"
#include "App/Extensions/Mesh/Extension.hpp"
#include "App/Extensions/ResourceMeta/Extension.hpp"
#include "Red/EntityBuilder.hpp"
#include "Red/Mesh.hpp"
#include "Red/ResourceDepot.hpp"

namespace
{
constexpr auto ExtensionName = "ResourcePatch";

constexpr auto AppearancePartsTag = Red::CName("AppearanceParts");
}

std::string_view App::ResourcePatchExtension::GetName()
{
    return ExtensionName;
}

bool App::ResourcePatchExtension::Load()
{
    HookBefore<Raw::ResourceDepot::RequestResource>(&OnResourceRequest).OrThrow();
    HookBefore<Raw::ResourceSerializer::Deserialize>(&OnResourceDeserialize).OrThrow();
    HookBefore<Raw::EntityTemplate::OnLoad>(&OnEntityTemplateLoad).OrThrow();
    HookBefore<Raw::AppearanceResource::OnLoad>(&OnAppearanceResourceLoad).OrThrow();
    HookBefore<Raw::CMesh::OnLoad>(&OnMeshResourceLoad).OrThrow();
    HookBefore<Raw::EntityBuilder::ScheduleExtractComponentsJob>(&OnEntityPackageLoad).OrThrow();
    HookAfter<Raw::AppearanceDefinition::ExtractPartComponents>(&OnPartPackageExtract).OrThrow();
    HookAfter<Raw::GarmentAssembler::ExtractComponentsJob>(&OnGarmentPackageExtract).OrThrow();

    return true;
}

bool App::ResourcePatchExtension::Unload()
{
    Unhook<Raw::ResourceDepot::RequestResource>();
    Unhook<Raw::ResourceSerializer::Deserialize>();
    Unhook<Raw::EntityTemplate::OnLoad>();
    Unhook<Raw::AppearanceResource::OnLoad>();
    Unhook<Raw::CMesh::OnLoad>();
    Unhook<Raw::EntityBuilder::ScheduleExtractComponentsJob>();
    Unhook<Raw::AppearanceDefinition::ExtractPartComponents>();
    Unhook<Raw::GarmentAssembler::ExtractComponentsJob>();

    return true;
}

void App::ResourcePatchExtension::Configure()
{
    s_patches.clear();

    auto depot = Red::ResourceDepot::Get();
    Core::Set<Red::ResourcePath> invalidPaths;
    Core::Set<Red::ResourcePath> patchPaths;

    for (auto& config : m_configs)
    {
        for (const auto& [patchPath, patchScope] : config.patches)
        {
            if (!depot->ResourceExists(patchPath))
            {
                if (!invalidPaths.contains(patchPath))
                {
                    LogError("|{}| Patch resource \"{}\" doesn't exist.", ExtensionName, config.paths[patchPath]);
                    invalidPaths.insert(patchPath);
                }
                continue;
            }

            Core::Set<Red::ResourcePath> targetList;

            for (const auto& includePath : patchScope.includes)
            {
                const auto& includeList = ResourceMetaExtension::GetResourceList(includePath);
                if (!includeList.empty())
                {
                    targetList.insert(includeList.begin(), includeList.end());
                }
                else
                {
                    targetList.insert(includePath);
                }
            }

            for (const auto& excludePath : patchScope.excludes)
            {
                const auto& excludeList = ResourceMetaExtension::GetResourceList(excludePath);
                if (!excludeList.empty())
                {
                    targetList.erase(excludeList.begin(), excludeList.end());
                }
                else
                {
                    targetList.erase(excludePath);
                }
            }

            targetList.erase(patchPath);

            for (const auto& targetPath : targetList)
            {
                // if (!depot->ResourceExists(targetPath))
                // {
                //     if (!invalidPaths.contains(targetPath))
                //     {
                //         LogWarning("|{}| Target resource \"{}\" doesn't exist.",
                //                    ExtensionName, config.paths[targetPath]);
                //         invalidPaths.insert(targetPath);
                //     }
                //     continue;
                // }

                s_patches[targetPath].insert(patchPath);
                s_paths[targetPath] = config.paths[targetPath];
            }

            s_paths[patchPath] = config.paths[patchPath];
            patchPaths.insert(patchPath);
        }
    }

    for (auto patch = s_patches.begin(); patch != s_patches.end();)
    {
        const auto& targetPath = patch->first;

        if (patchPaths.contains(targetPath))
        {
            if (!invalidPaths.contains(targetPath))
            {
                LogError(R"(|{}| Patch resource "{}" cannot be patched.)", ExtensionName, s_paths[targetPath]);
                invalidPaths.insert(targetPath);
            }

            patch = s_patches.erase(patch);
            continue;
        }

        ++patch;
    }

    m_configs.clear();
}

void App::ResourcePatchExtension::OnResourceRequest(Red::ResourceDepot*, const uintptr_t* aOut, Red::ResourcePath aPath,
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

void App::ResourcePatchExtension::OnResourceDeserialize(void* aSerializer, uint64_t, uint64_t, Red::JobHandle& aJob,
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

void App::ResourcePatchExtension::OnEntityTemplateLoad(Red::EntityTemplate* aTemplate, void*)
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

void App::ResourcePatchExtension::OnAppearanceResourceLoad(Red::AppearanceResource* aResource)
{
    const auto& patchList = GetPatchList(aResource->path);

    if (patchList.empty())
        return;

    Core::Set<Red::CName> newAppearances;

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
                    if (!newAppearances.contains(patchDefinition->name))
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
                    }
                    else
                    {
                        existingDefinition = patchDefinition;
                    }

                    isNewAppearance = false;
                    break;
                }
            }

            if (isNewAppearance)
            {
                aResource->appearances.EmplaceBack(patchDefinition);
                newAppearances.insert(patchDefinition->name);
            }
        }
    }
}

void App::ResourcePatchExtension::OnMeshResourceLoad(Red::CMesh* aMesh, void* a2)
{
    const auto& fix = ResourceMetaExtension::GetResourceFix(aMesh->path);

    if (fix.DefinesNameMappings())
    {
        for (auto& appearance : aMesh->appearances)
        {
            for (auto& chunkMaterial : appearance->chunkMaterials)
            {
                chunkMaterial = fix.GetMappedName(chunkMaterial);
            }
        }
        for (auto& materialEntry : aMesh->materialEntries)
        {
            materialEntry.name = fix.GetMappedName(materialEntry.name);
        }
    }

    const auto& patchList = GetPatchList(aMesh->path);

    if (!patchList.empty())
    {
        for (const auto& patchPath : patchList)
        {
            auto patchMesh = GetPatchResource<Red::CMesh>(patchPath);

            if (!patchMesh || patchMesh->appearances.size == 0)
                continue;

            auto patchName = MeshExtension::RegisterMeshSource(aMesh, patchMesh);

            for (const auto& patchAppearance : patchMesh->appearances)
            {
                auto cloneAppearance = Red::MakeHandle<Red::meshMeshAppearance>();
                cloneAppearance->name = patchAppearance->name;
                cloneAppearance->chunkMaterials = patchAppearance->chunkMaterials;

                cloneAppearance->tags.Clear();
                cloneAppearance->tags.PushBack(patchName);

                auto isNewAppearance = true;

                for (auto& existingAppearance : aMesh->appearances)
                {
                    if (existingAppearance->name == cloneAppearance->name)
                    {
                        existingAppearance = cloneAppearance;
                        isNewAppearance = false;
                        break;
                    }
                }

                if (isNewAppearance)
                {
                    aMesh->appearances.EmplaceBack(cloneAppearance);
                }
            }

            aMesh->forceLoadAllAppearances = false;
        }
    }

    MeshExtension::PrefetchMeshState(aMesh, fix.GetContext());
}

void App::ResourcePatchExtension::OnEntityPackageLoad(Red::JobQueue& aJobQueue, void*,
                                                      Red::EntityBuilderJobParams* aParams)
{
    aJobQueue.Dispatch([entityBuilderWeak = aParams->entityBuilderWeak](const Red::JobGroup& aJobGroup) {
        if (entityBuilderWeak.Expired())
            return;

        auto& entityBuilder = entityBuilderWeak.instance;
        auto forceParts = entityBuilder->entityTemplate->visualTagsSchema &&
                          entityBuilder->entityTemplate->visualTagsSchema->visualTags.Contains(AppearancePartsTag);

        if (entityBuilder->flags.ExtractAppearance && entityBuilder->appearance.extractor)
        {
            IncludeAppearanceParts(entityBuilder->appearance.resource,
                                   entityBuilder->appearance.definition,
                                   entityBuilder->appearance.extractor->results,
                                   entityBuilder->appearance.extractor->disablePostLoad,
                                   entityBuilder->appearance.extractor->disableImports,
                                   entityBuilder->appearance.extractor->disablePreInitialization,
                                   forceParts, aJobGroup);
        }

        if (entityBuilder->flags.ExtractAppearances)
        {
            for (auto& appearance : entityBuilder->appearances)
            {
                IncludeAppearanceParts(appearance.resource,
                                       appearance.definition,
                                       appearance.extractor->results,
                                       appearance.extractor->disablePostLoad,
                                       appearance.extractor->disableImports,
                                       appearance.extractor->disablePreInitialization,
                                       forceParts, aJobGroup);
            }
        }
    });

    aJobQueue.Dispatch([entityBuilderWeak = aParams->entityBuilderWeak](const Red::JobGroup& aJobGroup) {
        if (entityBuilderWeak.Expired())
            return;

        auto& entityBuilder = entityBuilderWeak.instance;

        if (entityBuilder->flags.ExtractEntity && entityBuilder->entityExtractor)
        {
            PatchPackageResults(entityBuilder->entityTemplate,
                                entityBuilder->entityExtractor->results,
                                entityBuilder->entityExtractor->disablePostLoad,
                                entityBuilder->entityExtractor->disableImports,
                                entityBuilder->entityExtractor->disablePreInitialization);
        }

        if (entityBuilder->flags.ExtractAppearance && entityBuilder->appearance.extractor)
        {
            PatchPackageResults(entityBuilder->appearance.resource,
                                entityBuilder->appearance.definition,
                                entityBuilder->appearance.extractor->results,
                                entityBuilder->appearance.extractor->disablePostLoad,
                                entityBuilder->appearance.extractor->disableImports,
                                entityBuilder->appearance.extractor->disablePreInitialization,
                                aJobGroup);
        }

        if (entityBuilder->flags.ExtractAppearances)
        {
            for (auto& appearance : entityBuilder->appearances)
            {
                PatchPackageResults(appearance.resource,
                                    appearance.definition,
                                    appearance.extractor->results,
                                    appearance.extractor->disablePostLoad,
                                    appearance.extractor->disableImports,
                                    appearance.extractor->disablePreInitialization,
                                    aJobGroup);
            }
        }
    });
}

void App::ResourcePatchExtension::OnPartPackageExtract(
    Red::DynArray<Red::Handle<Red::ISerializable>>& aResultObjects,
    const Red::SharedPtr<Red::ResourceToken<Red::EntityTemplate>>& aPartToken)
{
    PatchPackageResults(aPartToken->resource, aResultObjects, false, true, false);
}

void App::ResourcePatchExtension::OnGarmentPackageExtract(Red::GarmentExtractionParams* aParams,
                                                          const Red::JobGroup& aJobGroup)
{
    const auto& patchList = GetPatchList(aParams->partTemplate->path);

    if (patchList.empty())
        return;

    auto originalEntityTemplate = aParams->partTemplate;

    for (const auto& patchPath : patchList)
    {
        auto patchTemplate = GetPatchResource<Red::EntityTemplate>(patchPath);
        if (patchTemplate)
        {
            aParams->partTemplate = patchTemplate;
            Raw::GarmentAssembler::ExtractComponentsJob(aParams, aJobGroup);
        }
    }

    aParams->partTemplate = originalEntityTemplate;
}

void App::ResourcePatchExtension::IncludeAppearanceParts(const Red::Handle<Red::AppearanceResource>& aResource,
                                                         const Red::Handle<Red::AppearanceDefinition>& aDefinition,
                                                         Red::DynArray<Red::Handle<Red::ISerializable>>& aResultObjects,
                                                         bool aDisablePostLoad, bool aDisableImports,
                                                         bool aDisablePreInitialization, bool aForceIncludeParts,
                                                         const Red::JobGroup& aJobGroup)
{
    if (!aResource || !aDefinition)
        return;

    if (aDefinition->partsValues.size == 0)
        return;

    if (!aForceIncludeParts && !aDefinition->visualTags.Contains(AppearancePartsTag))
        return;

    Red::JobQueue jobQueue{aJobGroup};

    for (const auto& part : aDefinition->partsValues)
    {
        auto partToken = Red::ResourceLoader::Get()->LoadAsync<Red::EntityTemplate>(part.resource.path);

        jobQueue.Wait(partToken->job);
        jobQueue.Dispatch([partToken = std::move(partToken), &aResultObjects, aDisablePostLoad, aDisableImports,
                           aDisablePreInitialization]() {
            if (partToken->IsFailed())
                return;

            auto& partHeader = partToken->resource->compiledDataHeader;

            if (partHeader.IsEmpty())
                return;

            auto partExtractor = Red::ObjectPackageExtractor(partHeader);
            partExtractor.disablePostLoad = aDisablePostLoad;
            partExtractor.disableImports = aDisableImports;
            partExtractor.disablePreInitialization = aDisablePreInitialization;
            partExtractor.ExtractSync();

            if (partExtractor.results.size > 0)
            {
                MergeComponents(aResultObjects, partExtractor.results);

                // TODO: Process external components and components overrides
            }
        });
    }
}

void App::ResourcePatchExtension::PatchPackageResults(const Red::Handle<Red::EntityTemplate>& aTemplate,
                                                      Red::DynArray<Red::Handle<Red::ISerializable>>& aResultObjects,
                                                      bool aDisablePostLoad, bool aDisableImports,
                                                      bool aDisablePreInitialization)
{
    if (!aTemplate)
        return;

    auto& templateHeader = aTemplate->compiledDataHeader;

    if (templateHeader.IsEmpty())
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
        patchExtractor.disablePostLoad = aDisablePostLoad;
        patchExtractor.disableImports = aDisableImports;
        patchExtractor.disablePreInitialization = aDisablePreInitialization;
        patchExtractor.ExtractSync();

        if (patchExtractor.results.size > 0)
        {
            MergeComponents(aResultObjects, patchExtractor.results);
            MergeEntity(aResultObjects, patchExtractor.results, templateHeader.rootIndex, patchHeader.rootIndex);
        }
    }
}

void App::ResourcePatchExtension::PatchPackageResults(const Red::Handle<Red::AppearanceResource>& aResource,
                                                      const Red::Handle<Red::AppearanceDefinition>& aDefinition,
                                                      Red::DynArray<Red::Handle<Red::ISerializable>>& aResultObjects,
                                                      bool aDisablePostLoad, bool aDisableImports,
                                                      bool aDisablePreInitialization, const Red::JobGroup& aJobGroup)
{
    if (!aResource || !aDefinition)
        return;

    CustomizationExtension::FixCustomizationComponents(aResource, aDefinition, aResultObjects);

    const auto& patchList = GetPatchList(aResource->path);

    if (patchList.empty())
        return;

    Red::JobQueue jobQueue{aJobGroup};

    for (const auto& patchPath : patchList)
    {
        auto patchDefinition = GetPatchDefinition(patchPath, aDefinition->name);

        if (!patchDefinition)
            continue;

        jobQueue.Wait(patchDefinition->compiledData.LoadAsync()->job);
        jobQueue.Dispatch([patchDefinition, &aResultObjects, aDisablePostLoad, aDisableImports,
                           aDisablePreInitialization](const Red::JobGroup& aJobGroup) {
            auto patchLoader = Red::ObjectPackageReader(patchDefinition->compiledData);
            patchLoader.ReadHeader(patchDefinition->compiledDataHeader);

            auto patchExtractor = Red::MakeShared<Red::ObjectPackageExtractor>(patchDefinition->compiledDataHeader);
            patchExtractor->disablePostLoad = aDisablePostLoad;
            patchExtractor->disableImports = aDisableImports;
            patchExtractor->disablePreInitialization = aDisablePreInitialization;

            Red::JobQueue jobQueue{aJobGroup};
            jobQueue.Wait(patchExtractor->ExtractAsync());
            jobQueue.Dispatch([patchExtractor = std::move(patchExtractor), &aResultObjects]() {
                if (patchExtractor->results.size > 0)
                {
                    MergeComponents(aResultObjects, patchExtractor->results);
                }
            });
        });
    }
}

void App::ResourcePatchExtension::MergeEntity(Red::DynArray<Red::Handle<Red::ISerializable>>& aResultObjects,
                                              Red::DynArray<Red::Handle<Red::ISerializable>>& aPatchObjects,
                                              int16_t aResultEntityIndex, int16_t aPatchEntityIndex)
{
    if (aResultEntityIndex < 0 || aResultEntityIndex >= aResultObjects.size)
        return;

    if (aPatchEntityIndex < 0 || aPatchEntityIndex >= aPatchObjects.size)
        return;

    if (auto patchEntity = Red::Cast<Red::Entity>(aPatchObjects[aPatchEntityIndex]))
    {
        if (patchEntity->GetNativeType() != Red::GetClass<Red::Entity>())
        {
            aResultObjects[aResultEntityIndex] = std::move(patchEntity);
        }
    }
}

void App::ResourcePatchExtension::MergeComponents(Red::DynArray<Red::Handle<Red::ISerializable>>& aResultObjects,
                                                  Red::DynArray<Red::Handle<Red::ISerializable>>& aPatchObjects)
{
    for (auto& patchObject : aPatchObjects)
    {
        if (auto patchComponent = Red::Cast<Red::IComponent>(patchObject))
        {
            auto isNewComponent = true;

            for (auto& resultObject : aResultObjects)
            {
                if (auto& resultComponent = Red::Cast<Red::IComponent>(resultObject))
                {
                    if (resultComponent->name == patchComponent->name &&
                        resultComponent->id.unk00 == patchComponent->id.unk00)
                    {
                        resultObject = patchComponent;
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

void App::ResourcePatchExtension::MergeObjects(Red::DynArray<Red::Handle<Red::ISerializable>>& aResultObjects,
                                               Red::DynArray<Red::Handle<Red::ISerializable>>& aPatchObjects)
{
    for (auto& patchObject : aPatchObjects)
    {
        aResultObjects.PushBack(patchObject);
    }
}

void App::ResourcePatchExtension::MergeResources(Red::DynArray<Red::SharedPtr<Red::ResourceToken<>>>& aResultResources,
                                                 Red::DynArray<Red::SharedPtr<Red::ResourceToken<>>>& aPatchResources)
{
    for (auto& patchResource : aPatchResources)
    {
        aResultResources.PushBack(patchResource);
    }
}

const Core::Set<Red::ResourcePath>& App::ResourcePatchExtension::GetPatchList(Red::ResourcePath aTargetPath)
{
    static const Core::Set<Red::ResourcePath> s_null;

    const auto& patchIt = s_patches.find(aTargetPath);

    if (patchIt == s_patches.end())
        return s_null;

    return patchIt.value();
}

template<typename T>
Red::SharedPtr<Red::ResourceToken<T>> App::ResourcePatchExtension::GetPatchToken(Red::ResourcePath aPatchPath)
{
    std::shared_lock _(s_tokenLock);
    auto& token = s_tokens[aPatchPath];

    if constexpr (!std::is_same_v<T, Red::CResource>)
    {
        if (!token->IsFinished())
        {
            LogWarning("|{}| Patch resource \"{}\" is not ready.", ExtensionName, s_paths[token->path]);

            Red::WaitForResource(token, std::chrono::milliseconds(250));

            if (!token->IsFinished())
            {
                Red::WaitForResource(token, std::chrono::milliseconds(250));
            }
        }

        if (token->IsFailed())
        {
            LogError("|{}| Patch resource \"{}\" is failed to load.", ExtensionName, s_paths[token->path]);
            return {};
        }
    }

    return *reinterpret_cast<Red::SharedPtr<Red::ResourceToken<T>>*>(&token);
}

template<typename T>
Red::Handle<T> App::ResourcePatchExtension::GetPatchResource(Red::ResourcePath aPatchPath)
{
    auto token = GetPatchToken<T>(aPatchPath);

    if (!token)
        return {};

    return token->resource;
}

Red::Handle<Red::AppearanceDefinition> App::ResourcePatchExtension::GetPatchDefinition(Red::ResourcePath aResourcePath,
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