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

    for (auto& unit : m_units)
    {
        for (const auto& [aliasPath, targetList] : unit.aliases)
        {
            s_paths[aliasPath] = unit.paths[aliasPath];

            for (const auto& targetPath : targetList)
            {
                if (targetPath == aliasPath)
                    continue;

                s_aliases[aliasPath].insert(targetPath);
                s_paths[targetPath] = unit.paths[targetPath];
            }
        }
    }

    for (auto& [aliasPath, targetList] : s_aliases)
    {
        bool updated;
        do
        {
            updated = false;
            for (const auto& targetPath : targetList)
            {
                const auto& includePaths = s_aliases.find(targetPath);
                if (includePaths != s_aliases.end())
                {
                    s_aliases[aliasPath].erase(targetPath);
                    s_aliases[aliasPath].insert(includePaths.value().begin(), includePaths.value().end());
                    updated = true;
                    break;
                }
            }
        }
        while (updated);
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

bool App::ResourceAliasModule::IsAliased(Red::ResourcePath aAliasPath, Red::ResourcePath aTargetPath)
{
    const auto& aliasIt = s_aliases.find(aAliasPath);

    if (aliasIt == s_aliases.end())
        return false;

    return aliasIt.value().contains(aTargetPath);
}
