#include "Extension.hpp"
#include "Red/CommunitySystem.hpp"

namespace
{
constexpr auto ExtensionName = "WorldStreaming";

constexpr auto MainWorldResource = Red::ResourcePath(R"(base\worlds\03_night_city\_compiled\default\03_night_city.streamingworld)");
constexpr auto CollisionNodeType = Red::GetTypeName<Red::worldCollisionNode>();
constexpr auto AdvertisementNodeType = Red::GetTypeName<Red::worldAdvertisementNode>();
constexpr auto StaticMeshNodeType = Red::GetTypeName<Red::worldStaticMeshNode>();
constexpr auto InstancedMeshNodeType = Red::GetTypeName<Red::worldInstancedMeshNode>();
constexpr auto InstancedDestructibleNodeType = Red::GetTypeName<Red::worldInstancedDestructibleMeshNode>();

constexpr auto DelZ = static_cast<int32_t>(-2000 * (2 << 16));

Red::Handle<Red::worldStaticMeshNode> s_dummyNode;
}

std::string_view App::WorldStreamingExtension::GetName()
{
    return ExtensionName;
}

bool App::WorldStreamingExtension::Load()
{
    HookAfter<Raw::StreamingWorld::Serialize>(&WorldStreamingExtension::OnWorldSerialize).OrThrow();
    HookAfter<Raw::StreamingSector::PostLoad>(&OnSectorPostLoad).OrThrow();
    Hook<Raw::AIWorkspotManager::RegisterSpots>(&OnRegisterSpots).OrThrow();

    s_dummyNode = Red::MakeHandle<Red::worldStaticMeshNode>();
    s_dummyNode->isVisibleInGame = false;

    return true;
}

bool App::WorldStreamingExtension::Unload()
{
    Unhook<Raw::StreamingWorld::Serialize>();
    Unhook<Raw::StreamingSector::PostLoad>();
    Unhook<Raw::AIWorkspotManager::RegisterSpots>();

    return true;
}

void App::WorldStreamingExtension::Configure()
{
    std::unique_lock _(s_sectorsLock);

    s_sectors.clear();

    auto depot = Red::ResourceDepot::Get();
    auto rtti = Red::CRTTISystem::Get();

    Core::Set<Red::ResourcePath> invalidPaths;
    Core::Set<Red::CName> invalidNodeTypes;

    for (auto& unit : m_configs)
    {
        for (auto& sectorMod : unit.sectors)
        {
            auto sectorPath = Red::ResourcePath(sectorMod.path.data());

            if (!depot->ResourceExists(sectorPath))
            {
                if (!invalidPaths.contains(sectorPath))
                {
                    LogError("[{}] {}: Sector \"{}\" doesn't exist.",
                             ExtensionName, sectorMod.mod, sectorMod.path);
                    invalidPaths.insert(sectorPath);
                }
                continue;
            }

            bool allNodeTypesValid = true;

            for (const auto& nodeDeletion : sectorMod.nodeDeletions)
            {
                if (!rtti->GetClass(nodeDeletion.nodeType))
                {
                    allNodeTypesValid = false;

                    if (!invalidNodeTypes.contains(nodeDeletion.nodeType))
                    {
                        LogError("[{}] {}: Node type \"{}\" doesn't exist.",
                                 ExtensionName, sectorMod.mod, nodeDeletion.nodeType.ToString());
                        invalidNodeTypes.insert(nodeDeletion.nodeType);
                    }
                }
            }

            if (!allNodeTypesValid)
                continue;

            s_sectors[sectorPath].emplace_back(std::move(sectorMod));
        }

        unit.sectors.clear();
    }
}

