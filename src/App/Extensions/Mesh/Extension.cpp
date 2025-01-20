#include "Extension.hpp"
#include "App/Extensions/Garment/Extension.hpp"
#include "App/Utils/Str.hpp"
#include "Core/Facades/Container.hpp"

namespace
{
constexpr auto ExtensionName = "DynamicMesh";

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
    Red::CNamePool::Add("@material");

    s_resourcePathRegistry = Core::Resolve<ResourcePathRegistry>();

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

void App::MeshExtension::OnFindAppearance(Red::Handle<Red::meshMeshAppearance>& aAppearance, Red::CMesh* aMesh,
                                          Red::CName aName)
{
    if (!aAppearance)
        return;

    if (!Raw::MeshAppearance::Owner::Ptr(aAppearance))
    {
        aAppearance = s_dummyAppearance;
        return;
    }

    if (!aAppearance->name || aMesh->appearances.size == 0)
        return;

    if (aAppearance->chunkMaterials.size > 0 && aAppearance->tags.size == 0)
        return;

    auto meshState = AcquireMeshState(aMesh);

    std::unique_lock _(meshState->meshMutex);

    if (aAppearance->chunkMaterials.size == 0)
    {
        auto sourceIndex = meshState->GetExpansionSourceIndex(aMesh);
        auto sourceAppearance = aMesh->appearances[sourceIndex];

        if (sourceAppearance && sourceAppearance != aAppearance)
        {
            const auto appearanceNameStr = std::string{aAppearance->name.ToString()};
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
                    chunkMaterialName = aAppearance->name;
                }

                aAppearance->chunkMaterials.PushBack(chunkMaterialName);
            }

            auto meshPathStr = s_resourcePathRegistry->ResolvePathOrHash(aMesh->path);

            LogInfo(R"([{}] Appearance "{}" of "{}" has been expanded using "{}".)",
                    ExtensionName, aAppearance->name.ToString(), meshPathStr, sourceAppearance->name.ToString());
        }
        else if (aAppearance->name != DefaultAppearanceName)
        {
            auto meshPathStr = s_resourcePathRegistry->ResolvePathOrHash(aMesh->path);

            LogWarning(R"([{}] Appearance "{}" of "{}" has no chunk materials and cannot be expanded.)",
                       ExtensionName, aAppearance->name.ToString(), meshPathStr);
        }
    }

    if (aAppearance->chunkMaterials.size > 0 && aAppearance->tags.size == 1)
    {
        aAppearance->chunkMaterials.PushBack(aAppearance->tags[0]);
        aAppearance->tags.Clear();
    }
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
    if (!aMesh || !aMesh->path || aMesh->appearances.size == 0 || aMesh->materialEntries.size == 0)
        return false;

    auto result = Raw::CMesh::ShouldPreloadAppearances(aMesh);

    if (result && !aMesh->forceLoadAllAppearances && aMesh->appearances.size == 1)
    {
        if (aMesh->appearances[0]->chunkMaterials.size == 0 || IsSpecialMaterial(aMesh->materialEntries[0].name))
        {
            result = false;
        }
    }

    return result;
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
                auto context = Core::MakeShared<DynamicContext>(targetMeshState, sourceMeshState, targetMesh,
                                                                sourceMesh, materialNames, finalMaterials);
                ProcessDynamicMaterials(context, aJobGroup);
            }
        });

        targetMeshState->lastJob = std::move(jobQueue.Capture());
    });

    aToken.job = std::move(jobQueue.Capture());

    return &aToken;
}

