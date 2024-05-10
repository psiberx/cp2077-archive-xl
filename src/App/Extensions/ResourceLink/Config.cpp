#include "Config.hpp"

bool App::ResourceLinkConfig::IsDefined()
{
    return !links.empty();
}

void App::ResourceLinkConfig::LoadYAML(const YAML::Node& aNode)
{
    const auto& rootNode = aNode["resource"];

    if (!rootNode.IsDefined() || !rootNode.IsMap())
        return;

    const auto& entriesNode = rootNode["link"];

    if (!entriesNode.IsDefined() || !entriesNode.IsMap())
        return;

    for (const auto& entryNode : entriesNode)
    {
        const auto& targetPathStr = entryNode.first.Scalar();
        const auto& targetPath = Red::ResourcePath(targetPathStr.data());
        const auto& linksNode = entryNode.second;

        if (linksNode.IsScalar())
        {
            const auto& linkPathStr = linksNode.Scalar();
            const auto& linkPath = Red::ResourcePath(linkPathStr.data());

            if (linkPath)
            {
                links[linkPath].insert(targetPath);
                paths[linkPath] = linkPathStr;
            }
        }
        else if (linksNode.IsSequence())
        {
            for (const auto& linkNode : linksNode)
            {
                const auto& linkPathStr = linkNode.Scalar();
                const auto& linkPath = Red::ResourcePath(linkPathStr.data());

                if (linkPath)
                {
                    links[targetPath].insert(linkPath);
                    paths[linkPath] = linkPathStr;
                }
            }
        }

        paths[targetPath] = targetPathStr;
    }
}
