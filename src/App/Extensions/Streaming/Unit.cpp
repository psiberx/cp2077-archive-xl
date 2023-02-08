#include "Unit.hpp"

namespace
{
constexpr auto StreamingNodeKey = "streaming";
constexpr auto BlockNodeKey = "blocks";
}

bool App::StreamingUnit::IsDefined()
{
    return !blocks.empty();
}

void App::StreamingUnit::LoadYAML(const YAML::Node& aNode)
{
    const auto& streamingNode = aNode[StreamingNodeKey];

    if (!streamingNode.IsDefined())
        return;

    const auto& blockNode = streamingNode[BlockNodeKey];

    if (!blockNode.IsDefined() || !blockNode.IsSequence())
        return;

    for (const auto& pathNode : blockNode)
    {
        if (pathNode.IsScalar())
        {
            blocks.emplace_back(pathNode.Scalar());
        }
    }
}