void App::MeshExtension::ProcessDynamicMaterials(const Core::SharedPtr<DynamicContext>& aContext,
                                                 const Red::JobGroup& aJobGroup)
{
    Red::JobQueue jobQueue(aJobGroup);

    std::scoped_lock _(aContext->targetState->meshMutex, aContext->sourceState->sourceMutex);

    aContext->targetState->FillMaterials(aContext->targetMesh);

    if (aContext->sourceState != aContext->targetState)
    {
        aContext->sourceState->FillMaterials(aContext->sourceMesh);
    }

    for (int32_t chunkIndex = 0; chunkIndex < aContext->materialNames.size; ++chunkIndex)
    {
        const auto& chunkName = aContext->materialNames[chunkIndex];

        if (aContext->targetState->HasMaterialEntry(chunkName))
            continue;

        if (chunkName.hash == aContext->sourceMesh->path.hash)
        {
            auto materialIndex = static_cast<int32_t>(aContext->targetMesh->materialEntries.size);
            aContext->targetState->materials[chunkName] = materialIndex;
            aContext->targetMesh->materialEntries.EmplaceBack();

            auto& materialEntry = aContext->targetMesh->materialEntries.Back();
            materialEntry.name = chunkName;
            materialEntry.isLocalInstance = true;
            materialEntry.material = s_dummyMaterial;
            materialEntry.materialWeak = s_dummyMaterial;

            continue;
        }

        // if (aContext->sourceState == aContext->targetState)
        // {
        //     const auto meshPathStr = s_resourcePathRegistry->ResolvePathOrHash(aContext->targetMesh->path);
        //
        //     LogInfo(R"([{}] Resolving material "{}" for "{}"...)",
        //             ExtensionName, chunkName.ToString(), meshPathStr);
        // }
        // else
        // {
        //     const auto meshPathStr = s_resourcePathRegistry->ResolvePathOrHash(aContext->targetMesh->path);
        //     const auto sourcePathStr = s_resourcePathRegistry->ResolvePathOrHash(aContext->sourceMesh->path);
        //
        //     LogInfo(R"([{}] Resolving material "{}" for "{}" using source "{}"...)",
        //             ExtensionName, chunkName.ToString(), meshPathStr, sourcePathStr);
        // }

        auto chunk = Core::MakeShared<ChunkData>();
        chunk->chunkName = chunkName;
        chunk->materialName = chunkName;
        chunk->templateName = chunkName;
        chunk->sourceIndex = aContext->sourceState->GetMaterialEntryIndex(chunkName);

        if (chunk->sourceIndex < 0)
        {
            auto chunkMaterialNameStr = std::string_view(chunkName.ToString());
            auto templateNamePos = chunkMaterialNameStr.find(SpecialMaterialMarker);
            if (templateNamePos != std::string_view::npos)
            {
                std::string templateNameStr{chunkMaterialNameStr.data() + templateNamePos,
                                            chunkMaterialNameStr.size() - templateNamePos};
                std::string materialNameStr{chunkMaterialNameStr.data(), templateNamePos};

                chunk->templateName = Red::CNamePool::Add(templateNameStr.data());
                chunk->materialName = Red::CNamePool::Add(materialNameStr.data());
            }
            else
            {
                chunk->templateName = DefaultTemplateName;
            }

            chunk->sourceIndex = aContext->sourceState->GetTemplateEntryIndex(chunk->templateName);

            if (chunk->sourceIndex < 0)
            {
                const auto sourcePathStr = s_resourcePathRegistry->ResolvePathOrHash(aContext->sourceMesh->path);

                LogError(R"([{}] Material "{}" of "{}" is not defined and cannot be dynamically instantiated, material template "{}" doesn't exist.)",
                         ExtensionName, chunkName.ToString(), sourcePathStr, chunk->templateName.ToString());
                continue;
            }
        }

        auto& sourceEntry = aContext->sourceMesh->materialEntries[chunk->sourceIndex];

        chunk->sourceInstance = Red::Cast<Red::CMaterialInstance>(sourceEntry.materialWeak.Lock());

        if (!chunk->sourceInstance)
        {
            if (sourceEntry.isLocalInstance)
            {
                Raw::MeshMaterialBuffer::LoadMaterialAsync(&aContext->sourceMesh->localMaterialBuffer,
                                                           chunk->sourceToken, aContext->sourceMesh,
                                                           sourceEntry.index, 0, 0);

                if (!chunk->sourceToken || chunk->sourceToken->IsFailed())
                {
                    const auto sourcePathStr = s_resourcePathRegistry->ResolvePathOrHash(aContext->sourceMesh->path);

                    LogError(R"([{}] Source material "{}" of "{}" failed to load: not found in buffer.)",
                             ExtensionName, chunk->templateName.ToString(), sourcePathStr);
                    continue;
                }
            }
            else
            {
                auto externalPath = aContext->sourceMesh->externalMaterials[sourceEntry.index].path;
                auto materialPath = ExpandResourcePath(externalPath, chunk->materialName, aContext->targetState);

                if (!materialPath)
                {
                    const auto sourcePathStr = s_resourcePathRegistry->ResolvePathOrHash(aContext->sourceMesh->path);
                    const auto externalPathStr = s_resourcePathRegistry->ResolvePathOrHash(externalPath);

                    LogError(R"([{}] Source material "{}" of "{}" failed to load: unresolvable external path "{}".)",
                             ExtensionName, chunk->templateName.ToString(), sourcePathStr, externalPathStr);
                    continue;
                }

                chunk->sourceToken = Red::ResourceLoader::Get()->LoadAsync<Red::IMaterial>(materialPath);

                if (!chunk->sourceToken || chunk->sourceToken->IsFailed())
                {
                    const auto sourcePathStr = s_resourcePathRegistry->ResolvePathOrHash(aContext->sourceMesh->path);
                    const auto externalPathStr = s_resourcePathRegistry->ResolvePathOrHash(materialPath);

                    LogError(R"([{}] Source material "{}" of "{}" failed to load: invalid external path "{}".)",
                             ExtensionName, chunk->templateName.ToString(), sourcePathStr, externalPathStr);
                    continue;
                }
            }

            if (!chunk->sourceToken->IsLoaded())
            {
                aContext->deferredMaterials.push_back(chunk);
                continue;
            }

            chunk->sourceInstance = Red::Cast<Red::CMaterialInstance>(chunk->sourceToken->resource);

            if (!chunk->sourceInstance)
            {
                const auto sourcePathStr = s_resourcePathRegistry->ResolvePathOrHash(aContext->sourceMesh->path);

                LogError(R"([{}] Source material "{}" of "{}" must be instance of {}, got {}.)", ExtensionName,
                         chunk->templateName.ToString(), sourcePathStr, Red::CMaterialInstance::NAME,
                         chunk->sourceToken->resource->GetType()->name.ToString());
                continue;
            }

            sourceEntry.material = chunk->sourceInstance;
            sourceEntry.materialWeak = chunk->sourceInstance;
        }

        FinalizeDynamicMaterial(aContext, chunk, jobQueue);
    }

    if (aContext->deferredMaterials.empty())
    {
        FixFinalMaterialsIndexes(aContext);
        return;
    }

    for (auto& chunk : aContext->deferredMaterials)
    {
        jobQueue.Wait(chunk->sourceToken->job);
    }

    jobQueue.Dispatch([aContext](const Red::JobGroup& aJobGroup) {
        std::scoped_lock _(aContext->targetState->meshMutex, aContext->sourceState->sourceMutex);

        Red::JobQueue jobQueue(aJobGroup);

        for (auto& chunk : aContext->deferredMaterials)
        {
            if (chunk->sourceToken->IsFailed())
            {
                const auto sourcePathStr = s_resourcePathRegistry->ResolvePathOrHash(aContext->sourceMesh->path);

                if (chunk->sourceToken->path)
                {
                    const auto externalPathStr = s_resourcePathRegistry->ResolvePathOrHash(chunk->sourceToken->path);

                    LogError(R"([{}] Source material "{}" of "{}" failed to load: invalid external path "{}".)",
                             ExtensionName, chunk->templateName.ToString(), sourcePathStr, externalPathStr);
                }
                else
                {
                    LogError(R"([{}] Source material "{}" of "{}" failed to load: cannot read from buffer.)",
                             ExtensionName, chunk->templateName.ToString(), sourcePathStr);
                }
                continue;
            }

            chunk->sourceInstance = Red::Cast<Red::CMaterialInstance>(chunk->sourceToken->resource);

            if (!chunk->sourceInstance)
            {
                const auto sourcePathStr = s_resourcePathRegistry->ResolvePathOrHash(aContext->sourceMesh->path);

                LogError(R"([{}] Source material "{}" of "{}" must be instance of {}, got {}.)", ExtensionName,
                         chunk->templateName.ToString(), sourcePathStr, Red::CMaterialInstance::NAME,
                         chunk->sourceToken->resource->GetType()->name.ToString());
                continue;
            }

            auto& sourceEntry = aContext->sourceMesh->materialEntries[chunk->sourceIndex];
            sourceEntry.material = chunk->sourceInstance;
            sourceEntry.materialWeak = chunk->sourceInstance;

            FinalizeDynamicMaterial(aContext, chunk, jobQueue);
        }

        FixFinalMaterialsIndexes(aContext);
    });
}

