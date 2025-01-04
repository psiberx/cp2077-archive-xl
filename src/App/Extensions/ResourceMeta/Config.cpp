#include "Config.hpp"

bool App::ResourceMetaConfig::IsDefined()
{
    return !scopes.empty() || !fixes.empty();
}

void App::ResourceMetaConfig::LoadYAML(const YAML::Node& aNode)
{
    const auto& rootNode = aNode["resource"];

    if (!rootNode.IsDefined() || !rootNode.IsMap())
        return;

    LoadScopes(rootNode["scope"]);
    LoadFixes(rootNode["fix"]);
}

void App::ResourceMetaConfig::LoadScopes(const YAML::Node& aNode)
{
    if (!aNode.IsDefined() || !aNode.IsMap())
        return;

    for (const auto& entryNode : aNode)
    {
        const auto& scopePathStr = entryNode.first.Scalar();
        const auto& scopePath = Red::ResourcePath(scopePathStr.data());
        const auto& targetsNode = entryNode.second;

        paths[scopePath] = scopePathStr;

        if (targetsNode.IsScalar())
        {
            const auto& targetPathStr = targetsNode.Scalar();
            const auto& targetPath = Red::ResourcePath(targetPathStr.data());

            if (targetPath)
            {
                scopes[scopePath].insert(targetPath);
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
                    scopes[scopePath].insert(targetPath);
                    paths[targetPath] = targetPathStr;
                }
            }
        }
    }
}

void App::ResourceMetaConfig::LoadFixes(const YAML::Node& aNode)
{
    if (!aNode.IsDefined() || !aNode.IsMap())
        return;

    for (const auto& entryNode : aNode)
    {
        const auto& definitionNode = entryNode.second;

        if (!definitionNode.IsMap())
            continue;

        const auto& namesNode = definitionNode["names"];
        const auto& pathsNode = definitionNode["paths"];
        const auto& contextNode = definitionNode["context"];

        if (namesNode.IsDefined() && !namesNode.IsMap())
            continue;

        if (pathsNode.IsDefined() && !pathsNode.IsMap())
            continue;

        if (contextNode.IsDefined() && !contextNode.IsMap())
            continue;

        const auto& targetPathStr = entryNode.first.Scalar();
        const auto& targetPath = Red::ResourcePath(targetPathStr.data());

        if (namesNode.IsDefined())
        {
            for (const auto& nameNode : namesNode)
            {
                auto oldName = Red::CNamePool::Add(nameNode.first.Scalar().data());
                auto newName = Red::CNamePool::Add(nameNode.second.Scalar().data());

                fixes[targetPath].names[oldName] = newName;
            }
        }

        if (pathsNode.IsDefined())
        {
            for (const auto& pathNode : pathsNode)
            {
                auto oldPath = Red::ResourcePath(pathNode.first.Scalar().data());
                auto newPath = Red::ResourcePath(pathNode.second.Scalar().data());

                fixes[targetPath].paths[oldPath] = newPath;

                paths[oldPath] = pathNode.first.Scalar();
                paths[newPath] = pathNode.second.Scalar();
            }
        }

        if (contextNode.IsDefined())
        {
            for (const auto& paramNode : contextNode)
            {
                auto paramName = Red::CNamePool::Add(paramNode.first.Scalar().data());
                const auto& paramValue = paramNode.second.Scalar();

                fixes[targetPath].context[paramName] = paramValue;
            }
        }

        paths[targetPath] = targetPathStr;
    }
}
