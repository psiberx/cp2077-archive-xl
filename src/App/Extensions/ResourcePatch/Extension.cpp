#include "Extension.hpp"
#include "App/Extensions/Customization/Extension.hpp"
#include "App/Extensions/Mesh/Extension.hpp"
#include "App/Extensions/ResourceMeta/Extension.hpp"
#include "Core/Facades/Container.hpp"
#include "Red/Buffer.hpp"
#include "Red/EntityBuilder.hpp"
#include "Red/Mesh.hpp"
#include "Red/MorphTarget.hpp"
#include "Red/PersistencySystem.hpp"
#include "Red/StreamingWorld.hpp"

namespace
{
constexpr auto ExtensionName = "ResourcePatch";

constexpr auto ResourcePatchTag = Red::CName("ResourcePatch");
constexpr auto AppearancePartsTag = Red::CName("AppearanceParts");

constexpr auto EntityTemplateAppearancesProp = Red::CName("appearances");
constexpr auto EntityTemplateEntityProp = Red::CName("entity");
constexpr auto EntityTemplateComponentsProp = Red::CName("components");
constexpr auto EntityTemplateDependenciesProp = Red::CName("resolvedDependencies");
constexpr auto EntityTemplateVisualTagsProp = Red::CName("visualTags");

constexpr auto AppearanceResourceDefinitionsProp = Red::CName("appearances");
constexpr auto AppearanceResourceComponentsProp = Red::CName("components");
constexpr auto AppearanceResourcePartsValuesProp = Red::CName("partsValues");
constexpr auto AppearanceResourcePartsOverridesProp = Red::CName("partsOverrides");
constexpr auto AppearanceResourceDependenciesProp = Red::CName("resolvedDependencies");
constexpr auto AppearanceResourceVisualTagsProp = Red::CName("visualTags");
constexpr auto AppearanceResourceCensorshipProp = Red::CName("censorshipMapping");

constexpr auto MeshAppearancesProp = Red::CName("appearances");
constexpr auto MeshBlobProp = Red::CName("renderResourceBlob");

constexpr auto MorphTargetMeshProp = Red::CName("baseMesh");
constexpr auto MorphTargetMeshAppProp = Red::CName("baseMeshAppearance");
constexpr auto MorphTargetTextureProp = Red::CName("baseTexture");
constexpr auto MorphTargetTextureParamProp = Red::CName("baseTextureParamName");
constexpr auto MorphTargetBlobProp = Red::CName("blob");
constexpr auto MorphTargetBoundingBoxProp = Red::CName("boundingBox");
constexpr auto MorphTargetTargetsProp = Red::CName("targets");
}

std::string_view App::ResourcePatchExtension::GetName()
{
    return ExtensionName;
}

bool App::ResourcePatchExtension::Load()
{
    HookBefore<Raw::ResourceSerializer::Load>(&OnResourceRequest).OrThrow();
    HookBefore<Raw::ResourceSerializer::Deserialize>(&OnResourceDeserialize).OrThrow();
    HookBefore<Raw::ResourceSerializer::OnDependenciesReady>(&OnResourceReady).OrThrow();
    HookBefore<Raw::EntityTemplate::PostLoad>(&OnEntityTemplateLoad).OrThrow();
    HookBefore<Raw::AppearanceResource::OnLoad>(&OnAppearanceResourceLoad).OrThrow();
    HookBefore<Raw::CMesh::PostLoad>(&OnMeshResourceLoad).OrThrow();
    Hook<Raw::MorphTargetMesh::PostLoad>(&OnMorphTargetResourceLoad).OrThrow();
    HookBefore<Raw::EntityBuilder::ScheduleExtractComponentsJob>(&OnEntityPackageLoad).OrThrow();
    HookAfter<Raw::AppearanceDefinition::ExtractPartComponents>(&OnPartPackageExtract).OrThrow();
    HookAfter<Raw::GarmentAssembler::ExtractComponentsJob>(&OnGarmentPackageExtract).OrThrow();
    HookAfter<Raw::PersistencySystem::SetPersistentStateData>(&OnSetPersistentStateData).OrThrow();

    s_resourcePathRegistry = Core::Resolve<ResourcePathRegistry>();

    return true;
}