void App::MeshExtension::FinalizeDynamicMaterial(const Core::SharedPtr<DynamicContext>& aContext,
                                                 const Core::SharedPtr<ChunkData>& aChunk, Red::JobQueue& aJobQueue)
{
    auto materialIndex = static_cast<int32_t>(aContext->targetMesh->materialEntries.size);
    aContext->targetState->materials[aChunk->chunkName] = materialIndex;
    aContext->targetMesh->materialEntries.EmplaceBack();

    auto& materialEntry = aContext->targetMesh->materialEntries.Back();
    materialEntry.name = aChunk->chunkName;
    materialEntry.isLocalInstance = true;

    if (aChunk->materialName == aChunk->templateName)
    {
        materialEntry.material = aChunk->sourceInstance;
        materialEntry.materialWeak = aChunk->sourceInstance;

        if (aContext->sourceState == aContext->targetState)
        {
            const auto meshPathStr = s_resourcePathRegistry->ResolvePathOrHash(aContext->targetMesh->path);

            LogInfo(R"([{}] Material "{}" of "{}" has been successfully instantiated.)",
                    ExtensionName, aChunk->chunkName.ToString(), meshPathStr);
        }
        else
        {
            const auto meshPathStr = s_resourcePathRegistry->ResolvePathOrHash(aContext->targetMesh->path);
            const auto sourcePathStr = s_resourcePathRegistry->ResolvePathOrHash(aContext->sourceMesh->path);

            LogInfo(R"([{}] Material "{}" of "{}" has been successfully instantiated using source "{}".)",
                    ExtensionName, aChunk->chunkName.ToString(), meshPathStr, sourcePathStr);
        }
    }
    else
    {
        auto materialInstance = CloneMaterialInstance(aContext, aChunk, aChunk->sourceInstance, aJobQueue);

        materialEntry.material = materialInstance;
        materialEntry.materialWeak = materialInstance;

        aJobQueue.Dispatch([aContext, aChunk, materialInstance]() {
            if (aChunk->failed)
            {
                materialInstance->baseMaterial = {};
                return;
            }

            if (aContext->sourceState == aContext->targetState)
            {
                const auto meshPathStr = s_resourcePathRegistry->ResolvePathOrHash(aContext->targetMesh->path);

                LogInfo(R"([{}] Material "{}" of "{}" has been successfully instantiated.)",
                        ExtensionName, aChunk->chunkName.ToString(), meshPathStr);
            }
            else
            {
                const auto meshPathStr = s_resourcePathRegistry->ResolvePathOrHash(aContext->targetMesh->path);
                const auto sourcePathStr = s_resourcePathRegistry->ResolvePathOrHash(aContext->sourceMesh->path);

                LogInfo(R"([{}] Material "{}" of "{}" has been successfully instantiated using source "{}".)",
                        ExtensionName, aChunk->chunkName.ToString(), meshPathStr, sourcePathStr);
            }
        });
    }
}

