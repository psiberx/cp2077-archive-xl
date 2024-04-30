#include "Unit.hpp"

bool App::ResourcePatchUnit::IsDefined()
{
    return !patches.empty();
}

void App::ResourcePatchUnit::LoadYAML(const YAML::Node& aNode)
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

        paths[patchPath] = patchPathStr;

        if (targetsNode.IsScalar())
        {
            const auto& targetPathStr = targetsNode.Scalar();
            const auto& targetPath = Red::ResourcePath(targetPathStr.data());

            if (targetPath)
            {
                patches[targetPath].emplace_back(patchPath);
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
                    patches[targetPath].emplace_back(patchPath);
                    paths[targetPath] = targetPathStr;
                }
            }
        }
    }
}