void App::WorldStreamingExtension::OnWorldSerialize(Red::world::StreamingWorld* aWorld, Red::BaseStream* aStream)
{
    if (aWorld->path != MainWorldResource)
        return;

    LogInfo("[{}] World streaming is initializing...", ExtensionName);

    if (!m_configs.empty())
    {
        Core::Vector<StreamingBlockRef> blockRefs;
        Core::Map<Red::ResourcePath, std::string_view> blockPaths;

        for (const auto& unit : m_configs)
        {
            if (!unit.blocks.empty())
            {
                // LogInfo("[{}] Processing \"{}\"...", ExtensionName, unit.name);

                for (const auto& blockPathStr : unit.blocks)
                {
                    auto blockPath = Red::ResourcePath(blockPathStr.c_str());
                    if (!blockPaths.contains(blockPath))
                    {
                        blockRefs.emplace_back(blockPath);
                        blockRefs.back().LoadAsync();

                        blockPaths.insert({blockPath, blockPathStr});
                    }
                }
            }
        }

        if (!blockRefs.empty())
        {
            Red::WaitForResources(blockRefs, std::chrono::milliseconds(5000));

            bool allSucceeded = true;

            for (auto& blockRef : blockRefs)
            {
                if (!blockRef.token->IsFailed())
                {
                    LogInfo("[{}] Merging streaming block \"{}\"...", ExtensionName, blockPaths[blockRef.path]);

                    aWorld->blockRefs.EmplaceBack(std::move(blockRef));
                }
                else
                {
                    LogError("[{}] Resource \"{}\" failed to load.", ExtensionName, blockPaths[blockRef.path]);

                    allSucceeded = false;
                }
            }

            if (allSucceeded)
                LogInfo("[{}] All streaming blocks merged.", ExtensionName);
            else
                LogWarning("[{}] Streaming blocks merged with issues.", ExtensionName);
        }
        else
        {
            LogInfo("[{}] No blocks to merge.", ExtensionName);
        }
    }
    else
    {
        LogInfo("[{}] No blocks to merge.", ExtensionName);
    }
}

void App::WorldStreamingExtension::OnSectorPostLoad(Red::world::StreamingSector* aSector, uint64_t)
{
    std::shared_lock _(s_sectorsLock);

    const auto& sectorMods = s_sectors.find(aSector->path);

    if (sectorMods == s_sectors.end())
        return;

    const auto& sectorPath = sectorMods.value().begin()->path;
    auto patchedAny = false;
    auto successAll = true;

    LogInfo("[{}] Patching sector \"{}\"...", ExtensionName, sectorPath);

    for (const auto& sectorMod : sectorMods.value())
    {
        LogInfo("[{}] Applying changes from \"{}\"...", ExtensionName, sectorMod.mod);

        if (PatchSector(aSector, sectorMod))
        {
            patchedAny = true;
        }
        else
        {
            successAll = false;
        }
    }

    if (successAll)
        LogInfo("[{}] All patches have been applied to \"{}\".", ExtensionName, sectorPath);
    else if (patchedAny)
        LogWarning("[{}] Some patches have not been applied to \"{}\".", ExtensionName, sectorPath);
    else
        LogWarning("[{}] No patches have been applied to \"{}\".", ExtensionName, sectorPath);
}

