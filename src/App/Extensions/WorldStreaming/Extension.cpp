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

// Game 2.5 keeps the resolved node pointer in the setup (+0x50, written by the node fix-up),
// the packed node index is only a fallback and is bounds checked.
Red::worldNode* ResolveNode(Red::StreamingSectorNodeBuffer& aBuffer, Red::CompiledNodeInstanceSetupInfo* aNodeSetup)
{
    if (aNodeSetup->node)
        return aNodeSetup->node;

    if (aNodeSetup->nodeIndex < aBuffer.nodes.Size())
        return aBuffer.nodes[static_cast<uint32_t>(aNodeSetup->nodeIndex)].instance;

    return nullptr;
}
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
        auto* nodeDefinition = ResolveNode(buffer, nodeSetup);

        if (!nodeDefinition)
        {
            LogError(R"([{}] {}: The target node #{} can't be resolved (node index {} of {}).)",
                     ExtensionName, aSectorMod.mod, nodeMutation.nodeIndex,
                     static_cast<uint32_t>(nodeSetup->nodeIndex), buffer.nodes.Size());
            nodeValidationPassed = false;
            continue;
        }

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

        // Game 2.5 changed the layouts of collision and instanced mesh nodes (the instance transforms are now
        // cooked), so element level edits can't be applied safely yet. Whole node edits still work.
        if (!nodeMutation.elementMutations.empty())
        {
            LogError(R"([{}] {}: The target node #{} has type {}, element edits for this type are not supported on game 2.5.)",
                     ExtensionName, aSectorMod.mod, nodeMutation.nodeIndex, nodeMutation.nodeType.ToString());
            nodeValidationPassed = false;
            continue;
        }
    }

    for (const auto& nodeDeletion : aSectorMod.nodeDeletions)
    {
        auto* nodeSetup = buffer.nodeSetups.GetInstance(nodeDeletion.nodeIndex);
        auto* nodeDefinition = ResolveNode(buffer, nodeSetup);

        if (!nodeDefinition)
        {
            LogError(R"([{}] {}: The target node #{} can't be resolved (node index {} of {}).)",
                     ExtensionName, aSectorMod.mod, nodeDeletion.nodeIndex,
                     static_cast<uint32_t>(nodeSetup->nodeIndex), buffer.nodes.Size());
            nodeValidationPassed = false;
            continue;
        }

        if (nodeDeletion.nodeType != nodeDefinition->GetNativeType()->name)
        {
            LogError(R"([{}] {}: The target node #{} has type {}, but the mod expects {}.)",
                     ExtensionName, aSectorMod.mod, nodeDeletion.nodeIndex,
                     nodeDefinition->GetNativeType()->name.ToString(),
                     nodeDeletion.nodeType.ToString());
            nodeValidationPassed = false;
            continue;
        }

        // Game 2.5 changed the layouts of collision and instanced mesh nodes (the instance transforms are now
        // cooked), so element level edits can't be applied safely yet. Whole node edits still work.
        if (!nodeDeletion.elementDeletions.empty())
        {
            LogError(R"([{}] {}: The target node #{} has type {}, element edits for this type are not supported on game 2.5.)",
                     ExtensionName, aSectorMod.mod, nodeDeletion.nodeIndex, nodeDeletion.nodeType.ToString());
            nodeValidationPassed = false;
            continue;
        }
    }

    if (!nodeValidationPassed)
    {
        return false;
    }

    for (const auto& nodeMutation : aSectorMod.nodeMutations)
    {
        auto* nodeSetup = buffer.nodeSetups.GetInstance(nodeMutation.nodeIndex);
        auto* nodeDefinition = ResolveNode(buffer, nodeSetup);

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

    }

    for (const auto& nodeDeletion : aSectorMod.nodeDeletions)
    {
        auto* nodeSetup = buffer.nodeSetups.GetInstance(nodeDeletion.nodeIndex);
        auto* nodeDefinition = ResolveNode(buffer, nodeSetup);

        if (nodeDeletion.nodeType == StaticMeshNodeType)
        {
            nodeSetup->scale.X = 0;
            nodeSetup->scale.Y = 0;
            nodeSetup->scale.Z = 0;
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
