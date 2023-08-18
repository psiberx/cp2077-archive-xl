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

    return true;
}

bool App::StreamingModule::Unload()
{
    Unhook<Raw::StreamingWorld::OnLoad>();

    return true;
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
            LogInfo("|{}| Processing \"{}\"...", ModuleName, unit.name);

            for (const auto& path : unit.blocks)
            {
                StreamingBlockRef blockRef(path.c_str());
                blockRef.LoadAsync();

                blockPaths.insert({blockRef.path, path.c_str()});
                blockRefs.emplace_back(std::move(blockRef));
            }
        }

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