bool App::WorldStreamingExtension::PatchSector(Red::world::StreamingSector* aSector,
                                               const App::WorldSectorMod& aSectorMod)
{
    auto& buffer = Raw::StreamingSector::NodeBuffer::Ref(aSector);
    auto nodeCount = buffer.nodeSetups.GetInstanceCount();

    if (nodeCount!= aSectorMod.expectedNodes)
    {
        LogError(R"([{}] {}: The target sector has {} node(s), but the mod expects {}.)",
                 ExtensionName, aSectorMod.mod, nodeCount, aSectorMod.expectedNodes);
        return false;
    }

    bool nodeValidationPassed = true;

    for (const auto& nodeMutation : aSectorMod.nodeMutations)
    {
        auto* nodeSetup = buffer.nodeSetups.GetInstance(nodeMutation.nodeIndex);
        auto* nodeDefinition = buffer.nodes[nodeSetup->nodeIndex].instance;

        if (nodeMutation.nodeType != nodeDefinition->GetNativeType()->name)
        {
            LogError(R"([{}] {}: The target node #{} has type {}, but the mod expects {}.)",
                     ExtensionName, aSectorMod.mod,
                     nodeMutation.nodeIndex,
                     nodeDefinition->GetNativeType()->name.ToString(),
                     nodeMutation.nodeType.ToString());
            nodeValidationPassed = false;
            continue;
        }

        if (!nodeMutation.subNodeMutations.empty())
        {
            if (nodeMutation.nodeType == CollisionNodeType)
            {
                auto& actors = Raw::CollisionNode::Actors::Ref(nodeDefinition);
                if (actors.GetSize() != nodeMutation.expectedSubNodes)
                {
                    LogError(R"([{}] {}: The target node #{} has {} actor(s), but the mod expects {}.)",
                             ExtensionName, aSectorMod.mod, nodeMutation.nodeIndex,
                             actors.GetSize(), nodeMutation.expectedSubNodes);
                    nodeValidationPassed = false;
                    continue;
                }
            }
            else if (nodeMutation.nodeType == InstancedMeshNodeType)
            {
                auto* meshNode = Red::Cast<Red::worldInstancedMeshNode>(nodeDefinition);
                if (meshNode->worldTransformsBuffer.numElements != nodeMutation.expectedSubNodes)
                {
                    LogError(R"([{}] {}: The target node #{} has {} instance(s), but the mod expects {}.)",
                             ExtensionName, aSectorMod.mod, nodeMutation.nodeIndex,
                             meshNode->worldTransformsBuffer.numElements, nodeMutation.expectedSubNodes);
                    nodeValidationPassed = false;
                    continue;
                }
            }
            else if (nodeMutation.nodeType == InstancedDestructibleNodeType)
            {
                auto* meshNode = Red::Cast<Red::worldInstancedDestructibleMeshNode>(nodeDefinition);
                if (meshNode->cookedInstanceTransforms.numElements != nodeMutation.expectedSubNodes)
                {
                    LogError(R"([{}] {}: The target node #{} has {} instance(s), but the mod expects {}.)",
                             ExtensionName, aSectorMod.mod, nodeMutation.nodeIndex,
                             meshNode->cookedInstanceTransforms.numElements, nodeMutation.expectedSubNodes);
                    nodeValidationPassed = false;
                    continue;
                }
            }
        }
    }

    for (const auto& nodeDeletion : aSectorMod.nodeDeletions)
    {
        auto* nodeSetup = buffer.nodeSetups.GetInstance(nodeDeletion.nodeIndex);
        auto* nodeDefinition = buffer.nodes[nodeSetup->nodeIndex].instance;

        if (nodeDeletion.nodeType != nodeDefinition->GetNativeType()->name)
        {
            LogError(R"([{}] {}: The target node #{} has type {}, but the mod expects {}.)",
                     ExtensionName, aSectorMod.mod, nodeDeletion.nodeIndex,
                     nodeDefinition->GetNativeType()->name.ToString(),
                     nodeDeletion.nodeType.ToString());
            nodeValidationPassed = false;
            continue;
        }

        if (!nodeDeletion.subNodeDeletions.empty())
        {
            if (nodeDeletion.nodeType == CollisionNodeType)
            {
                auto& actors = Raw::CollisionNode::Actors::Ref(nodeDefinition);
                if (actors.GetSize() != nodeDeletion.expectedSubNodes)
                {
                    LogError(R"([{}] {}: The target node #{} has {} actor(s), but the mod expects {}.)",
                             ExtensionName, aSectorMod.mod, nodeDeletion.nodeIndex,
                             actors.GetSize(), nodeDeletion.expectedSubNodes);
                    nodeValidationPassed = false;
                    continue;
                }
            }
            else if (nodeDeletion.nodeType == InstancedMeshNodeType)
            {
                auto* meshNode = Red::Cast<Red::worldInstancedMeshNode>(nodeDefinition);
                if (meshNode->worldTransformsBuffer.numElements != nodeDeletion.expectedSubNodes)
                {
                    LogError(R"([{}] {}: The target node #{} has {} instance(s), but the mod expects {}.)",
                             ExtensionName, aSectorMod.mod, nodeDeletion.nodeIndex,
                             meshNode->worldTransformsBuffer.numElements, nodeDeletion.expectedSubNodes);
                    nodeValidationPassed = false;
                    continue;
                }
            }
            else if (nodeDeletion.nodeType == InstancedDestructibleNodeType)
            {
                auto* meshNode = Red::Cast<Red::worldInstancedDestructibleMeshNode>(nodeDefinition);
                if (meshNode->cookedInstanceTransforms.numElements != nodeDeletion.expectedSubNodes)
                {
                    LogError(R"([{}] {}: The target node #{} has {} instance(s), but the mod expects {}.)",
                             ExtensionName, aSectorMod.mod, nodeDeletion.nodeIndex,
                             meshNode->cookedInstanceTransforms.numElements, nodeDeletion.expectedSubNodes);
                    nodeValidationPassed = false;
                    continue;
                }
            }
        }
    }

    if (!nodeValidationPassed)
    {
        return false;
    }

    for (const auto& nodeMutation : aSectorMod.nodeMutations)
    {
        auto* nodeSetup = buffer.nodeSetups.GetInstance(nodeMutation.nodeIndex);
        auto* nodeDefinition = buffer.nodes[nodeSetup->nodeIndex].instance;

        if (nodeMutation.modifyPosition)
        {
            nodeSetup->transform.position = nodeMutation.position;
        }

        if (nodeMutation.modifyOrientation)
        {
            nodeSetup->transform.orientation = nodeMutation.orientation;
        }

        if (nodeMutation.modifyScale)
        {
            nodeSetup->scale = nodeMutation.scale;
        }

        if (nodeMutation.modifyResource)
        {
            if (auto* meshDefinition = Red::Cast<Red::worldMeshNode>(nodeDefinition))
            {
                meshDefinition->mesh.path = nodeMutation.resourcePath;
            }
            else if (auto* instancedMeshDefinition = Red::Cast<Red::worldInstancedMeshNode>(nodeDefinition))
            {
                instancedMeshDefinition->mesh.path = nodeMutation.resourcePath;
            }
            else if (auto* entityDefinition = Red::Cast<Red::worldEntityNode>(nodeDefinition))
            {
                entityDefinition->entityTemplate.path = nodeMutation.resourcePath;
            }
            else if (auto* effectDefinition = Red::Cast<Red::worldEffectNode>(nodeDefinition))
            {
                effectDefinition->effect.path = nodeMutation.resourcePath;
            }
            else if (auto* decalDefinition = Red::Cast<Red::worldStaticDecalNode>(nodeDefinition))
            {
                decalDefinition->material.path = nodeMutation.resourcePath;
            }
            else if (auto* physicalDefinition = Red::Cast<Red::worldPhysicalDestructionNode>(nodeDefinition))
            {
                physicalDefinition->mesh.path = nodeMutation.resourcePath;
            }
            else if (auto* staticOccluderDefinition = Red::Cast<Red::worldStaticOccluderMeshNode>(nodeDefinition))
            {
                staticOccluderDefinition->mesh.path = nodeMutation.resourcePath;
            }
            else if (auto* instancedOccluderDefinition = Red::Cast<Red::worldInstancedOccluderNode>(nodeDefinition))
            {
                instancedOccluderDefinition->mesh.path = nodeMutation.resourcePath;
            }
            else if (auto* foliageDefinition = Red::Cast<Red::worldFoliageNode>(nodeDefinition))
            {
                foliageDefinition->mesh.path = nodeMutation.resourcePath;
            }
            else if (auto* terrainDefinition = Red::Cast<Red::worldTerrainMeshNode>(nodeDefinition))
            {
                terrainDefinition->meshRef.path = nodeMutation.resourcePath;
            }
        }

        if (nodeMutation.modifyAppearance)
        {
            if (auto* meshDefinition = Red::Cast<Red::worldMeshNode>(nodeDefinition))
            {
                meshDefinition->meshAppearance = nodeMutation.appearanceName;
            }
            else if (auto* instancedMeshDefinition = Red::Cast<Red::worldInstancedMeshNode>(nodeDefinition))
            {
                instancedMeshDefinition->meshAppearance = nodeMutation.appearanceName;
            }
            else if (auto* entityDefinition = Red::Cast<Red::worldEntityNode>(nodeDefinition))
            {
                entityDefinition->appearanceName = nodeMutation.appearanceName;
            }
            else if (auto* spawnerDefinition = Red::Cast<Red::worldPopulationSpawnerNode>(nodeDefinition))
            {
                spawnerDefinition->appearanceName = nodeMutation.appearanceName;
            }
            else if (auto* physicalDefinition = Red::Cast<Red::worldPhysicalDestructionNode>(nodeDefinition))
            {
                physicalDefinition->meshAppearance = nodeMutation.appearanceName;
            }
            else if (auto* foliageDefinition = Red::Cast<Red::worldFoliageNode>(nodeDefinition))
            {
                foliageDefinition->meshAppearance = nodeMutation.appearanceName;
            }
        }

        if (nodeMutation.modifyRecordID)
        {
            if (auto* spawnerDefinition = Red::Cast<Red::worldPopulationSpawnerNode>(nodeDefinition))
            {
                spawnerDefinition->objectRecordId = nodeMutation.recordID;
            }
        }

        if (nodeMutation.modifyProxyNodes && nodeMutation.nbNodesUnderProxyDiff > 0)
        {
            if (auto* proxyDefinition = Red::Cast<Red::worldPrefabProxyMeshNode>(nodeDefinition))
            {
                proxyDefinition->nbNodesUnderProxy += nodeMutation.nbNodesUnderProxyDiff;
            }
        }

        if (!nodeMutation.subNodeMutations.empty())
        {
            if (nodeMutation.nodeType == CollisionNodeType)
            {
                auto& actors = Raw::CollisionNode::Actors::Ref(nodeDefinition);
                for (const auto& subNodeMutation : nodeMutation.subNodeMutations)
                {
                    auto* instance = actors.beginPtr + subNodeMutation.subNodeIndex;

                    if (subNodeMutation.modifyPosition)
                    {
                        instance->transform.Position = subNodeMutation.position;
                    }

                    if (subNodeMutation.modifyOrientation)
                    {
                        instance->transform.Orientation = subNodeMutation.orientation;
                    }
                }
                continue;
            }

            if (nodeMutation.nodeType == InstancedMeshNodeType)
            {
                auto* meshNode = Red::Cast<Red::worldInstancedMeshNode>(nodeDefinition);
                auto* instances = std::bit_cast<Red::RenderProxyTransformData*>(&meshNode->worldTransformsBuffer.sharedDataBuffer->buffer.buffer.data);
                auto startIndex = meshNode->worldTransformsBuffer.startIndex;
                for (const auto& subNodeMutation : nodeMutation.subNodeMutations)
                {
                    auto* instance = instances->Get(startIndex + subNodeMutation.subNodeIndex);

                    if (subNodeMutation.modifyPosition)
                    {
                        instance->transform.Position = subNodeMutation.position;
                    }

                    if (subNodeMutation.modifyOrientation)
                    {
                        instance->transform.Orientation = subNodeMutation.orientation;
                    }

                    if (subNodeMutation.modifyScale)
                    {
                        instance->scale = subNodeMutation.scale;
                    }
                }
                continue;
            }

            if (nodeMutation.nodeType == InstancedDestructibleNodeType)
            {
                auto* meshNode = Red::Cast<Red::worldInstancedDestructibleMeshNode>(nodeDefinition);
                auto* instances = std::bit_cast<Red::TransformBuffer*>(&meshNode->cookedInstanceTransforms.sharedDataBuffer->buffer.buffer.data);
                auto startIndex = meshNode->cookedInstanceTransforms.startIndex;
                auto inverseTransform = nodeSetup->transform.Inverse();
                for (const auto& subNodeMutation : nodeMutation.subNodeMutations)
                {
                    auto* instance = instances->Get(startIndex + subNodeMutation.subNodeIndex);
                    auto newTransform = nodeSetup->transform;

                    if (subNodeMutation.modifyPosition)
                    {
                        newTransform.position = subNodeMutation.position;
                    }

                    if (subNodeMutation.modifyOrientation)
                    {
                        newTransform.orientation = subNodeMutation.orientation;
                    }

                    *instance = newTransform * inverseTransform;
                }
                continue;
            }
        }
    }

    for (const auto& nodeDeletion : aSectorMod.nodeDeletions)
    {
        auto* nodeSetup = buffer.nodeSetups.GetInstance(nodeDeletion.nodeIndex);
        auto* nodeDefinition = buffer.nodes[nodeSetup->nodeIndex].instance;

        if (!nodeDeletion.subNodeDeletions.empty())
        {
            if (nodeDeletion.nodeType == CollisionNodeType)
            {
                auto& actors = Raw::CollisionNode::Actors::Ref(nodeDefinition);
                for (const auto& subNodeIndex : nodeDeletion.subNodeDeletions)
                {
                    actors.beginPtr[subNodeIndex].transform.Position.z.Bits = DelZ;
                }
                continue;
            }

            if (nodeDeletion.nodeType == InstancedMeshNodeType)
            {
                auto* meshNode = Red::Cast<Red::worldInstancedMeshNode>(nodeDefinition);
                auto* instances = std::bit_cast<Red::RenderProxyTransformData*>(&meshNode->worldTransformsBuffer.sharedDataBuffer->buffer.buffer.data);
                auto startIndex = meshNode->worldTransformsBuffer.startIndex;
                for (const auto& subNodeIndex : nodeDeletion.subNodeDeletions)
                {
                    auto* instance = instances->Get(startIndex + subNodeIndex);
                    instance->scale.X = 0;
                    instance->scale.Y = 0;
                    instance->scale.Z = 0;
                }
                continue;
            }

            if (nodeDeletion.nodeType == InstancedDestructibleNodeType)
            {
                auto* meshNode = Red::Cast<Red::worldInstancedDestructibleMeshNode>(nodeDefinition);
                auto* instances = std::bit_cast<Red::TransformBuffer*>(&meshNode->cookedInstanceTransforms.sharedDataBuffer->buffer.buffer.data);
                auto startIndex = meshNode->cookedInstanceTransforms.startIndex;
                for (const auto& subNodeIndex : nodeDeletion.subNodeDeletions)
                {
                    auto* instance = instances->Get(startIndex + subNodeIndex);
                    instance->position.Z = -2000;
                }
                continue;
            }
        }

        if (nodeDeletion.nodeType == StaticMeshNodeType)
        {
            nodeSetup->scale.X = 0;
            nodeSetup->scale.Y = 0;
            nodeSetup->scale.Z = 0;
            continue;
        }

        if (nodeDeletion.nodeType == InstancedMeshNodeType)
        {
            auto* meshNode = Red::Cast<Red::worldInstancedMeshNode>(nodeDefinition);
            auto* instances = std::bit_cast<Red::RenderProxyTransformData*>(&meshNode->worldTransformsBuffer.sharedDataBuffer->buffer.buffer.data);
            auto startIndex = meshNode->worldTransformsBuffer.startIndex;
            auto numElements = meshNode->worldTransformsBuffer.numElements;
            for (auto i = 0; i < numElements; ++i)
            {
                auto* instance = instances->Get(startIndex + i);
                instance->scale.X = 0;
                instance->scale.Y = 0;
                instance->scale.Z = 0;
            }
            continue;
        }

        if (nodeDeletion.nodeType == AdvertisementNodeType)
        {
            nodeSetup->scale.X = 0;
            nodeSetup->scale.Y = 0;
            nodeSetup->scale.Z = 0;
            nodeSetup->node = s_dummyNode.instance;
            continue;
        }

        nodeSetup->transform.position.Z = -2000;
        nodeSetup->scale.X = 0;
        nodeSetup->scale.Y = 0;
        nodeSetup->scale.Z = 0;

        nodeSetup->globalNodeID = 0;
        // nodeSetup->proxyNodeID = 0;

        nodeSetup->node = s_dummyNode.instance;
    }

    return true;
}

void App::WorldStreamingExtension::OnRegisterSpots(Red::AIWorkspotManager* aManager,
                                                   const Red::DynArray<Red::AISpotPersistentData>& aNewSpots)
{
    auto allSpots = Raw::AIWorkspotManager::Spots::Ptr(aManager);
    auto merging = (allSpots->size != 0);

    if (merging)
    {
        auto newSize = allSpots->size + aNewSpots.size;
        if (allSpots->capacity < newSize)
        {
            Raw::AISpotPersistentDataArray::Reserve(allSpots, newSize);
        }

        allSpots->flags |= static_cast<int32_t>(Red::SortedArray<Red::AISpotPersistentData>::Flags::NotSorted);
    }

    Raw::AIWorkspotManager::RegisterSpots(aManager, aNewSpots);
}