Red::Handle<Red::CMaterialInstance> App::MeshExtension::CloneMaterialInstance(
    const Core::SharedPtr<DynamicContext>& aContext, const Core::SharedPtr<ChunkData>& aChunk,
    const Red::Handle<Red::CMaterialInstance>& aSourceInstance, Red::JobQueue& aJobQueue)
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

    if (aSourceInstance == aChunk->sourceInstance)
    {
        for (const auto& contextParam : aContext->targetState->GetContextParams())
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

    ExpandMaterialParams(aContext, aChunk, materialInstance, aJobQueue);
    ExpandMaterialInheritance(aContext, aChunk, materialInstance, aJobQueue);

    return materialInstance;
}

void App::MeshExtension::ExpandMaterialParams(const Core::SharedPtr<DynamicContext>& aContext,
                                              const Core::SharedPtr<ChunkData>& aChunk,
                                              const Red::Handle<Red::CMaterialInstance>& aMaterialInstance,
                                              Red::JobQueue& aJobQueue)
{
    for (auto i = static_cast<int32_t>(aMaterialInstance->params.size) - 1; i >= 0; --i)
    {
        const auto& param = aMaterialInstance->params[i];

        if (param.data.GetType()->GetType() != Red::ERTTIType::ResourceReference)
            continue;

        auto& reference = *reinterpret_cast<Red::ResourceReference<>*>(param.data.GetDataPtr());

        if (reference.token)
        {
            if (reference.token->IsFailed())
            {
                const auto meshPathStr = s_resourcePathRegistry->ResolvePathOrHash(aContext->targetMesh->path);

                if (aMaterialInstance->path)
                {
                    const auto materialPathStr = s_resourcePathRegistry->ResolvePathOrHash(aMaterialInstance->path);

                    LogError(R"([{}] Material "{}" of "{}" failed to instantiate: invalid reference for param "{}" in "{}".)",
                             ExtensionName, aChunk->chunkName.ToString(), meshPathStr, param.name.ToString(), materialPathStr);
                }
                else
                {
                    LogError(R"([{}] Material "{}" of "{}" failed to instantiate: invalid reference for param "{}".)",
                             ExtensionName, aChunk->chunkName.ToString(), meshPathStr, param.name.ToString());
                }

                aChunk->failed = true;
            }
            continue;
        }

        auto referencePath = ExpandResourcePath(reference.path, aChunk->materialName, aContext->targetState);

        if (!referencePath)
        {
            const auto meshPathStr = s_resourcePathRegistry->ResolvePathOrHash(aContext->targetMesh->path);
            const auto paramPathStr = s_resourcePathRegistry->ResolvePathOrHash(reference.path);

            if (aMaterialInstance->path)
            {
                const auto materialPathStr = s_resourcePathRegistry->ResolvePathOrHash(aMaterialInstance->path);

                LogError(R"([{}] Material "{}" of "{}" failed to instantiate: unresolvable path "{}" for param "{}" in "{}".)",
                         ExtensionName, aChunk->chunkName.ToString(), meshPathStr, paramPathStr, param.name.ToString(), materialPathStr);
            }
            else
            {
                LogError(R"([{}] Material "{}" of "{}" failed to instantiate: unresolvable path "{}" for param "{}".)",
                         ExtensionName, aChunk->chunkName.ToString(), meshPathStr, paramPathStr, param.name.ToString());
            }

            aChunk->failed = true;
            continue;
        }

        reference.path = referencePath;
        reference.LoadAsync();

        if (!reference.token || reference.token->IsFailed())
        {
            const auto meshPathStr = s_resourcePathRegistry->ResolvePathOrHash(aContext->targetMesh->path);
            const auto paramPathStr = s_resourcePathRegistry->ResolvePathOrHash(referencePath);

            if (aMaterialInstance->path)
            {
                const auto materialPathStr = s_resourcePathRegistry->ResolvePathOrHash(aMaterialInstance->path);

                LogError(R"([{}] Material "{}" of "{}" failed to instantiate: invalid path "{}" for param "{}" in "{}".)",
                         ExtensionName, aChunk->chunkName.ToString(), meshPathStr, paramPathStr, param.name.ToString(), materialPathStr);
            }
            else
            {
                LogError(R"([{}] Material "{}" of "{}" failed to instantiate: invalid path "{}" for param "{}".)",
                         ExtensionName, aChunk->chunkName.ToString(), meshPathStr, paramPathStr, param.name.ToString());
            }

            aChunk->failed = true;
            continue;
        }

        aJobQueue.Wait(reference.token->job);
    }
}

