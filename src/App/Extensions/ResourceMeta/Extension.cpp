#include "Extension.hpp"
#include "App/Shared/ResourcePathRegistry.hpp"
#include "Core/Facades/Container.hpp"

namespace
{
constexpr auto ExtensionName = "ResourceManager";
}

std::string_view App::ResourceMetaExtension::GetName()
{
    return ExtensionName;
}

void App::ResourceMetaExtension::Configure()
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
            s_fixes[targetPath].Merge(targetFix);
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

    auto resourcePathRegistry = Core::Resolve<ResourcePathRegistry>();
    for (const auto& [knownPath, knownPathStr] : s_paths)
    {
        resourcePathRegistry->RegisterPath(knownPath, knownPathStr);
    }

    m_configs.clear();
}

bool App::ResourceMetaExtension::InScope(Red::ResourcePath aScopePath, Red::ResourcePath aTargetPath)
{
    const auto& it = s_scopes.find(aScopePath);

    if (it == s_scopes.end())
        return false;

    return it.value().contains(aTargetPath);
}

const Core::Set<Red::ResourcePath>& App::ResourceMetaExtension::GetList(Red::ResourcePath aScopePath)
{
    static const Core::Set<Red::ResourcePath> s_null;
    const auto& it = s_scopes.find(aScopePath);

    if (it == s_scopes.end())
        return s_null;

    return it.value();
}

Core::Set<std::string> App::ResourceMetaExtension::ExpandList(const Core::Set<std::string>& aList)
{
    Core::Set<std::string> result;

    for (const auto& targetPath : aList)
    {
        const auto& scopeList = GetList(targetPath.data());
        if (!scopeList.empty())
        {
            for (const auto& expandedPath : scopeList)
            {
                result.insert(s_paths[expandedPath]);
            }
        }
        else
        {
            result.insert(targetPath);
        }
    }

    return result;
}

Core::Set<Red::ResourcePath> App::ResourceMetaExtension::ExpandList(const Core::Set<Red::ResourcePath>& aList)
{
    Core::Set<Red::ResourcePath> result;

    for (const auto& targetPath : aList)
    {
        const auto& scopePaths = GetList(targetPath);
        if (!scopePaths.empty())
        {
            result.insert(scopePaths.begin(), scopePaths.end());
        }
        else
        {
            result.insert(targetPath);
        }
    }

    return result;
}

Core::Map<Red::ResourcePath, std::string> App::ResourceMetaExtension::ExpandList(
    const Core::Map<Red::ResourcePath, std::string>& aList)
{
    Core::Map<Red::ResourcePath, std::string> result;

    for (const auto& [targetPath, targetPathStr] : aList)
    {
        const auto& scopeList = GetList(targetPath);
        if (!scopeList.empty())
        {
            for (const auto& expandedPath : scopeList)
            {
                result.insert_or_assign(expandedPath, s_paths[expandedPath]);
            }
        }
        else
        {
            result.insert_or_assign(targetPath, targetPathStr);
        }
    }

    return result;
}

const App::ResourceFix& App::ResourceMetaExtension::GetFix(Red::ResourcePath aTargetPath)
{
    static const ResourceFix s_null;
    const auto& it = s_fixes.find(aTargetPath);

    if (it == s_fixes.end())
        return s_null;

    return it.value();
}