bool App::ResourcePatchExtension::Unload()
{
    Unhook<Raw::ResourceSerializer::Load>();
    Unhook<Raw::ResourceSerializer::Deserialize>();
    Unhook<Raw::ResourceSerializer::OnDependenciesReady>();
    Unhook<Raw::EntityTemplate::PostLoad>();
    Unhook<Raw::AppearanceResource::OnLoad>();
    Unhook<Raw::CMesh::PostLoad>();
    Unhook<Raw::MorphTargetMesh::PostLoad>();
    Unhook<Raw::EntityBuilder::ScheduleExtractComponentsJob>();
    Unhook<Raw::AppearanceDefinition::ExtractPartComponents>();
    Unhook<Raw::GarmentAssembler::ExtractComponentsJob>();
    Unhook<Raw::PersistencySystem::SetPersistentStateData>();

    return true;
}

void App::ResourcePatchExtension::Configure()
{
    s_patchTargets.clear();

    auto depot = Red::ResourceDepot::Get();

    Core::Map<Red::ResourcePath, std::string> knownPaths;
    Core::Set<Red::ResourcePath> invalidPaths;
    Core::Set<Red::ResourcePath> patchPaths;

    for (auto& config : m_configs)
    {
        for (const auto& [patchPath, patchConfig] : config.patches)
        {
            if (!depot->ResourceExists(patchPath))
            {
                if (!invalidPaths.contains(patchPath))
                {
                    LogError("[{}] Patch resource \"{}\" doesn't exist.", ExtensionName, config.paths[patchPath]);
                    invalidPaths.insert(patchPath);
                }
                continue;
            }

            Core::Set<Red::ResourcePath> targetList;

            for (const auto& includePath : patchConfig.includes)
            {
                const auto& includeList = ResourceMetaExtension::GetList(includePath);
                if (!includeList.empty())
                {
                    targetList.insert(includeList.begin(), includeList.end());
                }
                else
                {
                    targetList.insert(includePath);
                }
            }

            for (const auto& excludePath : patchConfig.excludes)
            {
                const auto& excludeList = ResourceMetaExtension::GetList(excludePath);
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
                s_patchTargets[targetPath].insert(patchPath);

                knownPaths.insert_or_assign(targetPath, config.paths[targetPath]);
            }

            knownPaths.insert_or_assign(patchPath, config.paths[patchPath]);
            patchPaths.insert(patchPath);

            s_patches[patchPath] = Core::MakeShared<ResourcePatch>(patchConfig);
        }
    }

    for (auto patch = s_patchTargets.begin(); patch != s_patchTargets.end();)
    {
        const auto& targetPath = patch->first;

        if (patchPaths.contains(targetPath))
        {
            if (!invalidPaths.contains(targetPath))
            {
                LogError(R"([{}] Patch resource "{}" cannot be target of another patch.)",
                         ExtensionName, knownPaths[targetPath]);
                invalidPaths.insert(targetPath);
            }

            patch = s_patchTargets.erase(patch);
            continue;
        }

        ++patch;
    }

    for (const auto& [knownPath, knownPathStr] : knownPaths)
    {
        s_resourcePathRegistry->RegisterPath(knownPath, knownPathStr);
    }

    m_configs.clear();
}

void App::ResourcePatchExtension::OnResourceRequest(void* aSerializer, uint64_t a2,
                                                    Red::ResourceSerializerRequest& aRequest, uint64_t a4, uint64_t a5)
{
    const auto& patchList = GetPatchList(aRequest.path);

    if (patchList.empty())
        return;

    for (const auto& patchPath : patchList)
    {
        LoadPatchResource(patchPath);
    }
}

void App::ResourcePatchExtension::OnResourceDeserialize(void* aSerializer, uint64_t, uint64_t, Red::JobHandle& aJob,
                                                        Red::ResourceSerializerRequest& aRequest, uint64_t,
                                                        Red::DynArray<Red::Handle<Red::ISerializable>>& aResults,
                                                        uint64_t)
{
    const auto& patchList = GetPatchList(aRequest.path);

    if (patchList.empty())
        return;

    for (const auto& patchPath : patchList)
    {
        auto patchToken = GetPatchToken(patchPath);
        if (patchToken)
        {
            aJob.Join(patchToken->job);
        }
    }
}