void App::MeshExtension::ExpandMaterialInheritance(const Core::SharedPtr<DynamicContext>& aContext,
                                                   const Core::SharedPtr<ChunkData>& aChunk,
                                                   const Red::Handle<Red::CMaterialInstance>& aMaterialInstance,
                                                   Red::JobQueue& aJobQueue)
{
    auto& baseReference = aMaterialInstance->baseMaterial;

    if (!baseReference.path)
    {
        const auto meshPathStr = s_resourcePathRegistry->ResolvePathOrHash(aContext->targetMesh->path);

        if (aMaterialInstance->path)
        {
            const auto materialPathStr = s_resourcePathRegistry->ResolvePathOrHash(aMaterialInstance->path);

            LogError(R"([{}] Material "{}" of "{}" failed to instantiate: base material is not set in "{}".)",
                     ExtensionName, aChunk->chunkName.ToString(), meshPathStr, materialPathStr);
        }
        else
        {
            LogError(R"([{}] Material "{}" of "{}" failed to instantiate: base material is not set.)",
                     ExtensionName, aChunk->chunkName.ToString(), meshPathStr);
        }

        aChunk->failed = true;
        return;
    }

    if (baseReference.token)
    {
        if (baseReference.token->IsFailed())
        {
            const auto meshPathStr = s_resourcePathRegistry->ResolvePathOrHash(aContext->targetMesh->path);

            if (aMaterialInstance->path)
            {
                const auto materialPathStr = s_resourcePathRegistry->ResolvePathOrHash(aMaterialInstance->path);

                LogError(R"([{}] Material "{}" of "{}" failed to instantiate: invalid reference for base material in "{}".)",
                         ExtensionName, aChunk->chunkName.ToString(), meshPathStr, materialPathStr);
            }
            else
            {
                LogError(R"([{}] Material "{}" of "{}" failed to instantiate: invalid reference for base material.)",
                         ExtensionName, aChunk->chunkName.ToString(), meshPathStr);
            }

            aChunk->failed = true;
        }
        return;
    }

    auto basePath = ExpandResourcePath(baseReference.path, aChunk->materialName, aContext->targetState);

    if (!basePath)
    {
        const auto meshPathStr = s_resourcePathRegistry->ResolvePathOrHash(aContext->targetMesh->path);
        const auto basePathStr = s_resourcePathRegistry->ResolvePathOrHash(baseReference.path);

        if (aMaterialInstance->path)
        {
            const auto materialPathStr = s_resourcePathRegistry->ResolvePathOrHash(aMaterialInstance->path);

            LogError(R"([{}] Material "{}" of "{}" failed to instantiate: unresolvable path "{}" for base material in "{}".)",
                     ExtensionName, aChunk->chunkName.ToString(), meshPathStr, basePathStr, materialPathStr);
        }
        else
        {
            LogError(R"([{}] Material "{}" of "{}" failed to instantiate: unresolvable path "{}" for base material.)",
                     ExtensionName, aChunk->chunkName.ToString(), meshPathStr, basePathStr);
        }

        aChunk->failed = true;
        return;
    }

    baseReference.path = basePath;
    baseReference.LoadAsync();

    if (!baseReference.token || baseReference.token->IsFailed())
    {
        const auto meshPathStr = s_resourcePathRegistry->ResolvePathOrHash(aContext->targetMesh->path);
        const auto basePathStr = s_resourcePathRegistry->ResolvePathOrHash(basePath);

        if (aMaterialInstance->path)
        {
            const auto materialPathStr = s_resourcePathRegistry->ResolvePathOrHash(aMaterialInstance->path);

            LogError(R"([{}] Material "{}" of "{}" failed to instantiate: invalid path "{}" for base material in "{}".)",
                     ExtensionName, aChunk->chunkName.ToString(), meshPathStr, basePathStr, materialPathStr);
        }
        else
        {
            LogError(R"([{}] Material "{}" of "{}" failed to instantiate: invalid path "{}" for base material.)",
                     ExtensionName, aChunk->chunkName.ToString(), meshPathStr, basePathStr);
        }

        aChunk->failed = true;
        return;
    }

    aJobQueue.Wait(baseReference.token->job);
    aJobQueue.Dispatch([aContext, aChunk, aMaterialInstance](const Red::JobGroup& aJobGroup) {
        auto& baseReference = aMaterialInstance->baseMaterial;

        if (baseReference.token->IsFailed())
        {
            const auto meshPathStr = s_resourcePathRegistry->ResolvePathOrHash(aContext->targetMesh->path);
            const auto basePathStr = s_resourcePathRegistry->ResolvePathOrHash(baseReference.path);

            if (aMaterialInstance->path)
            {
                const auto materialPathStr = s_resourcePathRegistry->ResolvePathOrHash(aMaterialInstance->path);

                LogError(R"([{}] Material "{}" of "{}" failed to instantiate: base material "{}" cannot be loaded for "{}".)",
                         ExtensionName, aChunk->chunkName.ToString(), meshPathStr, basePathStr, materialPathStr);
            }
            else
            {
                LogError(R"([{}] Material "{}" of "{}" failed to instantiate: base material "{}" cannot be loaded.)",
                         ExtensionName, aChunk->chunkName.ToString(), meshPathStr, basePathStr);
            }

            aChunk->failed = true;
            return;
        }

        auto baseInstance = Red::Cast<Red::CMaterialInstance>(baseReference.token->resource);

        if (!baseInstance)
        {
            const auto meshPathStr = s_resourcePathRegistry->ResolvePathOrHash(aContext->targetMesh->path);

            if (aMaterialInstance->path)
            {
                const auto materialPathStr = s_resourcePathRegistry->ResolvePathOrHash(aMaterialInstance->path);

                LogError(R"([{}] Material "{}" of "{}" failed to instantiate: invalid reference for base material in "{}".)",
                         ExtensionName, aChunk->chunkName.ToString(), meshPathStr, materialPathStr);
            }
            else
            {
                LogError(R"([{}] Material "{}" of "{}" failed to instantiate: invalid reference for base material.)",
                         ExtensionName, aChunk->chunkName.ToString(), meshPathStr);
            }

            aChunk->failed = true;
            return;
        }

        Red::JobQueue jobQueue(aJobGroup);

        auto cloneToken = Red::MakeShared<Red::ResourceToken<Red::IMaterial>>();
        cloneToken->self = cloneToken;
        cloneToken->path = baseReference.path;
        cloneToken->resource = CloneMaterialInstance(aContext, aChunk, baseInstance, jobQueue);
        cloneToken->finished = 1;

        baseReference.token = std::move(cloneToken);
    });
}

