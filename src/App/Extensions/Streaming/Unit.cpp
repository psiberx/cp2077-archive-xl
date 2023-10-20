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
                const auto& countNode = sectorNode["expectedNodes"];
                const auto& deletionsNode = sectorNode["nodeDeletions"];

                if (!pathNode.IsDefined() || !pathNode.IsScalar()
                    || !deletionsNode.IsDefined() || !deletionsNode.IsSequence()
                    || !countNode.IsDefined() || !countNode.IsScalar())
                {
                    continue;
                }

                StreamingSectorMod sectorData{};
                sectorData.mod = name;
                sectorData.path = pathNode.Scalar();

                if (!ParseInt(countNode.Scalar(), sectorData.expectedNodes))
                {
                    continue;
                }

                if (sectorData.expectedNodes <= 0)
                {
                    continue;
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

                    ParseSubDeletions(deletionNode, deletionData);

                    sectorData.nodeDeletions.emplace_back(std::move(deletionData));
                }

                if (sectorData.path.empty() || !sectorData.expectedNodes || sectorData.nodeDeletions.empty())
                {
                    continue;
                }

                sectors.emplace_back(std::move(sectorData));
            }
        }
    }
}

bool App::StreamingUnit::ParseSubDeletions(const YAML::Node& aNode, WorldNodeDeletion& aDeletionData)
{
    const auto& subDeletionsNode = aNode["actorDeletions"];

    if (!subDeletionsNode.IsDefined() || !subDeletionsNode.IsSequence())
    {
        return false;
    }

    const auto& subCountNode = aNode["expectedActors"];

    if (!subCountNode.IsDefined() || !subCountNode.IsScalar())
    {
        return false;
    }

    if (!ParseInt(subCountNode.Scalar(), aDeletionData.expectedSubNodes))
    {
        return false;
    }

    if (aDeletionData.expectedSubNodes <= 0)
    {
        return false;
    }

    for (const auto& subDeletionNode : subDeletionsNode)
    {
        if (!subDeletionNode.IsScalar())
        {
            aDeletionData.subNodeDeletions.clear();
            return false;
        }

        int64_t subIndex;
        if (!ParseInt(subDeletionNode.Scalar(), subIndex))
        {
            aDeletionData.subNodeDeletions.clear();
            return false;
        }

        if (subIndex < 0 || subIndex >= aDeletionData.expectedSubNodes)
        {
            aDeletionData.subNodeDeletions.clear();
            return false;
        }

        aDeletionData.subNodeDeletions.push_back(subIndex);
    }

    return true;
}