void App::ResourcePatchExtension::OnResourceReady(Red::ResourceSerializerContext* aContext)
{
    if (aContext->serializables.size > 0)
    {
        for (const auto& serializable : aContext->serializables)
        {
            if (const auto& resource = Red::Cast<Red::CurveSet>(serializable))
            {
                OnCurveSetResourceLoad(resource);
                continue;
            }

            if (const auto& resource = Red::Cast<Red::gameDeviceResource>(serializable))
            {
                OnDeviceResourceLoad(resource);
                continue;
            }
        }
    }
}

void App::ResourcePatchExtension::OnEntityTemplateLoad(Red::EntityTemplate* aTemplate, void*)
{
    const auto& patchList = GetPatchList(aTemplate->path);

    if (patchList.empty())
        return;

    auto depot = Red::ResourceDepot::Get();

    for (const auto& patchPath : patchList)
    {
        auto patchResource = GetPatchResource<Red::EntityTemplate>(patchPath);

        if (!patchResource)
            continue;

        const auto& patchConfig = GetPatchConfig(patchPath);

        if (patchConfig->Modifies(EntityTemplateAppearancesProp))
        {
            for (const auto& patchAppearance : patchResource->appearances)
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
        }

        if (patchConfig->Modifies(EntityTemplateDependenciesProp))
        {
            for (const auto& dependency : patchResource->resolvedDependencies)
            {
                if (!depot->ResourceExists(dependency.path))
                {
                    LogError(R"([{}] Patch resource "{}" refers to non-existent resource "{}".)",
                             ExtensionName, s_resourcePathRegistry->ResolvePathOrHash(patchResource->path),
                             s_resourcePathRegistry->ResolvePathOrHash(dependency.path));
                    continue;
                }

                auto isNewDependency =
                    std::none_of(aTemplate->resolvedDependencies.begin(), aTemplate->resolvedDependencies.end(),
                                 [&dependency](const Red::RaRef<Red::CResource>& aDependency) {
                                     return aDependency.path == dependency.path;
                                 });

                if (isNewDependency)
                {
                    aTemplate->resolvedDependencies.PushBack(dependency);
                }
            }
        }

        if (patchResource->visualTagsSchema && (patchConfig->Modifies(EntityTemplateVisualTagsProp)))
        {
            if (!aTemplate->visualTagsSchema)
            {
                aTemplate->visualTagsSchema = {};
            }

            aTemplate->visualTagsSchema->visualTags.Add(patchResource->visualTagsSchema->visualTags);
        }
    }
}

