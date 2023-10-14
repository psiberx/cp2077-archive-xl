#include "Unit.hpp"
#include "App/Utils/Num.hpp"

bool App::StreamingUnit::IsDefined()
{
    return !blocks.empty() || !sectors.empty();
}

void App::StreamingUnit::LoadYAML(const YAML::Node& aNode)
{
    const auto& streamingNode = aNode["streaming"];

    if (!streamingNode.IsDefined())
        return;

    {
        const auto& blocksNode = streamingNode["blocks"];
        if (blocksNode.IsDefined() && blocksNode.IsSequence())
        {
            for (const auto& pathNode : blocksNode)
            {
                if (pathNode.IsScalar())
                {
                    blocks.emplace_back(pathNode.Scalar());
                }
            }
        }
    }

    {
        const auto& sectorsNode = streamingNode["sectors"];
        if (sectorsNode.IsDefined() && sectorsNode.IsSequence())
        {
            for (const auto& sectorNode : sectorsNode)
            {
                if (!sectorNode.IsMap())
                {
                    continue;
                }

                const auto& pathNode = sectorNode["path"];
                const auto& deletionsNode = sectorNode["deletions"];
                const auto& validationNode = sectorNode["validation"];

                if (!pathNode.IsDefined() || !pathNode.IsScalar()
                    || !deletionsNode.IsDefined() || !deletionsNode.IsSequence()
                    || !validationNode.IsDefined()|| !validationNode.IsMap())
                {
                    continue;
                }

                StreamingSectorMod sectorData{};
                sectorData.mod = name;
                sectorData.path = pathNode.Scalar();

                {
                    const auto& countNode = validationNode["totalNodes"];

                    if (!countNode.IsDefined() || !countNode.IsScalar())
                    {
                        continue;
                    }

                    if (!ParseInt(countNode.Scalar(), sectorData.expectedNodes))
                    {
                        continue;
                    }
                }

                for (const auto& deletionNode : deletionsNode)
                {
                    const auto& typeNode = deletionNode["type"];
                    const auto& indexNode = deletionNode["index"];

                    if (!typeNode.IsDefined() || !typeNode.IsScalar() || !indexNode.IsDefined() || !indexNode.IsScalar())
                    {
                        continue;
                    }

                    WorldNodeDeletion deletionData{};
                    deletionData.nodeType = typeNode.Scalar().data();

                    if (!ParseInt(indexNode.Scalar(), deletionData.nodeIndex))
                    {
                        continue;
                    }

                    if (deletionData.nodeIndex < 0 || deletionData.nodeIndex >= sectorData.expectedNodes)
                    {
                        continue;
                    }

                    sectorData.deletions.emplace_back(std::move(deletionData));
                }

                if (sectorData.path.empty() || !sectorData.expectedNodes || sectorData.deletions.empty())
                {
                    continue;
                }

                sectors.emplace_back(std::move(sectorData));
            }
        }
    }
}
