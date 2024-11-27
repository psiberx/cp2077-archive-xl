#include "Extension.hpp"
#include "Red/CommunitySystem.hpp"

namespace
{
constexpr auto ExtensionName = "WorldStreaming";

constexpr auto MainWorldResource = Red::ResourcePath(R"(base\worlds\03_night_city\_compiled\default\03_night_city.streamingworld)");
constexpr auto CollisionNodeType = Red::GetTypeName<Red::world::CollisionNode>();

Red::Handle<Red::worldAudioTagNode> s_dummyNode;
}

std::string_view App::WorldStreamingExtension::GetName()
{
    return ExtensionName;
}

bool App::WorldStreamingExtension::Load()
{
    HookAfter<Raw::StreamingWorld::OnLoad>(&WorldStreamingExtension::OnWorldLoad).OrThrow();
    HookAfter<Raw::StreamingSector::OnReady>(&OnSectorReady).OrThrow();
    Hook<Raw::AIWorkspotManager::RegisterSpots>(&OnRegisterSpots).OrThrow();

    s_dummyNode = Red::MakeHandle<Red::worldAudioTagNode>();
    s_dummyNode->isVisibleInGame = false;
    s_dummyNode->radius = 0.00001;

    return true;
}

bool App::WorldStreamingExtension::Unload()
{
    Unhook<Raw::StreamingWorld::OnLoad>();
    Unhook<Raw::StreamingSector::OnReady>();
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
                    LogError("|{}| {}: Sector \"{}\" doesn't exist.",
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
                        LogError("|{}| {}: Node type \"{}\" doesn't exist.",
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

void App::WorldStreamingExtension::OnWorldLoad(Red::world::StreamingWorld* aWorld, Red::BaseStream* aStream)
{
    if (aWorld->path != MainWorldResource)
        return;

    LogInfo("|{}| World streaming is initializing...", ExtensionName);

    if (!m_configs.empty())
    {
        Core::Vector<StreamingBlockRef> blockRefs;
        Core::Map<Red::ResourcePath, const char*> blockPaths;

        for (const auto& unit : m_configs)
        {
            if (!unit.blocks.empty())
            {
                LogInfo("|{}| Processing \"{}\"...", ExtensionName, unit.name);

                for (const auto& path : unit.blocks)
                {
                    StreamingBlockRef blockRef(path.c_str());
                    blockRef.LoadAsync();

                    blockPaths.insert({blockRef.path, path.c_str()});
                    blockRefs.emplace_back(std::move(blockRef));
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
                    LogInfo("|{}| Merging streaming block \"{}\"...", ExtensionName, blockPaths[blockRef.path]);

                    aWorld->blockRefs.EmplaceBack(std::move(blockRef));
                }
                else
                {
                    LogError("|{}| Resource \"{}\" failed to load.", ExtensionName, blockPaths[blockRef.path]);

                    allSucceeded = false;
                }
            }

            if (allSucceeded)
                LogInfo("|{}| All streaming blocks merged.", ExtensionName);
            else
                LogWarning("|{}| Streaming blocks merged with issues.", ExtensionName);
        }
        else
        {
            LogInfo("|{}| No blocks to merge.", ExtensionName);
        }
    }
    else
    {
        LogInfo("|{}| No blocks to merge.", ExtensionName);
    }
}

void App::WorldStreamingExtension::OnSectorReady(Red::world::StreamingSector* aSector, uint64_t)
{
    std::shared_lock _(s_sectorsLock);

    const auto& sectorMods = s_sectors.find(aSector->path);

    if (sectorMods == s_sectors.end())
        return;

    const auto& sectorPath = sectorMods.value().begin()->path;
    auto patchedAny = false;
    auto successAll = true;

    LogInfo("|{}| Patching sector \"{}\"...", ExtensionName, sectorPath);

    for (const auto& sectorMod : sectorMods.value())
    {
        LogInfo("|{}| Applying changes from \"{}\"...", ExtensionName, sectorMod.mod);

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
        LogInfo("|{}| All patches have been applied to \"{}\".", ExtensionName, sectorPath);
    else if (patchedAny)
        LogWarning("|{}| Some patches have not been applied to \"{}\".", ExtensionName, sectorPath);
    else
        LogWarning("|{}| No patches have been applied to \"{}\".", ExtensionName, sectorPath);
}

bool App::WorldStreamingExtension::PatchSector(Red::world::StreamingSector* aSector, const App::WorldSectorMod& aSectorMod)
{
    auto& buffer = Raw::StreamingSector::NodeBuffer::Ref(aSector);
    auto nodeCount = buffer.nodeSetups.GetInstanceCount();

    if (buffer.nodeSetups.GetInstanceCount() != aSectorMod.expectedNodes)
    {
        LogError(R"(|{}| {}: The target sector has {} node(s), but the mod expects {}.)",
                 ExtensionName, aSectorMod.mod, nodeCount, aSectorMod.expectedNodes);
        return false;
    }

    bool nodeValidationPassed = true;

    for (const auto& nodeMutation : aSectorMod.nodeMutations)
    {
        auto* nodeInstance = buffer.nodeSetups.GetInstance(nodeMutation.nodeIndex);
        auto* nodeDefinition = buffer.nodes[nodeInstance->nodeIndex].instance;

        if (nodeMutation.nodeType != nodeDefinition->GetNativeType()->name)
        {
            LogError(R"(|{}| {}: The target node #{} has type {}, but the mod expects {}.)",
                     ExtensionName, aSectorMod.mod,
                     nodeMutation.nodeIndex,
                     nodeDefinition->GetNativeType()->name.ToString(),
                     nodeMutation.nodeType.ToString());
            nodeValidationPassed = false;
        }
    }

    for (const auto& nodeDeletion : aSectorMod.nodeDeletions)
    {
        auto* nodeInstance = buffer.nodeSetups.GetInstance(nodeDeletion.nodeIndex);
        auto* nodeDefinition = buffer.nodes[nodeInstance->nodeIndex].instance;

        if (nodeDeletion.nodeType != nodeDefinition->GetNativeType()->name)
        {
            LogError(R"(|{}| {}: The target node #{} has type {}, but the mod expects {}.)",
                     ExtensionName, aSectorMod.mod, nodeDeletion.nodeIndex,
                     nodeDefinition->GetNativeType()->name.ToString(),
                     nodeDeletion.nodeType.ToString());
            nodeValidationPassed = false;
            continue;
        }

        if (nodeDeletion.nodeType == CollisionNodeType && !nodeDeletion.subNodeDeletions.empty())
        {
            auto& actors = Raw::CollisionNode::Actors::Ref(nodeDefinition);
            if (actors.GetSize() != nodeDeletion.expectedSubNodes)
            {
                LogError(R"(|{}| {}: The target node #{} has {} actor(s), but the mod expects {}.)",
                         ExtensionName, aSectorMod.mod, nodeDeletion.nodeIndex,
                         actors.GetSize(), nodeDeletion.expectedSubNodes);
                nodeValidationPassed = false;
                continue;
            }
        }
    }

    if (!nodeValidationPassed)
    {
        return false;
    }

    for (const auto& nodeMutation : aSectorMod.nodeMutations)
    {
        auto* nodeInstance = buffer.nodeSetups.GetInstance(nodeMutation.nodeIndex);

        nodeInstance->transform.position = nodeMutation.position;
        nodeInstance->transform.orientation = nodeMutation.orientation;
        nodeInstance->scale = nodeMutation.scale;
    }

    for (const auto& nodeDeletion : aSectorMod.nodeDeletions)
    {
        auto* nodeInstance = buffer.nodeSetups.GetInstance(nodeDeletion.nodeIndex);
        auto* nodeDefinition = buffer.nodes[nodeInstance->nodeIndex].instance;

        if (nodeDeletion.nodeType == CollisionNodeType)
        {
            if (!nodeDeletion.subNodeDeletions.empty())
            {
                auto& actors = Raw::CollisionNode::Actors::Ref(nodeDefinition);
                for (const auto& subNodeIndex : nodeDeletion.subNodeDeletions)
                {
                    constexpr auto DelZ = static_cast<int32_t>(-2000 * (2 << 16));
                    actors.beginPtr[subNodeIndex].transform.Position.z.Bits = DelZ;
                }
                continue;
            }
        }

        nodeInstance->transform.position.Z = -2000;
        nodeInstance->scale.X = 0;
        nodeInstance->scale.Y = 0;
        nodeInstance->scale.Z = 0;

        nodeInstance->node = s_dummyNode.instance;
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
