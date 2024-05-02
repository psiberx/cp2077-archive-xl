#include "Module.hpp"

namespace
{
constexpr auto ModuleName = "ResourceAlias";
}

std::string_view App::ResourceAliasModule::GetName()
{
    return ModuleName;
}

bool App::ResourceAliasModule::Load()
{
    InitializeAliases();

    return true;
}

void App::ResourceAliasModule::Reload()
{
    InitializeAliases();
}

bool App::ResourceAliasModule::Unload()
{
    return true;
}

void App::ResourceAliasModule::InitializeAliases()
{
    s_aliases.clear();

    auto depot = Red::ResourceDepot::Get();
    Core::Set<Red::ResourcePath> invalidPaths;

    for (auto& unit : m_units)
    {
        for (const auto& [aliasPath, targetList] : unit.aliases)
        {
            s_paths[aliasPath] = unit.paths[aliasPath];

            for (const auto& targetPath : targetList)
            {
                if (targetPath == aliasPath)
                    continue;

                if (!depot->ResourceExists(targetPath))
                {
                    if (!invalidPaths.contains(targetPath))
                    {
                        LogWarning("|{}| Resource \"{}\" doesn't exist. Skipped.", ModuleName, unit.paths[targetPath]);
                        invalidPaths.insert(targetPath);
                    }
                    continue;
                }

                s_aliases[aliasPath].insert(targetPath);
                s_paths[targetPath] = unit.paths[targetPath];
            }
        }
    }
}

const Core::Set<Red::ResourcePath>& App::ResourceAliasModule::ResolveAlias(Red::ResourcePath aAliasPath)
{
    static const Core::Set<Red::ResourcePath> s_null;

    const auto& aliasIt = s_aliases.find(aAliasPath);

    if (aliasIt == s_aliases.end())
        return s_null;

    return aliasIt.value();
}
