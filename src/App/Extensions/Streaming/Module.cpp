#include "Module.hpp"

namespace
{
constexpr auto ModuleName = "Streaming";

constexpr auto MainWorldResource = Red::ResourcePath(R"(base\worlds\03_night_city\_compiled\default\03_night_city.streamingworld)");
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

            for (const auto& deletion : sectorMod.deletions)
            {
                if (!rtti->GetClass(deletion.nodeType))
                {
                    allNodeTypesValid = false;

                    if (!invalidNodeTypes.contains(deletion.nodeType))
                    {
                        LogWarning("|{}| Node type \"{}\" doesn't exist. Skipped.", ModuleName,
                                   deletion.nodeType.ToString());
                        invalidNodeTypes.insert(deletion.nodeType);
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
            LogInfo("|{}| No entries to merge.", ModuleName);
        }
    }
    else
    {
        LogInfo("|{}| No entries to merge.", ModuleName);
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
                for (const auto& deletion : sectorMod.deletions)
                {
                    if (deletion.nodeType != buffer.nodes[deletion.nodeIndex]->GetNativeType()->name)
                    {
                        isValidSector = false;
                        break;
                    }
                }
            }

            if (isValidSector)
            {
                for (const auto& deletion : sectorMod.deletions)
                {
                    buffer.nodes[deletion.nodeIndex]->isVisibleInGame = false;
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
