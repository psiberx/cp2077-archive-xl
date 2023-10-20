#include "Module.hpp"

namespace
{
constexpr auto ModuleName = "Streaming";

constexpr auto MainWorldResource = Red::ResourcePath(R"(base\worlds\03_night_city\_compiled\default\03_night_city.streamingworld)");
constexpr auto CollisionNodeType = Red::GetTypeName<Red::world::CollisionNode>();
}

std::string_view App::StreamingModule::GetName()
{
    return ModuleName;
}

bool App::StreamingModule::Load()
{
    if (!HookAfter<Raw::StreamingWorld::OnLoad>(&StreamingModule::OnWorldLoad))
        throw std::runtime_error("Failed to hook [StreamingWorld::OnLoad].");

    if (!HookBefore<Raw::StreamingSector::OnReady>(&OnSectorReady))
        throw std::runtime_error("Failed to hook [StreamingSector::OnReady].");

    PrepareSectors();

    return true;
}

void App::StreamingModule::Reload()
{
    PrepareSectors();
}

bool App::StreamingModule::Unload()
{
    Unhook<Raw::StreamingWorld::OnLoad>();
    Unhook<Raw::StreamingSector::OnReady>();

    return true;
}

void App::StreamingModule::PrepareSectors()
{
    s_sectors.clear();

    auto depot = Red::ResourceDepot::Get();
    auto rtti = Red::CRTTISystem::Get();

    Core::Set<Red::ResourcePath> invalidPaths;
    Core::Set<Red::CName> invalidNodeTypes;

    for (auto& unit : m_units)
    {
        for (auto& sectorMod : unit.sectors)
        {
            auto sectorPath = Red::ResourcePath(sectorMod.path.data());

            if (!depot->ResourceExists(sectorPath))
            {
                if (!invalidPaths.contains(sectorPath))
                {
                    LogWarning("|{}| Sector \"{}\" doesn't exist. Skipped.", ModuleName, sectorMod.path);
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
                        LogWarning("|{}| Node type \"{}\" doesn't exist. Skipped.", ModuleName,
                                   nodeDeletion.nodeType.ToString());
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

void App::StreamingModule::OnWorldLoad(Red::world::StreamingWorld* aWorld, Red::BaseStream* aStream)
{
    if (aWorld->path != MainWorldResource)
        return;

    LogInfo("|{}| World streaming is initializing...", ModuleName);

    if (!m_units.empty())
    {
        Core::Vector<StreamingBlockRef> blockRefs;
        Core::Map<Red::ResourcePath, const char*> blockPaths;

        for (const auto& unit : m_units)
        {
            if (!unit.blocks.empty())
            {
                LogInfo("|{}| Processing \"{}\"...", ModuleName, unit.name);

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
                    LogInfo("|{}| Merging streaming block \"{}\"...", ModuleName, blockPaths[blockRef.path]);

                    aWorld->blockRefs.EmplaceBack(std::move(blockRef));
                }
                else
                {
                    LogError("|{}| Resource \"{}\" failed to load.", ModuleName, blockPaths[blockRef.path]);

                    allSucceeded = false;
                }
            }

            if (allSucceeded)
                LogInfo("|{}| All streaming blocks merged.", ModuleName);
            else
                LogWarning("|{}| Streaming blocks merged with issues.", ModuleName);
        }
        else
        {
            LogInfo("|{}| No blocks to merge.", ModuleName);
        }
    }
    else
    {
        LogInfo("|{}| No blocks to merge.", ModuleName);
    }
}

void App::StreamingModule::OnSectorReady(Red::world::StreamingSector* aSector, uint64_t)
{
    const auto& sectorMods = s_sectors.find(aSector->path);
    if (sectorMods != s_sectors.end())
    {
        auto& buffer = Raw::StreamingSector::NodeBuffer::Ref(aSector);

        LogInfo("|{}| Patching \"{}\"...", ModuleName, sectorMods.value().begin()->path);

        for (const auto& sectorMod : sectorMods.value())
        {
            auto isValidSector = true;

            if (buffer.nodes.size != sectorMod.expectedNodes)
            {
                isValidSector = false;
            }
            else
            {
                for (const auto& nodeDeletion : sectorMod.nodeDeletions)
                {
                    auto& node = buffer.nodes[nodeDeletion.nodeIndex];

                    if (nodeDeletion.nodeType != node->GetNativeType()->name)
                    {
                        isValidSector = false;
                        break;
                    }

                    if (nodeDeletion.nodeType == CollisionNodeType && !nodeDeletion.subNodeDeletions.empty())
                    {
                        auto& actors = Raw::CollisionNode::Actors::Ref(node);
                        if (actors.GetSize() != nodeDeletion.expectedSubNodes)
                        {
                            isValidSector = false;
                            break;
                        }
                    }
                }
            }

            if (isValidSector)
            {
                for (const auto& nodeDeletion : sectorMod.nodeDeletions)
                {
                    auto& node = buffer.nodes[nodeDeletion.nodeIndex];

                    if (nodeDeletion.nodeType == CollisionNodeType && !nodeDeletion.subNodeDeletions.empty())
                    {
                        auto& actors = Raw::CollisionNode::Actors::Ref(node);
                        for (const auto& subNodeIndex : nodeDeletion.subNodeDeletions)
                        {
                            constexpr auto DelZ = static_cast<int32_t>(-2000 * (2 << 16));
                            actors.ptr[subNodeIndex].transform.Position.z.Bits = DelZ;
                        }
                        continue;
                    }

                    node->isVisibleInGame = false;
                }

                LogInfo("|{}| Sector \"{}\" patched with \"{}\".", ModuleName, sectorMod.path, sectorMod.mod);
            }
            else
            {
                LogWarning("|{}| Sector \"{}\" patching failed for \"{}\".", ModuleName, sectorMod.path, sectorMod.mod);
            }
        }
    }
}