void App::ResourcePatchExtension::OnAppearanceResourceLoad(Red::AppearanceResource* aResource)
{
    const auto& patchList = GetPatchList(aResource->path);

    if (patchList.empty())
        return;

    auto depot = Red::ResourceDepot::Get();

    Core::Set<Red::CName> newAppearances;

    for (const auto& patchPath : patchList)
    {
        auto patchResource = GetPatchResource<Red::AppearanceResource>(patchPath);

        if (!patchResource)
            continue;

        const auto& patchConfig = GetPatchConfig(patchPath);

        if (patchConfig->Modifies(AppearanceResourceDefinitionsProp))
        {
            for (const auto& patchDefinition : patchResource->appearances)
            {
                const auto isMultiTargetPatch = patchDefinition->name.IsNone();
                auto isNewAppearance = !isMultiTargetPatch;

                for (auto& existingDefinition : aResource->appearances)
                {
                    if (!isMultiTargetPatch)
                    {
                        if (existingDefinition->name != patchDefinition->name)
                        {
                            continue;
                        }

                        isNewAppearance = false;

                        if (newAppearances.contains(patchDefinition->name))
                        {
                            break;
                        }
                    }

                    if (patchConfig->Modifies(AppearanceResourceComponentsProp))
                    {
                        std::unique_lock _(s_appearanceDefinitionLock);
                        if (!s_appearanceDefinitions[patchPath].contains(existingDefinition->name))
                        {
                            auto patchBufferToken = patchDefinition->compiledData.LoadAsync();
                            s_appearanceDefinitions[patchPath][existingDefinition->name] = {patchDefinition,
                                                                                            patchBufferToken};
                        }
                    }

                    if (patchConfig->Modifies(AppearanceResourcePartsValuesProp))
                    {
                        for (const auto& partValue : patchDefinition->partsValues)
                        {
                            if (!depot->ResourceExists(partValue.resource.path))
                            {
                                LogError(R"([{}] Patch resource "{}" refers to non-existent resource "{}".)",
                                         ExtensionName, s_resourcePathRegistry->ResolvePathOrHash(patchResource->path),
                                         s_resourcePathRegistry->ResolvePathOrHash(partValue.resource.path));
                                continue;
                            }

                            auto isNewPart =
                                std::none_of(existingDefinition->partsValues.begin(),
                                             existingDefinition->partsValues.end(),
                                             [&partValue](const Red::appearanceAppearancePart& aExistingPart) {
                                                 return aExistingPart.resource.path == partValue.resource.path;
                                             });

                            if (isNewPart)
                            {
                                existingDefinition->partsValues.PushBack(partValue);
                            }
                        }
                    }

                    if (patchConfig->Modifies(AppearanceResourcePartsOverridesProp))
                    {
                        for (const auto& partOverride : patchDefinition->partsOverrides)
                        {
                            existingDefinition->partsOverrides.PushBack(partOverride);
                        }
                    }

                    if (patchConfig->Modifies(AppearanceResourceDependenciesProp))
                    {
                        for (const auto& dependency : patchDefinition->resolvedDependencies)
                        {
                            if (!depot->ResourceExists(dependency.path))
                            {
                                LogError(R"([{}] Patch resource "{}" refers to non-existent resource "{}".)",
                                         ExtensionName, s_resourcePathRegistry->ResolvePathOrHash(patchResource->path),
                                         s_resourcePathRegistry->ResolvePathOrHash(dependency.path));
                                continue;
                            }

                            auto isNewDependency =
                                std::none_of(existingDefinition->resolvedDependencies.begin(),
                                             existingDefinition->resolvedDependencies.end(),
                                             [&dependency](const Red::RaRef<Red::CResource>& aDependency) {
                                                 return aDependency.path == dependency.path;
                                             });

                            if (isNewDependency)
                            {
                                existingDefinition->resolvedDependencies.PushBack(dependency);
                            }
                        }
                    }

                    if (patchConfig->Modifies(AppearanceResourceVisualTagsProp))
                    {
                        existingDefinition->visualTags.Add(patchDefinition->visualTags);
                    }

                    if (!isMultiTargetPatch)
                    {
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

        if (patchConfig->Modifies(AppearanceResourceCensorshipProp))
        {
            for (const auto& censorship : patchResource->censorshipMapping)
            {
                aResource->censorshipMapping.PushBack(censorship);
            }
        }
    }
}

void App::ResourcePatchExtension::OnMeshResourceLoad(Red::CMesh* aMesh, Red::PostLoadParams* aParams)
{
    const auto& fix = ResourceMetaExtension::GetFix(aMesh->path);

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

            if (!patchMesh)
                continue;

            const auto& patchConfig = GetPatchConfig(patchPath);

            if (patchMesh->appearances.size != 0 && (patchConfig->Modifies(MeshAppearancesProp)))
            {
                auto sourceTag = MeshExtension::RegisterMeshSource(aMesh, patchMesh);
                auto expansionTag = Red::CName();

                for (const auto& patchAppearance : patchMesh->appearances)
                {
                    auto cloneAppearance = Red::MakeHandle<Red::meshMeshAppearance>();
                    cloneAppearance->name = patchAppearance->name;
                    cloneAppearance->chunkMaterials = patchAppearance->chunkMaterials;

                    if (patchAppearance->tags.size == 1)
                    {
                        expansionTag = patchAppearance->tags[0];
                    }

                    cloneAppearance->tags.PushBack(ResourcePatchTag);
                    cloneAppearance->tags.PushBack(expansionTag);
                    cloneAppearance->tags.PushBack(sourceTag);

                    auto isNewAppearance = true;

                    for (auto& existingAppearance : aMesh->appearances)
                    {
                        if (existingAppearance->name == cloneAppearance->name)
                        {
                            if (existingAppearance->chunkMaterials.size == 0 ||
                                cloneAppearance->chunkMaterials.size != 0)
                            {
                                existingAppearance = cloneAppearance;
                            }

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

            if (patchMesh->renderResourceBlob && patchConfig->Modifies(MeshBlobProp, aMesh->renderResourceBlob))
            {
                if (auto& renderBlob = Red::Cast<Red::rendRenderMeshBlob>(patchMesh->renderResourceBlob))
                {
                    aMesh->renderResourceBlob = CopyRenderBlob(renderBlob);
                    // aMesh->parameters = patchMesh->parameters;
                    aMesh->boneNames = patchMesh->boneNames;
                    aMesh->boneRigMatrices = patchMesh->boneRigMatrices;
                    aMesh->boneVertexEpsilons = patchMesh->boneVertexEpsilons;
                    aMesh->lodBoneMask = patchMesh->lodBoneMask;
                    aMesh->lodLevelInfo = patchMesh->lodLevelInfo;
                    aMesh->floatTrackNames = patchMesh->floatTrackNames;
                    aMesh->boundingBox = patchMesh->boundingBox;
                    aMesh->surfaceAreaPerAxis = patchMesh->surfaceAreaPerAxis;
                    aMesh->objectType = patchMesh->objectType;
                    aMesh->castGlobalShadowsCachedInCook = patchMesh->castGlobalShadowsCachedInCook;
                    aMesh->castLocalShadowsCachedInCook = patchMesh->castLocalShadowsCachedInCook;
                    aMesh->useRayTracingShadowLODBias = patchMesh->useRayTracingShadowLODBias;
                    aMesh->castsRayTracedShadowsFromOriginalGeometry = patchMesh->castsRayTracedShadowsFromOriginalGeometry;
                    aMesh->isShadowMesh = patchMesh->isShadowMesh;
                    aMesh->isPlayerShadowMesh = patchMesh->isPlayerShadowMesh;
                    aMesh->constrainAutoHideDistanceToTerrainHeightMap = patchMesh->constrainAutoHideDistanceToTerrainHeightMap;
                }
            }
        }
    }

    MeshExtension::PrefetchMeshState(aMesh, fix.GetContext());
}

void App::ResourcePatchExtension::OnMorphTargetResourceLoad(Red::MorphTargetMesh* aMorphTarget,
                                                            Red::PostLoadParams* aParams)
{
    if (IsPatchResource(aMorphTarget->path))
        return;

    const auto& patchList = GetPatchList(aMorphTarget->path);

    for (const auto& patchPath : patchList)
    {
        auto patchResource = GetPatchResource<Red::MorphTargetMesh>(patchPath);

        if (!patchResource)
            continue;

        const auto& patchConfig = GetPatchConfig(patchPath);

        if (patchResource->baseMesh.path && (patchConfig->Modifies(MorphTargetMeshProp)))
        {
            aMorphTarget->baseMesh = patchResource->baseMesh;
        }

        if (patchConfig->Modifies(MorphTargetMeshAppProp, !patchResource->baseMeshAppearance))
        {
            aMorphTarget->baseMeshAppearance = patchResource->baseMeshAppearance;
        }

        if (patchConfig->Modifies(MorphTargetTextureProp, !patchResource->baseTexture.path))
        {
            aMorphTarget->baseTexture = patchResource->baseTexture;
        }

        if (patchConfig->Modifies(MorphTargetTextureParamProp, !patchResource->baseTextureParamName))
        {
            aMorphTarget->baseTextureParamName = patchResource->baseTextureParamName;
        }

        if (patchResource->blob && patchConfig->Modifies(MorphTargetBlobProp, aMorphTarget->blob))
        {
            if (auto& renderBlob = Red::Cast<Red::rendRenderMorphTargetMeshBlob>(patchResource->blob))
            {
                aMorphTarget->blob = CopyRenderBlob(renderBlob);
            }
        }

        if (patchResource->boundingBox.Max.X > patchResource->boundingBox.Min.X
            && (patchConfig->Modifies(MorphTargetBoundingBoxProp)))
        {
            aMorphTarget->boundingBox = patchResource->boundingBox;
        }

        if (patchConfig->Modifies(MorphTargetTargetsProp))
        {
            for (const auto& patchTarget : patchResource->targets)
            {
                auto isNewTarget = true;

                for (auto& existingTarget : aMorphTarget->targets)
                {
                    if (existingTarget.name == patchTarget.name)
                    {
                        existingTarget = patchTarget;
                        isNewTarget = false;
                        break;
                    }
                }

                if (isNewTarget)
                {
                    aMorphTarget->targets.EmplaceBack(patchTarget);
                }
            }
        }
    }

    Raw::MorphTargetMesh::PostLoad(aMorphTarget, aParams);
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
                if (appearance.extractor)
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
                if (appearance.extractor)
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
        auto patchResource = GetPatchResource<Red::EntityTemplate>(patchPath);

        if (!patchResource)
            continue;

        const auto& patchConfig = GetPatchConfig(patchPath);

        if (patchConfig->Modifies(EntityTemplateComponentsProp))
        {
            aParams->partTemplate = patchResource;
            Raw::GarmentAssembler::ExtractComponentsJob(aParams, aJobGroup);
        }
    }

    aParams->partTemplate = originalEntityTemplate;
}

void App::ResourcePatchExtension::OnCurveSetResourceLoad(Red::CurveSet* aResource)
{
    const auto& patchList = GetPatchList(aResource->path);

    if (patchList.empty())
        return;

    for (const auto& patchPath : patchList)
    {
        auto patchResource = GetPatchResource<Red::CurveSet>(patchPath);

        if (!patchResource)
            continue;

        for (const auto& patchEntry : patchResource->curves)
        {
            auto isNewEntry = true;

            for (auto& existingEntry : aResource->curves)
            {
                if (existingEntry.name == patchEntry.name)
                {
                    isNewEntry = false;

                    if (existingEntry.curve.valueType == patchEntry.curve.valueType)
                    {
                        existingEntry.curve.interpolationType = patchEntry.curve.interpolationType;
                        existingEntry.curve.linkType = patchEntry.curve.linkType;
                        existingEntry.curve.Resize(patchEntry.curve.GetSize());
                        std::memcpy(existingEntry.curve.buffer.data, patchEntry.curve.buffer.data,
                                    patchEntry.curve.buffer.size);
                    }

                    break;
                }
            }

            if (isNewEntry)
            {
                aResource->curves.EmplaceBack();
                auto& newEntry = aResource->curves.Back();
                newEntry.name = patchEntry.name;
                newEntry.curve.name = patchEntry.curve.name;
                newEntry.curve.valueType = patchEntry.curve.valueType;
                newEntry.curve.interpolationType = patchEntry.curve.interpolationType;
                newEntry.curve.linkType = patchEntry.curve.linkType;
                newEntry.curve.Resize(patchEntry.curve.GetSize());
                std::memcpy(newEntry.curve.buffer.data, patchEntry.curve.buffer.data, patchEntry.curve.buffer.size);
            }
        }
    }
}

void App::ResourcePatchExtension::OnDeviceResourceLoad(Red::gameDeviceResource* aResource)
{
    const auto& patchList = GetPatchList(aResource->path);

    if (patchList.empty())
        return;

    auto deviceMap = std::bit_cast<Red::HashMap<uint64_t, Red::gameCookedDeviceData>*>(&aResource->data->unk30);

    for (const auto& patchPath : patchList)
    {
        auto patchResource = GetPatchResource<Red::gameDeviceResource>(patchPath);

        if (!patchResource)
            continue;

        auto patchMap = std::bit_cast<Red::HashMap<uint64_t, Red::gameCookedDeviceData>*>(&patchResource->data->unk30);

        patchMap->ForEach([&deviceMap](const uint64_t& aKey, const Red::gameCookedDeviceData& aData)
                          {
                              deviceMap->InsertOrAssign(aKey, aData);
                          });
    }
}

void App::ResourcePatchExtension::OnSetPersistentStateData(uint64_t a1, Red::DataBuffer& aData, uint64_t a3,
                                                           uint32_t a4)
{
    auto streamingSystem = Red::GetRuntimeSystem<Red::worldRuntimeSystemWorldStreaming>();
    auto& streamingWorld = Raw::RuntimeSystemWorldStreaming::StreamingWorld::Ref(streamingSystem);

    const auto& patchList = GetPatchList(streamingWorld->persistentStateData.path);

    if (patchList.empty())
        return;

    for (const auto& patchPath : patchList)
    {
        auto patchResource = GetPatchResource<Red::gamePersistentStateDataResource>(patchPath);

        if (!patchResource)
            continue;

        Raw::PersistencySystem::SetPersistentStateData(a1, patchResource->buffer, a3, a4);
    }
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
                MergeComponents(aResultObjects, partExtractor.results, true);

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
        auto patchResource = GetPatchResource<Red::EntityTemplate>(patchPath);

        if (!patchResource)
            continue;

        const auto& patchConfig = GetPatchConfig(patchPath);

        if (!patchConfig->Modifies(EntityTemplateEntityProp) &&
            !patchConfig->Modifies(EntityTemplateComponentsProp))
            continue;

        auto& patchBuffer = patchResource->compiledData;

        if (!patchBuffer.buffer.size)
            continue;

        auto& patchHeader = patchResource->compiledDataHeader;

        if (patchHeader.IsEmpty())
        {
            // auto patchReader = Red::ObjectPackageReader(patchBuffer);
            // patchReader.ReadHeader(patchHeader);
            //
            // if (patchHeader.IsEmpty())
            //     continue;

            continue;
        }

        auto patchExtractor = Red::ObjectPackageExtractor(patchHeader);
        patchExtractor.disablePostLoad = aDisablePostLoad;
        patchExtractor.disableImports = aDisableImports;
        patchExtractor.disablePreInitialization = aDisablePreInitialization;
        patchExtractor.ExtractSync();

        if (patchExtractor.results.size > 0)
        {
            if (patchConfig->Modifies(EntityTemplateComponentsProp))
            {
                MergeComponents(aResultObjects, patchExtractor.results, false);
            }

            if (patchConfig->Modifies(EntityTemplateEntityProp))
            {
                MergeEntity(aResultObjects, patchExtractor.results, templateHeader.rootIndex, patchHeader.rootIndex);
            }
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
        auto patchDefinition = GetPatchAppearance(patchPath, aDefinition->name);

        if (!patchDefinition)
            continue;

        auto patchBufferToken = patchDefinition->compiledData.LoadAsync();

        jobQueue.Wait(patchBufferToken->job);
        jobQueue.Dispatch([patchDefinition = std::move(patchDefinition), &aResultObjects, aDisablePostLoad,
                           aDisableImports, aDisablePreInitialization](const Red::JobGroup& aJobGroup) {
            auto patchReader = Red::ObjectPackageReader(patchDefinition->compiledData);
            patchReader.ReadHeader(patchDefinition->compiledDataHeader);

            auto patchExtractor = Red::MakeShared<Red::ObjectPackageExtractor>(patchDefinition->compiledDataHeader);
            patchExtractor->disablePostLoad = aDisablePostLoad;
            patchExtractor->disableImports = aDisableImports;
            patchExtractor->disablePreInitialization = aDisablePreInitialization;

            auto patchExtractionJob = patchExtractor->ExtractAsync();

            Red::JobQueue jobQueue{aJobGroup};
            jobQueue.Wait(patchExtractionJob);
            jobQueue.Dispatch([patchExtractor = std::move(patchExtractor), &aResultObjects]() {
                if (patchExtractor->results.size > 0)
                {
                    MergeComponents(aResultObjects, patchExtractor->results, false);
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
                                                  Red::DynArray<Red::Handle<Red::ISerializable>>& aPatchObjects,
                                                  bool aPartMerge)
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

            if (isNewComponent && (!aPartMerge || !Red::IsInstanceOf<Red::entExternalComponent>(patchComponent)))
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

Red::Handle<Red::rendRenderMeshBlob> App::ResourcePatchExtension::CopyRenderBlob(
    const Red::Handle<Red::rendRenderMeshBlob>& aSourceBlob)
{
    auto cloneBlob = Red::MakeHandle<Red::rendRenderMeshBlob>();
    cloneBlob->header = aSourceBlob->header;

    Red::CopyBuffer(cloneBlob->renderBuffer, aSourceBlob->renderBuffer);

    return cloneBlob;
}

Red::Handle<Red::rendRenderMorphTargetMeshBlob> App::ResourcePatchExtension::CopyRenderBlob(
    const Red::Handle<Red::rendRenderMorphTargetMeshBlob>& aSourceBlob)
{
    auto cloneBlob = Red::MakeHandle<Red::rendRenderMorphTargetMeshBlob>();

    if (auto& baseBlob = Red::Cast<Red::rendRenderMeshBlob>(aSourceBlob->baseBlob))
    {
        cloneBlob->header = aSourceBlob->header;
        cloneBlob->baseBlob = CopyRenderBlob(baseBlob);

        Red::CopyBuffer(cloneBlob->diffsBuffer, aSourceBlob->diffsBuffer);
        Red::CopyBuffer(cloneBlob->mappingBuffer, aSourceBlob->mappingBuffer);
        Red::CopyBuffer(cloneBlob->textureDiffsBuffer, aSourceBlob->textureDiffsBuffer);
    }

    return cloneBlob;
}

Core::SharedPtr<App::ResourcePatch> App::ResourcePatchExtension::GetPatchConfig(
    Red::ResourcePath aPatchPath)
{
    const auto& patchIt = s_patches.find(aPatchPath);

    if (patchIt == s_patches.end())
        return {};

    return patchIt.value();
}

const Core::Set<Red::ResourcePath>& App::ResourcePatchExtension::GetPatchList(Red::ResourcePath aTargetPath)
{
    static const Core::Set<Red::ResourcePath> s_null;

    const auto& patchIt = s_patchTargets.find(aTargetPath);

    if (patchIt == s_patchTargets.end())
        return s_null;

    return patchIt.value();
}

void App::ResourcePatchExtension::LoadPatchResource(Red::ResourcePath aPatchPath)
{
    std::unique_lock _(s_patchTokenLock);

    if (s_patchTokens.contains(aPatchPath))
        return;

    // Red::ResourceRequest patchRequest;
    // patchRequest.path = aPatchPath;
    // patchRequest.disablePreInitialization = true;
    // patchRequest.disablePostLoad = true;
    // s_tokens[aPatchPath] = Red::ResourceLoader::Get()->LoadAsync(patchRequest);

    s_patchTokens[aPatchPath] = Red::ResourceLoader::Get()->LoadAsync(aPatchPath);
}

bool App::ResourcePatchExtension::IsPatchResource(Red::ResourcePath aPath)
{
    std::shared_lock _(s_patchTokenLock);

    return s_patchTokens.contains(aPath);
}

template<typename T>
Red::SharedPtr<Red::ResourceToken<T>> App::ResourcePatchExtension::GetPatchToken(Red::ResourcePath aPatchPath)
{
    std::shared_lock _(s_patchTokenLock);

    auto& token = s_patchTokens[aPatchPath];

    if constexpr (!std::is_same_v<T, Red::CResource>)
    {
        if (token->IsFailed())
        {
            LogError("[{}] Patch resource \"{}\" failed to load.",
                     ExtensionName, s_resourcePathRegistry->ResolvePathOrHash(token->path));
            return {};
        }

        if (!token->IsFinished())
        {
            Red::WaitForResource(token, std::chrono::milliseconds(250));

            if (!token->IsFinished())
            {
                Red::WaitForResource(token, std::chrono::milliseconds(250));

                if (!token->IsFinished())
                {
                    LogError("[{}] Patch resource \"{}\" failed to loaded due to timeout.",
                             ExtensionName, s_resourcePathRegistry->ResolvePathOrHash(token->path));
                    return {};
                }
            }
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

Red::Handle<Red::AppearanceDefinition> App::ResourcePatchExtension::GetPatchAppearance(
    Red::ResourcePath aPatchPath, Red::CName aDefinitionName)
{
    std::shared_lock _(s_appearanceDefinitionLock);

    const auto& resourceIt = s_appearanceDefinitions.find(aPatchPath);
    if (resourceIt == s_appearanceDefinitions.end())
        return {};

    const auto& definitionIt = resourceIt.value().find(aDefinitionName);
    if (definitionIt == resourceIt.value().end())
        return {};

    return definitionIt.value().first;
}

bool App::ResourcePatchExtension::IsPatched(const Red::Handle<Red::meshMeshAppearance>& aAppearance)
{
    return aAppearance->tags.size == 3 && aAppearance->tags[0] == ResourcePatchTag;
}

Red::CName App::ResourcePatchExtension::GetExpansionName(const Red::Handle<Red::meshMeshAppearance>& aAppearance)
{
    return aAppearance->tags.size == 3 ? aAppearance->tags[1] : "";
}

Red::CName App::ResourcePatchExtension::GetPatchSource(const Red::Handle<Red::meshMeshAppearance>& aAppearance)
{
    return aAppearance->tags.size == 3 ? aAppearance->tags[2] : "";
}
