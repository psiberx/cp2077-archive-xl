#include "Module.hpp"

namespace
{
constexpr auto ModuleName = "ResourceMeta";
}

std::string_view App::ResourceMetaModule::GetName()
{
    return ModuleName;
}

void App::ResourceMetaModule::Configure()
{
    s_aliases.clear();
    s_fixes.clear();

    for (auto& unit : m_configs)
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

        for (auto [targetPath, targetFix] : unit.fixes)
        {
            s_fixes[targetPath] = std::move(targetFix);
            s_paths[targetPath] = unit.paths[targetPath];
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

    m_configs.clear();
}

const Core::Set<Red::ResourcePath>& App::ResourceMetaModule::GetResourceList(Red::ResourcePath aAliasPath)
{
    static const Core::Set<Red::ResourcePath> s_null;
    const auto& it = s_aliases.find(aAliasPath);

    if (it == s_aliases.end())
        return s_null;

    return it.value();
}

bool App::ResourceMetaModule::IsInResourceList(Red::ResourcePath aAliasPath, Red::ResourcePath aTargetPath)
{
    const auto& it = s_aliases.find(aAliasPath);

    if (it == s_aliases.end())
        return false;

    return it.value().contains(aTargetPath);
}

const App::ResourceFix& App::ResourceMetaModule::GetResourceFix(Red::ResourcePath aTargetPath)
{
    static const ResourceFix s_null;
    const auto& it = s_fixes.find(aTargetPath);

    if (it == s_fixes.end())
        return s_null;

    return it.value();
}