Red::ResourcePath App::MeshExtension::ExpandResourcePath(Red::ResourcePath aPath, Red::CName aMaterialName,
                                                         const Core::SharedPtr<MeshState>& aState)
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
        return {};
    }

    return result.value.data();
}

void App::MeshExtension::FixFinalMaterialsIndexes(const Core::SharedPtr<DynamicContext>& aContext)
{
    for (int32_t chunkIndex = 0; chunkIndex < aContext->materialNames.size; ++chunkIndex)
    {
        auto chunkName = aContext->materialNames[chunkIndex];
        auto materialIndex = aContext->targetState->GetMaterialEntryIndex(chunkName);
        if (materialIndex >= 0)
        {
            (*aContext->finalMaterials)[chunkIndex] = aContext->targetMesh->materialEntries[materialIndex].materialWeak;
        }
        else
        {
            (*aContext->finalMaterials)[chunkIndex] = {};
        }
    }
}

bool App::MeshExtension::ContainsUnresolvedMaterials(const Red::DynArray<Red::Handle<Red::IMaterial>>& aMaterials)
{
    return std::ranges::any_of(aMaterials, [](const auto& aMaterial) { return !aMaterial; });
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

void App::MeshExtension::PrefetchMeshState(Red::CMesh* aMesh, const Core::Map<Red::CName, std::string>& aContext)
{
    if (!aContext.empty())
    {
        auto meshState = AcquireMeshState(aMesh);
        meshState->PrefillContext(aContext);
    }
    else if (IsContextualMesh(aMesh))
    {
        AcquireMeshState(aMesh);
    }
}

Red::CName App::MeshExtension::RegisterMeshSource(Red::CMesh* aMesh, Red::CMesh* aSourceMesh)
{
    if (!aSourceMesh || aSourceMesh->materialEntries.size == 0)
        return {};

    auto meshState = AcquireMeshState(aMesh);
    auto sourceTag = meshState->RegisterSource(aSourceMesh);

    if (aMesh->materialEntries.size > 0)
    {
        std::scoped_lock _(meshState->meshMutex);

        auto materialIndex = static_cast<int32_t>(aMesh->materialEntries.size);
        meshState->materials[sourceTag] = materialIndex;
        aMesh->materialEntries.EmplaceBack();

        auto& materialEntry = aMesh->materialEntries.Back();
        materialEntry.name = sourceTag;
        materialEntry.isLocalInstance = true;
        materialEntry.material = s_dummyMaterial;
        materialEntry.materialWeak = s_dummyMaterial;
    }

    return sourceTag;
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

void App::MeshExtension::MeshState::PrefillContext(const Core::Map<Red::CName, std::string>& aContext)
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

void App::MeshExtension::MeshState::EnsureContextReady()
{
    if (!contextToken)
        return;

    if (!contextToken->IsFinished())
    {
        Red::WaitForResource(contextToken, std::chrono::milliseconds(500));
    }

    if (!contextToken->IsLoaded())
    {
        auto meshPathStr = s_resourcePathRegistry->ResolvePathOrHash(meshPath);
        LogError(R"([{}] Context for mesh \"{}\" cannot be loaded.)", ExtensionName, meshPathStr);

        contextToken.Reset();
        return;
    }

    auto& contextInstance = Red::Cast<Red::CMaterialInstance>(contextToken->resource);

    if (!contextInstance)
    {
        auto meshPathStr = s_resourcePathRegistry->ResolvePathOrHash(meshPath);
        LogError(R"([{}] Context for mesh \"{}\" must be instance of {}, got {}.)", ExtensionName, meshPathStr,
                 Red::CMaterialInstance::NAME, contextToken->resource->GetType()->name.ToString());

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
    EnsureContextReady();

    return contextParams;
}

const App::DynamicAttributeList& App::MeshExtension::MeshState::GetContextAttrs()
{
    EnsureContextReady();

    return contextAttrs;
}

std::string_view App::MeshExtension::MeshState::GetContextAttr(Red::CName aAttr)
{
    EnsureContextReady();

    const auto& it = contextAttrs.find(aAttr);

    if (it == contextAttrs.end())
        return {};

    return it.value().value;
}

Red::CName App::MeshExtension::MeshState::GetExpansionSource()
{
    EnsureContextReady();

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
    Red::CName sourceTag = aSourceMesh->path.hash;
    sources[sourceTag] = Red::AsWeakHandle(aSourceMesh);

    return sourceTag;
}

Red::Handle<Red::CMesh> App::MeshExtension::MeshState::ResolveSource(Red::CName aSourceName)
{
    auto sourceMesh = sources.find(aSourceName);

    if (sourceMesh == sources.end())
        return {};

    return sourceMesh.value().Lock();
}
