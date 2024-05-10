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
    s_scopes.clear();
    s_fixes.clear();

    for (auto& config : m_configs)
    {
        for (const auto& [scopePath, targetList] : config.scopes)
        {
            s_paths[scopePath] = config.paths[scopePath];

            for (const auto& targetPath : targetList)
            {
                if (targetPath == scopePath)
                    continue;

                s_scopes[scopePath].insert(targetPath);
                s_paths[targetPath] = config.paths[targetPath];
            }
        }

        for (auto [targetPath, targetFix] : config.fixes)
        {
            s_fixes[targetPath] = std::move(targetFix);
            s_paths[targetPath] = config.paths[targetPath];
        }
    }

    for (auto& [scopePath, targetList] : s_scopes)
    {
        bool updated;
        do
        {
            updated = false;
            for (const auto& targetPath : targetList)
            {
                const auto& includePaths = s_scopes.find(targetPath);
                if (includePaths != s_scopes.end())
                {
                    s_scopes[scopePath].erase(targetPath);
                    s_scopes[scopePath].insert(includePaths.value().begin(), includePaths.value().end());
                    updated = true;
                    break;
                }
            }
        }
        while (updated);
    }

    m_configs.clear();
}

const Core::Set<Red::ResourcePath>& App::ResourceMetaModule::GetResourceList(Red::ResourcePath aScopePath)
{
    static const Core::Set<Red::ResourcePath> s_null;
    const auto& it = s_scopes.find(aScopePath);

    if (it == s_scopes.end())
        return s_null;

    return it.value();
}

bool App::ResourceMetaModule::IsInResourceList(Red::ResourcePath aScopePath, Red::ResourcePath aTargetPath)
{
    const auto& it = s_scopes.find(aScopePath);

    if (it == s_scopes.end())
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
