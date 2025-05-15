#include "Config.hpp"

bool App::ResourcePatchConfig::IsDefined()
{
    return !patches.empty();
}

void App::ResourcePatchConfig::LoadYAML(const YAML::Node& aNode)
{
    const auto& rootNode = aNode["resource"];

    if (!rootNode.IsDefined() || !rootNode.IsMap())
        return;

    const auto& patchNode = rootNode["patch"];

    if (!patchNode.IsDefined() || !patchNode.IsMap())
        return;

    for (const auto& entryNode : patchNode)
    {
        const auto& patchPathStr = entryNode.first.Scalar();
        const auto& patchPath = Red::ResourcePath(patchPathStr.data());
        const auto& definitionNode = entryNode.second;

        ResourcePatch patchConfig;

        if (definitionNode.IsScalar())
        {
            const auto& targetPathStr = definitionNode.Scalar();
            const auto& targetPath = Red::ResourcePath(targetPathStr.data());

            if (targetPath)
            {
                patchConfig.includes.insert(targetPath);
                paths[targetPath] = targetPathStr;
            }
        }
        else if (definitionNode.IsSequence())
        {
            for (const auto& targetNode : definitionNode)
            {
                const auto& targetPathStr = targetNode.Scalar();
                const auto& targetPath = Red::ResourcePath(targetPathStr.data());

                if (targetPath)
                {
                    if (definitionNode.Tag() != "!exclude")
                    {
                        patchConfig.includes.insert(targetPath);
                    }
                    else
                    {
                        patchConfig.excludes.insert(targetPath);
                    }

                    paths[targetPath] = targetPathStr;
                }
            }
        }
        else if (definitionNode.IsMap())
        {
            const auto& propsNode = definitionNode["props"];
            const auto& targetsNode = definitionNode["targets"];

            for (const auto& propNode : propsNode)
            {
                const auto& propNameStr = propNode.Scalar();
                const auto& propName = Red::CName(propNameStr.data());

                if (propName)
                {
                    patchConfig.props.insert(propName);
                }
            }

            for (const auto& targetNode : targetsNode)
            {
                const auto& targetPathStr = targetNode.Scalar();
                const auto& targetPath = Red::ResourcePath(targetPathStr.data());

                if (targetPath)
                {
                    if (definitionNode.Tag() != "!exclude")
                    {
                        patchConfig.includes.insert(targetPath);
                    }
                    else
                    {
                        patchConfig.excludes.insert(targetPath);
                    }

                    paths[targetPath] = targetPathStr;
                }
            }
        }

        if (!patchConfig.includes.empty())
        {
            patches[patchPath] = std::move(patchConfig);
            paths[patchPath] = patchPathStr;
        }
    }
}
