#include "Extension.hpp"
#include "App/Extensions/ResourcePatch/Extension.hpp"
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
constexpr auto NoCollisionPreset = Red::CName("No Collision");

Red::Handle<Red::worldStaticMeshNode> s_dummyNode;
Core::Map<Red::ResourcePath, Core::Map<int64_t, Red::CName*>> s_collisionNodeOverrides;
}

std::string_view App::WorldStreamingExtension::GetName()
{
    return ExtensionName;
}

bool App::WorldStreamingExtension::Load()
{
    HookAfter<Raw::StreamingSector::PostLoad>(&OnSectorPostLoad).OrThrow();
    Hook<Raw::AIWorkspotManager::RegisterSpots>(&OnRegisterSpots).OrThrow();

    s_dummyNode = Red::MakeHandle<Red::worldStaticMeshNode>();
    s_dummyNode->isVisibleInGame = false;

    return true;
}

bool App::WorldStreamingExtension::Unload()
{
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

void App::WorldStreamingExtension::PostConfigure()
{
    // ResourcePatchExtension::ClearTarget(MainWorldResource);

    for (auto& unit : m_configs)
    {
        for (auto& blockPathStr : unit.blocks)
        {
            ResourcePatchExtension::RegisterPatch(MainWorldResource, blockPathStr.data());
        }

        unit.blocks.clear();
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

    if (nodeCount != aSectorMod.expectedNodes)
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

        if (!nodeMutation.elementMutations.empty())
        {
            if (nodeMutation.nodeType == CollisionNodeType)
            {
                auto& actors = Raw::CollisionNode::Actors::Ref(nodeDefinition);
                if (actors.GetSize() != nodeMutation.expectedElements)
                {
                    LogError(R"([{}] {}: The target node #{} has {} actor(s), but the mod expects {}.)",
                             ExtensionName, aSectorMod.mod, nodeMutation.nodeIndex,
                             actors.GetSize(), nodeMutation.expectedElements);
                    nodeValidationPassed = false;
                    continue;
                }
            }
            else if (nodeMutation.nodeType == InstancedMeshNodeType)
            {
                auto* meshNode = Red::Cast<Red::worldInstancedMeshNode>(nodeDefinition);
                if (meshNode->worldTransformsBuffer.numElements != nodeMutation.expectedElements)
                {
                    LogError(R"([{}] {}: The target node #{} has {} instance(s), but the mod expects {}.)",
                             ExtensionName, aSectorMod.mod, nodeMutation.nodeIndex,
                             meshNode->worldTransformsBuffer.numElements, nodeMutation.expectedElements);
                    nodeValidationPassed = false;
                    continue;
                }
            }
            else if (nodeMutation.nodeType == InstancedDestructibleNodeType)
            {
                auto* meshNode = Red::Cast<Red::worldInstancedDestructibleMeshNode>(nodeDefinition);
                if (meshNode->cookedInstanceTransforms.numElements != nodeMutation.expectedElements)
                {
                    LogError(R"([{}] {}: The target node #{} has {} instance(s), but the mod expects {}.)",
                             ExtensionName, aSectorMod.mod, nodeMutation.nodeIndex,
                             meshNode->cookedInstanceTransforms.numElements, nodeMutation.expectedElements);
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

        if (!nodeDeletion.elementDeletions.empty())
        {
            if (nodeDeletion.nodeType == CollisionNodeType)
            {
                auto& actors = Raw::CollisionNode::Actors::Ref(nodeDefinition);
                if (actors.GetSize() != nodeDeletion.expectedElements)
                {
                    LogError(R"([{}] {}: The target node #{} has {} actor(s), but the mod expects {}.)",
                             ExtensionName, aSectorMod.mod, nodeDeletion.nodeIndex,
                             actors.GetSize(), nodeDeletion.expectedElements);
                    nodeValidationPassed = false;
                    continue;
                }
                if (!nodeDeletion.elementDeletions.empty())
                {
                    auto& shapes = Raw::CollisionNode::Shapes::Ref(nodeDefinition);
                    for (const auto& elementDeletion : nodeDeletion.elementDeletions)
                    {
                        if (elementDeletion.subElementIndex >= 0)
                        {
                            auto& actor = actors.beginPtr[elementDeletion.elementIndex];
                            if (elementDeletion.subElementIndex >= actor.numShapes)
                            {
                                LogError(R"([{}] {}: The target node #{} actor #{} has {} shape(s), but the mod expects {} or more.)",
                                         ExtensionName, aSectorMod.mod, nodeDeletion.nodeIndex, elementDeletion.elementIndex,
                                         actor.numShapes, elementDeletion.subElementIndex + 1);
                                nodeValidationPassed = false;
                                continue;
                            }
                        }
                    }
                }
            }
            else if (nodeDeletion.nodeType == InstancedMeshNodeType)
            {
                auto* meshNode = Red::Cast<Red::worldInstancedMeshNode>(nodeDefinition);
                if (meshNode->worldTransformsBuffer.numElements != nodeDeletion.expectedElements)
                {
                    LogError(R"([{}] {}: The target node #{} has {} instance(s), but the mod expects {}.)",
                             ExtensionName, aSectorMod.mod, nodeDeletion.nodeIndex,
                             meshNode->worldTransformsBuffer.numElements, nodeDeletion.expectedElements);
                    nodeValidationPassed = false;
                    continue;
                }
            }
            else if (nodeDeletion.nodeType == InstancedDestructibleNodeType)
            {
                auto* meshNode = Red::Cast<Red::worldInstancedDestructibleMeshNode>(nodeDefinition);
                if (meshNode->cookedInstanceTransforms.numElements != nodeDeletion.expectedElements)
                {
                    LogError(R"([{}] {}: The target node #{} has {} instance(s), but the mod expects {}.)",
                             ExtensionName, aSectorMod.mod, nodeDeletion.nodeIndex,
                             meshNode->cookedInstanceTransforms.numElements, nodeDeletion.expectedElements);
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

        if (!nodeMutation.elementMutations.empty())
        {
            if (nodeMutation.nodeType == CollisionNodeType)
            {
                auto& actors = Raw::CollisionNode::Actors::Ref(nodeDefinition);
                for (const auto& elementMutation : nodeMutation.elementMutations)
                {
                    auto* instance = actors.beginPtr + elementMutation.elementIndex;

                    if (elementMutation.modifyPosition)
                    {
                        instance->transform.Position = elementMutation.position;
                    }

                    if (elementMutation.modifyOrientation)
                    {
                        instance->transform.Orientation = elementMutation.orientation;
                    }
                }
                continue;
            }

            if (nodeMutation.nodeType == InstancedMeshNodeType)
            {
                auto* meshNode = Red::Cast<Red::worldInstancedMeshNode>(nodeDefinition);
                auto* instances = std::bit_cast<Red::RenderProxyTransformData*>(&meshNode->worldTransformsBuffer.sharedDataBuffer->buffer.buffer.data);
                auto startIndex = meshNode->worldTransformsBuffer.startIndex;
                for (const auto& elementMutation : nodeMutation.elementMutations)
                {
                    auto* instance = instances->Get(startIndex + elementMutation.elementIndex);

                    if (elementMutation.modifyPosition)
                    {
                        instance->transform.Position = elementMutation.position;
                    }

                    if (elementMutation.modifyOrientation)
                    {
                        instance->transform.Orientation = elementMutation.orientation;
                    }

                    if (elementMutation.modifyScale)
                    {
                        instance->scale = elementMutation.scale;
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
                for (const auto& elementMutation : nodeMutation.elementMutations)
                {
                    auto* instance = instances->Get(startIndex + elementMutation.elementIndex);
                    auto newTransform = nodeSetup->transform;

                    if (elementMutation.modifyPosition)
                    {
                        newTransform.position = elementMutation.position;
                    }

                    if (elementMutation.modifyOrientation)
                    {
                        newTransform.orientation = elementMutation.orientation;
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

        if (!nodeDeletion.elementDeletions.empty())
        {
            if (nodeDeletion.nodeType == CollisionNodeType)
            {
                auto& actors = Raw::CollisionNode::Actors::Ref(nodeDefinition);
                auto& shapes = Raw::CollisionNode::Shapes::Ref(nodeDefinition);
                auto& presets = Raw::CollisionNode::Presets::Ref(nodeDefinition);
                uint8_t noCollisionIndex = 0xFF;

                for (const auto& elementDeletion : nodeDeletion.elementDeletions)
                {
                    auto& actor = actors.beginPtr[elementDeletion.elementIndex];
                    if (elementDeletion.subElementIndex >= 0 && actor.numShapes > 1)
                    {
                        if (elementDeletion.subElementIndex < actor.numShapes)
                        {
                            if (noCollisionIndex == 0xFF)
                            {
                                noCollisionIndex = presets.GetSize();
                                auto overrideSize = noCollisionIndex + 1;

                                auto& override = s_collisionNodeOverrides[aSector->path][nodeDeletion.nodeIndex];
                                if (!override)
                                {
                                    auto data = Red::Memory::DefaultAllocator::Get()->Alloc(sizeof(Red::CName) *
                                                                                            overrideSize);
                                    override = static_cast<Red::CName*>(data.memory);
                                    override[noCollisionIndex] = NoCollisionPreset;

                                    std::copy(presets.begin(), presets.end(), override);
                                }

                                presets.beginPtr = override;
                                presets.endPtr = override + overrideSize;
                            }

                            auto& shape = shapes.beginPtr[actor.shapeStartIndex + elementDeletion.subElementIndex];
                            shape.presetIndex = noCollisionIndex;
                        }
                    }
                    else
                    {
                        actor.transform.Position.z.Bits = DelZ;
                    }
                }
                continue;
            }

            if (nodeDeletion.nodeType == InstancedMeshNodeType)
            {
                auto* meshNode = Red::Cast<Red::worldInstancedMeshNode>(nodeDefinition);
                auto* instances = std::bit_cast<Red::RenderProxyTransformData*>(&meshNode->worldTransformsBuffer.sharedDataBuffer->buffer.buffer.data);
                auto startIndex = meshNode->worldTransformsBuffer.startIndex;
                for (const auto& elementDeletion : nodeDeletion.elementDeletions)
                {
                    auto* instance = instances->Get(startIndex + elementDeletion.elementIndex);
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
                for (const auto& elementDeletion : nodeDeletion.elementDeletions)
                {
                    auto* instance = instances->Get(startIndex + elementDeletion.elementIndex);
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
        auto newSize = allSpots->size + aNewSpots.Size();
        if (allSpots->capacity < newSize)
        {
            Raw::AISpotPersistentDataArray::Reserve(allSpots, newSize);
        }

        allSpots->flags |= static_cast<int32_t>(Red::SortedArray<Red::AISpotPersistentData>::Flags::NotSorted);
    }

    Raw::AIWorkspotManager::RegisterSpots(aManager, aNewSpots);
}
