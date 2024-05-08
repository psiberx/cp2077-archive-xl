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
        const auto& targetsNode = entryNode.second;

        ResourcePatchScope patchScope;

        if (targetsNode.IsScalar())
        {
            const auto& targetPathStr = targetsNode.Scalar();
            const auto& targetPath = Red::ResourcePath(targetPathStr.data());

            if (targetPath)
            {
                patchScope.includes.insert(targetPath);
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
                    if (targetsNode.Tag() != "!exclude")
                    {
                        patchScope.includes.insert(targetPath);
                    }
                    else
                    {
                        patchScope.excludes.insert(targetPath);
                    }

                    paths[targetPath] = targetPathStr;
                }
            }
        }

        if (!patchScope.includes.empty())
        {
            patches[patchPath] = std::move(patchScope);
            paths[patchPath] = patchPathStr;
        }
    }
}
