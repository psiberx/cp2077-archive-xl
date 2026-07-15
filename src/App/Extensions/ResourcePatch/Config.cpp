#include "Config.hpp"
#include "App/Utils/Num.hpp"

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

    if (!patchNode.IsDefined())
        return;

    if (!patchNode.IsMap() && !patchNode.IsSequence())
    {
        AddIssue("Line #{}: Patch config must be a map or list.", patchNode.Mark().line);
        return;
    }

    const auto isSequence = patchNode.IsSequence();

    for (const auto& entryNode : patchNode)
    {
        ResourcePatch patchConfig;

        if (isSequence)
        {
            const auto& sourceNode = entryNode["source"];

            if (!sourceNode.IsDefined())
            {
                AddIssue("Line #{}: Patch definition must have a source path.", sourceNode.Mark().line);
                continue;
            }

            if (!sourceNode.IsScalar() || sourceNode.Scalar().empty())
            {
                AddIssue("Line #{}: Patch source path must be a non-empty string.", sourceNode.Mark().line);
                continue;
            }

            patchConfig.source = sourceNode.Scalar().c_str();
            paths[patchConfig.source] = sourceNode.Scalar();
        }
        else
        {
            if (entryNode.first.Scalar().empty())
            {
                AddIssue("Line #{}: Patch source path must be a non-empty string.", entryNode.Mark().line);
                continue;
            }

            patchConfig.source = entryNode.first.Scalar().c_str();
            paths[patchConfig.source] = entryNode.first.Scalar();
        }

        const auto& definitionNode = isSequence ? entryNode : entryNode.second;

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
            const auto& orderNode = definitionNode["order"];

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

            if (orderNode.IsDefined() && orderNode.IsScalar())
            {
                ParseInt(orderNode.Scalar(), patchConfig.order);
            }
        }
        else
        {
            AddIssue("Line #{}: Patch definition must be a map, list or path.", definitionNode.Mark().line);
            continue;
        }

        if (patchConfig.includes.empty())
        {
            AddIssue("Line #{}: Patch definition must have at least one target.", definitionNode.Mark().line);
            continue;
        }

        patches.emplace_back(std::move(patchConfig));
    }
}
