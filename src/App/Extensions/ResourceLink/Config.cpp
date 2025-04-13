#include "Config.hpp"

bool App::ResourceLinkConfig::IsDefined()
{
    return !links.empty() || !copies.empty();
}

void App::ResourceLinkConfig::LoadYAML(const YAML::Node& aNode)
{
    const auto& rootNode = aNode["resource"];

    if (!rootNode.IsDefined() || !rootNode.IsMap())
        return;

    const auto& linksNode = rootNode["link"];

    if (linksNode.IsDefined() && linksNode.IsMap())
    {
        for (const auto& linkNode : linksNode)
        {
            const auto& targetPathStr = linkNode.first.Scalar();
            const auto& targetPath = Red::ResourcePath(targetPathStr.data());
            const auto& sourcesNode = linkNode.second;

            if (sourcesNode.IsScalar())
            {
                const auto& linkPathStr = sourcesNode.Scalar();
                const auto& sourcePath = Red::ResourcePath(linkPathStr.data());

                if (sourcePath)
                {
                    links[sourcePath].insert(targetPath);
                    paths[sourcePath] = linkPathStr;
                }
            }
            else if (sourcesNode.IsSequence())
            {
                for (const auto& sourceNode : sourcesNode)
                {
                    const auto& linkPathStr = sourceNode.Scalar();
                    const auto& sourcePath = Red::ResourcePath(linkPathStr.data());

                    if (sourcePath)
                    {
                        links[targetPath].insert(sourcePath);
                        paths[sourcePath] = linkPathStr;
                    }
                }
            }

            paths[targetPath] = targetPathStr;
        }
    }

    const auto& copiesNode = rootNode["copy"];

    if (copiesNode.IsDefined() && copiesNode.IsMap())
    {
        for (const auto& copyNode : copiesNode)
        {
            const auto& sourcePathStr = copyNode.first.Scalar();
            const auto& sourcePath = Red::ResourcePath(sourcePathStr.data());
            const auto& targetsNode = copyNode.second;

            if (targetsNode.IsScalar())
            {
                const auto& targetPathStr = targetsNode.Scalar();
                const auto& targetPath = Red::ResourcePath(targetPathStr.data());

                if (targetPath)
                {
                    copies[sourcePath].insert(targetPath);
                    paths[targetPath] = targetPathStr;
                }
            }
            else if (targetsNode.IsSequence())
            {
                for (const auto& targetNode : targetsNode)
                {
                    const auto& targetPathStr = targetNode.Scalar();
                    const auto& targetPath = Red::ResourcePath(targetPathStr.data());

                    if (targetPath)
                    {
                        copies[sourcePath].insert(targetPath);
                        paths[targetPath] = targetPathStr;
                    }
                }
            }

            paths[sourcePath] = sourcePathStr;
        }
    }
}
