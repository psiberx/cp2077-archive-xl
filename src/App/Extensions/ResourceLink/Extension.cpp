#include "Extension.hpp"
#include "App/Shared/ResourcePathRegistry.hpp"
#include "Core/Facades/Container.hpp"

namespace
{
constexpr auto ExtensionName = "ResourceManager";
}

std::string_view App::ResourceLinkExtension::GetName()
{
    return ExtensionName;
}

bool App::ResourceLinkExtension::Load()
{
    HookBefore<Raw::ResourceLoader::LoadAsync>(&OnLoaderResourceRequest).OrThrow();
    Hook<Raw::ResourceDepot::RequestResource>(&OnDepotResourceRequest).OrThrow();
    Hook<Raw::ResourceDepot::CheckResource>(&OnDepotResourceCheck).OrThrow();

    return true;
}

bool App::ResourceLinkExtension::Unload()
{
    Unhook<Raw::ResourceLoader::LoadAsync>();
    Unhook<Raw::ResourceDepot::RequestResource>();
    Unhook<Raw::ResourceDepot::CheckResource>();

    return true;
}

void App::ResourceLinkExtension::Configure()
{
    s_mappings.clear();
    s_copies.clear();
    s_links.clear();

    auto depot = Red::ResourceDepot::Get();

    Core::Map<Red::ResourcePath, Red::ResourcePath> finalCopies;
    Core::Map<Red::ResourcePath, Red::ResourcePath> finalLinks;
    Core::Map<Red::ResourcePath, std::string> knownPaths;
    Core::Set<Red::ResourcePath> invalidPaths;

    for (auto& config : m_configs)
    {
        for (const auto& [targetPath, copyPaths] : config.copies)
        {
            for (const auto& copyPath : copyPaths)
            {
                if (copyPath == targetPath)
                {
                    if (!invalidPaths.contains(copyPath))
                    {
                        LogError("[{}] Copy \"{}\" points to itself.", ExtensionName, config.paths[copyPath]);
                        invalidPaths.insert(copyPath);
                    }
                    continue;
                }

                if (depot->ResourceExists(copyPath))
                {
                    if (!invalidPaths.contains(copyPath))
                    {
                        LogError("[{}] Copy \"{}\" is an existing resource.", ExtensionName, config.paths[copyPath]);
                        invalidPaths.insert(copyPath);
                    }
                    continue;
                }

                finalCopies[copyPath] = targetPath;
                knownPaths[copyPath] = config.paths[copyPath];
            }

            knownPaths[targetPath] = config.paths[targetPath];
        }
    }

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

                if (depot->ResourceExists(linkPath) || finalCopies.contains(linkPath))
                {
                    if (!invalidPaths.contains(linkPath))
                    {
                        LogError("[{}] Link \"{}\" is an existing resource.", ExtensionName, config.paths[linkPath]);
                        invalidPaths.insert(linkPath);
                    }
                    continue;
                }

                // TODO: Warn about redefinitions?

                finalLinks[linkPath] = targetPath;
                knownPaths[linkPath] = config.paths[linkPath];
            }

            knownPaths[targetPath] = config.paths[targetPath];
        }
    }

    Core::Set<Red::ResourcePath> cyclicLinks;

    for (auto link = finalLinks.begin(); link != finalLinks.end();)
    {
        const auto& linkPath = link->first;
        auto targetPath = link->second;
        auto isCyclic = false;

        Core::Set<Red::ResourcePath> hops;
        hops.insert(linkPath);
        hops.insert(targetPath);

        while (true)
        {
            const auto& next = finalLinks.find(targetPath);

            if (next == finalLinks.end())
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
            LogError("[{}] Link \"{}\" is cyclic.", ExtensionName, knownPaths[linkPath]);
            invalidPaths.insert(linkPath);
        }

        finalLinks.erase(linkPath);
    }

    for (auto link = finalLinks.begin(); link != finalLinks.end();)
    {
        const auto& linkPath = link->first;
        const auto& targetPath = link->second;

        if (!depot->ResourceExists(targetPath) && !finalCopies.contains(targetPath))
        {
            if (!invalidPaths.contains(targetPath))
            {
                LogError(R"([{}] Link "{}" points to a non-existent resource "{}".)",
                         ExtensionName, knownPaths[linkPath], knownPaths[targetPath]);
                invalidPaths.insert(targetPath);
            }

            link = finalLinks.erase(link);
            continue;
        }

        ++link;
    }

    auto resourcePathRegistry = Core::Resolve<ResourcePathRegistry>();
    for (const auto& [knownPath, knownPathStr] : knownPaths)
    {
        resourcePathRegistry->RegisterPath(knownPath, knownPathStr);
    }

    s_copies = std::move(finalCopies);
    s_links = std::move(finalLinks);

    s_mappings.insert(s_copies.begin(), s_copies.end());
    s_mappings.insert(s_links.begin(), s_links.end());

    m_configs.clear();
}

void App::ResourceLinkExtension::OnLoaderResourceRequest(Red::ResourceLoader* aLoader,
                                                         Red::SharedPtr<Red::ResourceToken<>>& aToken,
                                                         Red::ResourceRequest& aRequest)
{
    const auto& link = s_links.find(aRequest.path);
    if (link != s_links.end())
    {
        aRequest.path = link->second;
    }
}

uintptr_t* App::ResourceLinkExtension::OnDepotResourceRequest(Red::ResourceDepot* aDepot,
                                                              const uintptr_t* aResourceHandle,
                                                              Red::ResourcePath aPath,
                                                              const int32_t* aArchiveHandle)
{
    const auto& link = s_copies.find(aPath);
    if (link != s_copies.end())
    {
        aPath = link->second;
    }

    return Raw::ResourceDepot::RequestResource(aDepot, aResourceHandle, aPath, aArchiveHandle);
}

bool App::ResourceLinkExtension::OnDepotResourceCheck(Red::ResourceDepot* aDepot, Red::ResourcePath aPath)
{
    const auto& link = s_mappings.find(aPath);
    if (link != s_mappings.end())
    {
        aPath = link->second;
    }

    return Raw::ResourceDepot::CheckResource(aDepot, aPath);
}
