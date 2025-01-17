#include "Extension.hpp"

namespace
{
constexpr auto ExtensionName = "ResourceLink";
}

std::string_view App::ResourceLinkExtension::GetName()
{
    return ExtensionName;
}

bool App::ResourceLinkExtension::Load()
{
    HookBefore<Raw::ResourceLoader::RequestResource>(&OnResourceRequest).OrThrow();
    Hook<Raw::ResourceDepot::CheckResource>(&OnResourceCheck).OrThrow();

    return true;
}

bool App::ResourceLinkExtension::Unload()
{
    Unhook<Raw::ResourceLoader::RequestResource>();
    Unhook<Raw::ResourceDepot::CheckResource>();

    return true;
}

void App::ResourceLinkExtension::Configure()
{
    s_links.clear();

    auto depot = Red::ResourceDepot::Get();
    Core::Set<Red::ResourcePath> invalidPaths;

    for (auto& config : m_configs)
    {
        for (const auto& [targetPath, linkPaths] : config.links)
        {
            for (const auto& linkPath : linkPaths)
            {
                if (linkPath == targetPath)
                {
                    if (!invalidPaths.contains(linkPath))
                    {
                        LogError("[{}] Link \"{}\" points to itself.", ExtensionName, config.paths[linkPath]);
                        invalidPaths.insert(linkPath);
                    }
                    continue;
                }

                if (depot->ResourceExists(linkPath))
                {
                    if (!invalidPaths.contains(linkPath))
                    {
                        LogError("[{}] Link \"{}\" is an existing resource.", ExtensionName, config.paths[linkPath]);
                        invalidPaths.insert(linkPath);
                    }
                    continue;
                }

                // TODO: Warn about redefinitions?

                s_links[linkPath] = targetPath;
                s_paths[linkPath] = config.paths[linkPath];
            }

            s_paths[targetPath] = config.paths[targetPath];
        }
    }

    Core::Set<Red::ResourcePath> cyclicLinks;

    for (auto link = s_links.begin(); link != s_links.end();)
    {
        const auto& linkPath = link->first;
        auto targetPath = link->second;
        auto isCyclic = false;

        Core::Set<Red::ResourcePath> hops;
        hops.insert(linkPath);
        hops.insert(targetPath);

        while (true)
        {
            const auto& next = s_links.find(targetPath);

            if (next == s_links.end())
                break;

            targetPath = next->second;

            if (hops.contains(targetPath))
            {
                isCyclic = true;
                break;
            }

            hops.insert(targetPath);
        }

        if (isCyclic)
        {
            cyclicLinks.insert(linkPath);
        }
        else
        {
            link.value() = targetPath;
        }

        ++link;
    }

    for (const auto& linkPath : cyclicLinks)
    {
        if (!invalidPaths.contains(linkPath))
        {
            LogError("[{}] Link \"{}\" is cyclic.", ExtensionName, s_paths[linkPath]);
            invalidPaths.insert(linkPath);
        }

        s_links.erase(linkPath);
    }

    for (auto link = s_links.begin(); link != s_links.end();)
    {
        const auto& linkPath = link->first;
        const auto& targetPath = link->second;

        if (!depot->ResourceExists(targetPath))
        {
            if (!invalidPaths.contains(targetPath))
            {
                LogError(R"([{}] Link "{}" points to a non-existent resource "{}".)",
                         ExtensionName, s_paths[linkPath], s_paths[targetPath]);
                invalidPaths.insert(targetPath);
            }

            link = s_links.erase(link);
            continue;
        }

        ++link;
    }

    m_configs.clear();
}

void App::ResourceLinkExtension::OnResourceRequest(Red::ResourceLoader*, Red::SharedPtr<Red::ResourceToken<>>&,
                                                Red::ResourceRequest& aRequest)
{
    const auto& link = s_links.find(aRequest.path);
    if (link != s_links.end())
    {
        aRequest.path = link->second;
    }
}

bool App::ResourceLinkExtension::OnResourceCheck(Red::ResourceDepot* aDepot, Red::ResourcePath aPath)
{
    const auto& link = s_links.find(aPath);
    if (link != s_links.end())
    {
        aPath = link->second;
    }

    return Raw::ResourceDepot::CheckResource(aDepot, aPath);
}
