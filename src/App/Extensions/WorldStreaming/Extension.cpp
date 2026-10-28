#include "Extension.hpp"
#include "App/Extensions/ResourcePatch/Extension.hpp"
#include "Red/CommunitySystem.hpp"

namespace
{
constexpr auto ExtensionName = "WorldStreaming";

constexpr auto MainWorldResource = Red::ResourcePath(R"(base\worlds\03_night_city\_compiled\default\03_night_city.streamingworld)");
constexpr auto CollisionNodeType = Red::GetTypeName<Red::worldCollisionNode>();
constexpr auto CookedCollisionNodeType = Red::GetTypeName<Red::worldCookedCollisionNode>();
constexpr auto AdvertisementNodeType = Red::GetTypeName<Red::worldAdvertisementNode>();
constexpr auto StaticMeshNodeType = Red::GetTypeName<Red::worldStaticMeshNode>();
constexpr auto CookedStaticMeshNodeType = Red::GetTypeName<Red::worldCookedStaticMeshNode>();
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

bool App::WorldStreamingExtension::PatchSector(Red::worldStreamingSector* aSector,
                                               const App::WorldSectorMod& aSectorMod)
{
    auto nodeCount = aSector->compiled.buffer.nodeSetups.Size();

    if (nodeCount != aSectorMod.expectedNodes)
    {
        LogError(R"([{}] {}: The target sector has {} node(s), but the mod expects {}.)",
                 ExtensionName, aSectorMod.mod, nodeCount, aSectorMod.expectedNodes);
        return false;
    }

    bool nodeValidationPassed = true;

    for (const auto& nodeMutation : aSectorMod.nodeMutations)
    {
        auto& nodeSetup = aSector->compiled.buffer.nodeSetups.At(nodeMutation.nodeIndex);
        auto* nodeDefinition = aSector->compiled.nodes[nodeSetup.nodeIndex].instance;

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
                auto collisionNode = Red::Cast<Red::worldCollisionNode>(nodeDefinition);
                if (collisionNode->numActors != nodeMutation.expectedElements)
                {
                    LogError(R"([{}] {}: The target node #{} has {} actor(s), but the mod expects {}.)",
                             ExtensionName, aSectorMod.mod, nodeMutation.nodeIndex,
                             collisionNode->numActors, nodeMutation.expectedElements);
                    nodeValidationPassed = false;
                    continue;
                }
            }
            else if (nodeMutation.nodeType == CookedCollisionNodeType)
            {
                auto collisionNode = Red::Cast<Red::worldCookedCollisionNode>(nodeDefinition);
                auto innerNodes = collisionNode->GetInnerNodes();
                if (nodeSetup.instanceIndex < innerNodes.Size())
                {
                    auto& innerNode = innerNodes[nodeSetup.instanceIndex];
                    if (innerNode.numActors != nodeMutation.expectedElements)
                    {
                        LogError(R"([{}] {}: The target node #{} has {} actor(s), but the mod expects {}.)",
                                 ExtensionName, aSectorMod.mod, nodeMutation.nodeIndex,
                                 innerNode.numActors, nodeMutation.expectedElements);
                        nodeValidationPassed = false;
                        continue;
                    }
                }
            }
            else if (nodeMutation.nodeType == InstancedMeshNodeType)
            {
                auto* meshNode = Red::Cast<Red::worldInstancedMeshNode>(nodeDefinition);
                auto instanceCount = meshNode->transformRanges[nodeSetup.instanceIndex].numTransforms;
                if (instanceCount != nodeMutation.expectedElements)
                {
                    LogError(R"([{}] {}: The target node #{} has {} instance(s), but the mod expects {}.)",
                             ExtensionName, aSectorMod.mod, nodeMutation.nodeIndex,
                             instanceCount, nodeMutation.expectedElements);
                    nodeValidationPassed = false;
                    continue;
                }
            }
            else if (nodeMutation.nodeType == InstancedDestructibleNodeType)
            {
                auto* meshNode = Red::Cast<Red::worldInstancedDestructibleMeshNode>(nodeDefinition);
                if (meshNode->instanceTransforms.Size() != nodeMutation.expectedElements)
                {
                    LogError(R"([{}] {}: The target node #{} has {} instance(s), but the mod expects {}.)",
                             ExtensionName, aSectorMod.mod, nodeMutation.nodeIndex,
                             meshNode->instanceTransforms.Size(), nodeMutation.expectedElements);
                    nodeValidationPassed = false;
                    continue;
                }
            }
        }
    }

    for (const auto& nodeDeletion : aSectorMod.nodeDeletions)
    {
        auto& nodeSetup = aSector->compiled.buffer.nodeSetups.At(nodeDeletion.nodeIndex);
        auto* nodeDefinition = aSector->compiled.nodes[nodeSetup.nodeIndex].instance;

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
                auto collisionNode = Red::Cast<Red::worldCollisionNode>(nodeDefinition);

                if (collisionNode->numActors != nodeDeletion.expectedElements)
                {
                    LogError(R"([{}] {}: The target node #{} has {} actor(s), but the mod expects {}.)",
                             ExtensionName, aSectorMod.mod, nodeDeletion.nodeIndex,
                             collisionNode->numActors, nodeDeletion.expectedElements);
                    nodeValidationPassed = false;
                    continue;
                }

                if (!nodeDeletion.elementDeletions.empty())
                {
                    auto collisionData = collisionNode->GetCollisionDataView();

                    for (const auto& elementDeletion : nodeDeletion.elementDeletions)
                    {
                        auto hasSubDeletions = false;

                        if (elementDeletion.subElementIndex >= 0)
                        {
                            auto& actor = collisionData.actors[elementDeletion.elementIndex];
                            if (elementDeletion.subElementIndex >= actor.numShapes)
                            {
                                LogError(R"([{}] {}: The target node #{} actor #{} has {} shape(s), but the mod expects {} or more.)",
                                         ExtensionName, aSectorMod.mod, nodeDeletion.nodeIndex, elementDeletion.elementIndex,
                                         actor.numShapes, elementDeletion.subElementIndex + 1);
                                nodeValidationPassed = false;
                                continue;
                            }
                        }

                        if (hasSubDeletions)
                        {
                            LogWarning(R"([{}] {}: The target node #{} actor #{} shape deletions cannot be applied.)",
                                       ExtensionName, aSectorMod.mod, nodeDeletion.nodeIndex, elementDeletion.elementIndex);
                        }
                    }
                }
            }
            else if (nodeDeletion.nodeType == CookedCollisionNodeType)
            {
                auto collisionNode = Red::Cast<Red::worldCookedCollisionNode>(nodeDefinition);
                auto innerNodes = collisionNode->GetInnerNodes();
                if (nodeSetup.instanceIndex < innerNodes.Size())
                {
                    auto& innerNode = innerNodes[nodeSetup.instanceIndex];

                    if (innerNode.numActors != nodeDeletion.expectedElements)
                    {
                        LogError(R"([{}] {}: The target node #{} has {} actor(s), but the mod expects {}.)",
                                 ExtensionName, aSectorMod.mod, nodeDeletion.nodeIndex, innerNode.numActors,
                                 nodeDeletion.expectedElements);
                        nodeValidationPassed = false;
                        continue;
                    }

                    if (!nodeDeletion.elementDeletions.empty())
                    {
                        auto collisionData = collisionNode->GetCollisionDataView(nodeSetup.instanceIndex);

                        for (const auto& elementDeletion : nodeDeletion.elementDeletions)
                        {
                            auto hasSubDeletions = false;

                            if (elementDeletion.subElementIndex >= 0)
                            {
                                auto& actor = collisionData.actors[elementDeletion.elementIndex];
                                if (elementDeletion.subElementIndex >= actor.numShapes)
                                {
                                    LogError(R"([{}] {}: The target node #{} actor #{} has {} shape(s), but the mod expects {} or more.)",
                                             ExtensionName, aSectorMod.mod, nodeDeletion.nodeIndex, elementDeletion.elementIndex,
                                             actor.numShapes, elementDeletion.subElementIndex + 1);
                                    nodeValidationPassed = false;
                                    continue;
                                }
                            }

                            if (hasSubDeletions)
                            {
                                LogWarning(R"([{}] {}: The target node #{} actor #{} shape deletions cannot be applied.)",
                                           ExtensionName, aSectorMod.mod, nodeDeletion.nodeIndex, elementDeletion.elementIndex);
                            }
                        }
                    }
                }
            }
            else if (nodeDeletion.nodeType == InstancedMeshNodeType)
            {
                auto* meshNode = Red::Cast<Red::worldInstancedMeshNode>(nodeDefinition);
                auto instanceCount = meshNode->transformRanges[nodeSetup.instanceIndex].numTransforms;
                if (instanceCount != nodeDeletion.expectedElements)
                {
                    LogError(R"([{}] {}: The target node #{} has {} instance(s), but the mod expects {}.)",
                             ExtensionName, aSectorMod.mod, nodeDeletion.nodeIndex,
                             instanceCount, nodeDeletion.expectedElements);
                    nodeValidationPassed = false;
                    continue;
                }
            }
            else if (nodeDeletion.nodeType == InstancedDestructibleNodeType)
            {
                auto* meshNode = Red::Cast<Red::worldInstancedDestructibleMeshNode>(nodeDefinition);
                if (meshNode->instanceTransforms.Size() != nodeDeletion.expectedElements)
                {
                    LogError(R"([{}] {}: The target node #{} has {} instance(s), but the mod expects {}.)",
                             ExtensionName, aSectorMod.mod, nodeDeletion.nodeIndex,
                             meshNode->instanceTransforms.Size(), nodeDeletion.expectedElements);
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
        auto& nodeSetup = aSector->compiled.buffer.nodeSetups.At(nodeMutation.nodeIndex);
        auto* nodeDefinition = aSector->compiled.nodes[nodeSetup.nodeIndex].instance;

        if (nodeMutation.modifyPosition)
        {
            nodeSetup.transform.position = nodeMutation.position;
        }

        if (nodeMutation.modifyOrientation)
        {
            nodeSetup.transform.orientation = nodeMutation.orientation;
        }

        if (nodeMutation.modifyScale)
        {
            nodeSetup.scale = nodeMutation.scale;
        }

        if (nodeMutation.modifyResource)
        {
            if (auto* meshNode = Red::Cast<Red::worldMeshNode>(nodeDefinition))
            {
                meshNode->mesh.path = nodeMutation.resourcePath;
            }
            else if (auto* instancedMeshNode = Red::Cast<Red::worldInstancedMeshNode>(nodeDefinition))
            {
                if (nodeSetup.instanceIndex < instancedMeshNode->cookedInstances.Size())
                {
                    instancedMeshNode->cookedInstances[nodeSetup.instanceIndex].mesh.path = nodeMutation.resourcePath;
                }
            }
            else if (auto* cookedMeshNode = Red::Cast<Red::worldCookedStaticMeshNode>(nodeDefinition))
            {
                auto innerNodes = cookedMeshNode->GetInnerNodes();
                if (nodeSetup.instanceIndex < innerNodes.Size())
                {
                    innerNodes[nodeSetup.instanceIndex].mesh.path = nodeMutation.resourcePath;
                }
            }
            else if (auto* cookedDecalNode = Red::Cast<Red::worldCookedStaticDecalNode>(nodeDefinition))
            {
                auto innerNodes = cookedDecalNode->GetInnerNodes();
                if (nodeSetup.instanceIndex < innerNodes.Size())
                {
                    innerNodes[nodeSetup.instanceIndex].material.path = nodeMutation.resourcePath;
                }
            }
            else if (auto* cookedProxyNode = Red::Cast<Red::worldCookedGenericProxyMeshNode>(nodeDefinition))
            {
                auto innerNodes = cookedProxyNode->GetInnerNodes();
                if (nodeSetup.instanceIndex < innerNodes.Size())
                {
                    innerNodes[nodeSetup.instanceIndex].mesh.path = nodeMutation.resourcePath;
                }
            }
            else if (auto* cookedAcousticNode = Red::Cast<Red::worldCookedAcousticSectorNode>(nodeDefinition))
            {
                auto innerNodes = cookedAcousticNode->GetInnerNodes();
                if (nodeSetup.instanceIndex < innerNodes.Size())
                {
                    innerNodes[nodeSetup.instanceIndex].data.path = nodeMutation.resourcePath;
                }
            }
            else if (auto* entityNode = Red::Cast<Red::worldEntityNode>(nodeDefinition))
            {
                entityNode->entityTemplate.path = nodeMutation.resourcePath;
            }
            else if (auto* effectNode = Red::Cast<Red::worldEffectNode>(nodeDefinition))
            {
                effectNode->effect.path = nodeMutation.resourcePath;
            }
            else if (auto* decalNode = Red::Cast<Red::worldStaticDecalNode>(nodeDefinition))
            {
                decalNode->material.path = nodeMutation.resourcePath;
            }
            else if (auto* physicalDestructionNode = Red::Cast<Red::worldPhysicalDestructionNode>(nodeDefinition))
            {
                physicalDestructionNode->mesh.path = nodeMutation.resourcePath;
            }
            else if (auto* staticOccluderNode = Red::Cast<Red::worldStaticOccluderMeshNode>(nodeDefinition))
            {
                staticOccluderNode->mesh.path = nodeMutation.resourcePath;
            }
            else if (auto* instancedOccluderNode = Red::Cast<Red::worldInstancedOccluderNode>(nodeDefinition))
            {
                instancedOccluderNode->mesh.path = nodeMutation.resourcePath;
            }
            else if (auto* foliageNode = Red::Cast<Red::worldFoliageNode>(nodeDefinition))
            {
                foliageNode->mesh.path = nodeMutation.resourcePath;
            }
            else if (auto* terrainMeshNode = Red::Cast<Red::worldTerrainMeshNode>(nodeDefinition))
            {
                terrainMeshNode->meshRef.path = nodeMutation.resourcePath;
            }
        }

        if (nodeMutation.modifyAppearance)
        {
            if (auto* meshNode = Red::Cast<Red::worldMeshNode>(nodeDefinition))
            {
                meshNode->meshAppearance = nodeMutation.appearanceName;
            }
            else if (auto* instancedMeshNode = Red::Cast<Red::worldInstancedMeshNode>(nodeDefinition))
            {
                if (nodeSetup.instanceIndex < instancedMeshNode->cookedInstances.Size())
                {
                    instancedMeshNode->cookedInstances[nodeSetup.instanceIndex].meshAppearance = nodeMutation.appearanceName;
                }
            }
            else if (auto* cookedMeshNode = Red::Cast<Red::worldCookedStaticMeshNode>(nodeDefinition))
            {
                auto innerNodes = cookedMeshNode->GetInnerNodes();
                if (nodeSetup.instanceIndex < innerNodes.Size())
                {
                    innerNodes[nodeSetup.instanceIndex].meshAppearance = nodeMutation.appearanceName;
                }
            }
            else if (auto* cookedProxyNode = Red::Cast<Red::worldCookedGenericProxyMeshNode>(nodeDefinition))
            {
                auto innerNodes = cookedProxyNode->GetInnerNodes();
                if (nodeSetup.instanceIndex < innerNodes.Size())
                {
                    innerNodes[nodeSetup.instanceIndex].meshAppearance = nodeMutation.appearanceName;
                }
            }
            else if (auto* entityNode = Red::Cast<Red::worldEntityNode>(nodeDefinition))
            {
                entityNode->appearanceName = nodeMutation.appearanceName;
            }
            else if (auto* spawnerNode = Red::Cast<Red::worldPopulationSpawnerNode>(nodeDefinition))
            {
                spawnerNode->appearanceName = nodeMutation.appearanceName;
            }
            else if (auto* physicalDestructionNode = Red::Cast<Red::worldPhysicalDestructionNode>(nodeDefinition))
            {
                physicalDestructionNode->meshAppearance = nodeMutation.appearanceName;
            }
            else if (auto* foliageNode = Red::Cast<Red::worldFoliageNode>(nodeDefinition))
            {
                foliageNode->meshAppearance = nodeMutation.appearanceName;
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
            else if (auto* cookedPoxyDefinition = Red::Cast<Red::worldCookedGenericProxyMeshNode>(nodeDefinition))
            {
                auto innerNodes = cookedPoxyDefinition->GetInnerNodes();
                if (nodeSetup.instanceIndex < innerNodes.Size())
                {
                    innerNodes.At(nodeSetup.instanceIndex).nbNodesUnderProxy += nodeMutation.nbNodesUnderProxyDiff;
                }
            }
        }

        if (!nodeMutation.elementMutations.empty())
        {
            if (nodeMutation.nodeType == CollisionNodeType)
            {
                auto* collisionNode = Red::Cast<Red::worldCollisionNode>(nodeDefinition);
                auto collisionData = collisionNode->GetCollisionDataView();

                for (const auto& elementMutation : nodeMutation.elementMutations)
                {
                    auto& actor = collisionData.actors[elementMutation.elementIndex];

                    if (elementMutation.modifyPosition)
                    {
                        actor.transform.Position = elementMutation.position;
                    }

                    if (elementMutation.modifyOrientation)
                    {
                        actor.transform.Orientation = elementMutation.orientation;
                    }
                }

                continue;
            }

            if (nodeMutation.nodeType == CookedCollisionNodeType)
            {
                auto* collisionNode = Red::Cast<Red::worldCookedCollisionNode>(nodeDefinition);
                auto innerNodes = collisionNode->GetInnerNodes();

                if (nodeSetup.instanceIndex < innerNodes.Size())
                {
                    auto collisionData = collisionNode->GetCollisionDataView(nodeSetup.instanceIndex);

                    for (const auto& elementMutation : nodeMutation.elementMutations)
                    {
                        auto& actor = collisionData.actors[elementMutation.elementIndex];

                        if (elementMutation.modifyPosition)
                        {
                            actor.transform.Position = elementMutation.position;
                        }

                        if (elementMutation.modifyOrientation)
                        {
                            actor.transform.Orientation = elementMutation.orientation;
                        }
                    }
                }

                continue;
            }

            if (nodeMutation.nodeType == InstancedMeshNodeType)
            {
                auto meshNode = Red::Cast<Red::worldInstancedMeshNode>(nodeDefinition);
                auto startIndex = meshNode->transformRanges[nodeSetup.instanceIndex].transformStartIndex;

                for (const auto& elementMutation : nodeMutation.elementMutations)
                {
                    auto& instanceTransform = meshNode->cookedWorldTransformsBuffer[startIndex + elementMutation.elementIndex];

                    if (elementMutation.modifyPosition)
                    {
                        instanceTransform.transform.Position = elementMutation.position;
                    }

                    if (elementMutation.modifyOrientation)
                    {
                        instanceTransform.transform.Orientation = elementMutation.orientation;
                    }

                    if (elementMutation.modifyScale)
                    {
                        instanceTransform.scale = elementMutation.scale;
                    }
                }
                continue;
            }

            if (nodeMutation.nodeType == InstancedDestructibleNodeType)
            {
                auto* meshNode = Red::Cast<Red::worldInstancedDestructibleMeshNode>(nodeDefinition);
                auto inverseTransform = nodeSetup.transform.Inverse();

                for (const auto& elementMutation : nodeMutation.elementMutations)
                {
                    auto& instanceTransform = meshNode->instanceTransforms[elementMutation.elementIndex];
                    auto newTransform = nodeSetup.transform;

                    if (elementMutation.modifyPosition)
                    {
                        newTransform.position = elementMutation.position;
                    }

                    if (elementMutation.modifyOrientation)
                    {
                        newTransform.orientation = elementMutation.orientation;
                    }

                    instanceTransform = newTransform * inverseTransform;
                }
                continue;
            }
        }
    }

    for (const auto& nodeDeletion : aSectorMod.nodeDeletions)
    {
        auto& nodeSetup = aSector->compiled.buffer.nodeSetups.At(nodeDeletion.nodeIndex);
        auto* nodeDefinition = aSector->compiled.nodes[nodeSetup.nodeIndex].instance;

        if (!nodeDeletion.elementDeletions.empty())
        {
            if (nodeDeletion.nodeType == CollisionNodeType)
            {
                auto* collisionNode = Red::Cast<Red::worldCollisionNode>(nodeDefinition);
                auto collisionData = collisionNode->GetCollisionDataView();

                for (const auto& elementDeletion : nodeDeletion.elementDeletions)
                {
                    auto& actor = collisionData.actors[elementDeletion.elementIndex];
                    if (elementDeletion.subElementIndex >= 0 && actor.numShapes > 1)
                    {
                        // Unsupported on current patch.
                    }
                    else
                    {
                        actor.transform.Position.z.Bits = DelZ;
                    }
                }

                continue;
            }

            if (nodeDeletion.nodeType == CookedCollisionNodeType)
            {
                auto* collisionNode = Red::Cast<Red::worldCookedCollisionNode>(nodeDefinition);
                auto innerNodes = collisionNode->GetInnerNodes();

                if (nodeSetup.instanceIndex < innerNodes.Size())
                {
                    auto collisionData = collisionNode->GetCollisionDataView(nodeSetup.instanceIndex);

                    for (const auto& elementDeletion : nodeDeletion.elementDeletions)
                    {
                        auto& actor = collisionData.actors[elementDeletion.elementIndex];
                        if (elementDeletion.subElementIndex >= 0 && actor.numShapes > 1)
                        {
                            // Unsupported on current patch.
                        }
                        else
                        {
                            actor.transform.Position.z.Bits = DelZ;
                        }
                    }
                }

                continue;
            }

            if (nodeDeletion.nodeType == InstancedMeshNodeType)
            {
                auto* meshNode = Red::Cast<Red::worldInstancedMeshNode>(nodeDefinition);
                auto startIndex = meshNode->transformRanges[nodeSetup.instanceIndex].transformStartIndex;

                for (const auto& elementDeletion : nodeDeletion.elementDeletions)
                {
                    auto& transform = meshNode->cookedWorldTransformsBuffer[startIndex + elementDeletion.elementIndex];
                    transform.scale.X = 0;
                    transform.scale.Y = 0;
                    transform.scale.Z = 0;
                }

                continue;
            }

            if (nodeDeletion.nodeType == InstancedDestructibleNodeType)
            {
                auto* meshNode = Red::Cast<Red::worldInstancedDestructibleMeshNode>(nodeDefinition);

                for (const auto& elementDeletion : nodeDeletion.elementDeletions)
                {
                    auto& instance = meshNode->instanceTransforms[elementDeletion.elementIndex];
                    instance.position.Z = -2000;
                }

                continue;
            }
        }

        if (nodeDeletion.nodeType == InstancedMeshNodeType)
        {
            auto* meshNode = Red::Cast<Red::worldInstancedMeshNode>(nodeDefinition);
            auto startIndex = meshNode->transformRanges[nodeSetup.instanceIndex].transformStartIndex;
            auto numElements = meshNode->transformRanges[nodeSetup.instanceIndex].numTransforms;
            for (auto i = 0; i < numElements; ++i)
            {
                auto& transform = meshNode->cookedWorldTransformsBuffer[startIndex + i];
                transform.scale.X = 0;
                transform.scale.Y = 0;
                transform.scale.Z = 0;
            }
            continue;
        }

        nodeSetup.scale.X = 0;
        nodeSetup.scale.Y = 0;
        nodeSetup.scale.Z = 0;

        if (nodeDeletion.nodeType == StaticMeshNodeType || nodeDeletion.nodeType == CookedStaticMeshNodeType)
        {
            continue;
        }

        nodeSetup.node = s_dummyNode.instance;

        if (nodeDeletion.nodeType == AdvertisementNodeType)
        {
            continue;
        }

        nodeSetup.transform.position.Z = -2000;
        nodeSetup.globalNodeID = 0;

        if (aSector->compiled.buffer.version >= 68)
        {
            nodeSetup.instanceIndex = -1;
        }
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
