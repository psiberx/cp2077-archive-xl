#include "Config.hpp"
#include "App/Utils/Num.hpp"

bool App::WorldStreamingConfig::IsDefined()
{
    return !blocks.empty() || !sectors.empty();
}

void App::WorldStreamingConfig::LoadYAML(const YAML::Node& aNode)
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
            for (size_t sectorIndex = 0, sectorCount = sectorsNode.size(); sectorIndex < sectorCount; ++sectorIndex)
            {
                const auto& sectorNode = sectorsNode[sectorIndex];

                if (!sectorNode.IsMap())
                {
                    issues.push_back(std::format("Invalid sector definition at index #{}.", sectorIndex));
                    continue;
                }

                WorldSectorMod sectorData{};
                sectorData.mod = name;

                {
                    const auto& pathNode = sectorNode["path"];

                    if (!pathNode.IsDefined())
                    {
                        issues.push_back(std::format("Sector definition at index #{} is missing path.", sectorIndex));
                        continue;
                    }

                    if (!pathNode.IsScalar())
                    {
                        issues.push_back(std::format("Sector definition at index #{} has invalid path.", sectorIndex));
                        continue;
                    }

                    sectorData.path = pathNode.Scalar();

                    if (sectorData.path.empty())
                    {
                        issues.push_back(std::format("Sector definition at index #{} has invalid path.", sectorIndex));
                        continue;
                    }
                }

                {
                    const auto& countNode = sectorNode["expectedNodes"];

                    if (!countNode.IsDefined())
                    {
                        issues.push_back(std::format("Sector definition for \"{}\" is missing expected nodes count.",
                                                     sectorData.path));
                        continue;
                    }

                    if (!countNode.IsScalar())
                    {
                        issues.push_back(std::format("Sector definition for \"{}\" has invalid expected nodes count.",
                                                     sectorData.path));
                        continue;
                    }

                    if (!ParseInt(countNode.Scalar(), sectorData.expectedNodes))
                    {
                        issues.push_back(std::format("Sector definition for \"{}\" has invalid expected nodes count.",
                                                     sectorData.path));
                        continue;
                    }

                    if (sectorData.expectedNodes <= 0)
                    {
                        issues.push_back(std::format("Sector definition for \"{}\" has invalid expected nodes count.",
                                                     sectorData.path));
                        continue;
                    }
                }

                // TODO: More errors

                {
                    const auto& deletionsNode = sectorNode["nodeDeletions"];

                    if (deletionsNode.IsDefined() && deletionsNode.IsSequence())
                    {
                        for (const auto& deletionNode : deletionsNode)
                        {
                            if (!deletionNode.IsMap())
                            {
                                continue;
                            }

                            const auto& typeNode = deletionNode["type"];

                            if (!typeNode.IsDefined() || !typeNode.IsScalar())
                            {
                                continue;
                            }

                            const auto& indexNode = deletionNode["index"];

                            if (!indexNode.IsDefined() || !indexNode.IsScalar())
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

                            ParseSubDeletions(deletionNode["actorDeletions"], deletionNode["expectedActors"],
                                              deletionData.subNodeDeletions, deletionData.expectedSubNodes);
                            ParseSubDeletions(deletionNode["instanceDeletions"], deletionNode["expectedInstances"],
                                              deletionData.subNodeDeletions, deletionData.expectedSubNodes);

                            sectorData.nodeDeletions.emplace_back(std::move(deletionData));
                        }
                    }
                }

                {
                    const auto& mutationsNode = sectorNode["nodeMutations"];

                    if (mutationsNode.IsDefined() && mutationsNode.IsSequence())
                    {
                        for (const auto& mutationNode : mutationsNode)
                        {
                            if (!mutationNode.IsMap())
                            {
                                continue;
                            }

                            WorldNodeMutation mutationData{};

                            {
                                const auto& typeNode = mutationNode["type"];

                                if (!typeNode.IsDefined() || !typeNode.IsScalar())
                                {
                                    continue;
                                }

                                mutationData.nodeType = typeNode.Scalar().data();
                            }

                            {
                                const auto& indexNode = mutationNode["index"];

                                if (!indexNode.IsDefined() || !indexNode.IsScalar())
                                {
                                    continue;
                                }

                                if (!ParseInt(indexNode.Scalar(), mutationData.nodeIndex))
                                {
                                    continue;
                                }

                                if (mutationData.nodeIndex < 0 || mutationData.nodeIndex >= sectorData.expectedNodes)
                                {
                                    continue;
                                }
                            }

                            {
                                const auto& positionNode = mutationNode["position"];

                                if (positionNode.IsDefined())
                                {
                                    if (!positionNode.IsSequence())
                                    {
                                        continue;
                                    }

                                    const auto positionValues = positionNode.as<std::vector<float>>();

                                    if (positionValues.size() != 4)
                                    {
                                        continue;
                                    }

                                    mutationData.position.X = positionValues[0];
                                    mutationData.position.Y = positionValues[1];
                                    mutationData.position.Z = positionValues[2];
                                    mutationData.position.W = positionValues[3];

                                    mutationData.modifyPosition = true;
                                }
                            }

                            {
                                const auto& orientationNode = mutationNode["orientation"];

                                if (orientationNode.IsDefined())
                                {
                                    if (!orientationNode.IsSequence())
                                    {
                                        continue;
                                    }

                                    const auto orientationValues = orientationNode.as<std::vector<float>>();

                                    if (orientationValues.size() != 4)
                                    {
                                        continue;
                                    }

                                    mutationData.orientation.i = orientationValues[0];
                                    mutationData.orientation.j = orientationValues[1];
                                    mutationData.orientation.k = orientationValues[2];
                                    mutationData.orientation.r = orientationValues[3];

                                    mutationData.modifyOrientation = true;
                                }
                            }

                            {
                                const auto& scaleNode = mutationNode["scale"];

                                if (scaleNode.IsDefined())
                                {
                                    if (!scaleNode.IsSequence())
                                    {
                                        continue;
                                    }

                                    const auto scaleValues = scaleNode.as<std::vector<float>>();

                                    if (scaleValues.size() != 3)
                                    {
                                        continue;
                                    }

                                    mutationData.scale.X = scaleValues[0];
                                    mutationData.scale.Y = scaleValues[1];
                                    mutationData.scale.Z = scaleValues[2];

                                    mutationData.modifyScale = true;
                                }
                            }

                            {
                                const auto& propertyNode = mutationNode["nbNodesUnderProxyDiff"];

                                if (propertyNode.IsDefined())
                                {
                                    if (!propertyNode.IsScalar())
                                    {
                                        continue;
                                    }

                                    if (!ParseInt(propertyNode.Scalar(), mutationData.nbNodesUnderProxyDiff))
                                    {
                                        continue;
                                    }

                                    mutationData.modifyProxyNodes = true;
                                }
                            }

                            ParseResource(mutationNode["resource"], mutationData.resourcePath, mutationData.modifyResource);
                            ParseResource(mutationNode["mesh"], mutationData.resourcePath, mutationData.modifyResource);
                            ParseResource(mutationNode["meshRef"], mutationData.resourcePath, mutationData.modifyResource);
                            ParseResource(mutationNode["material"], mutationData.resourcePath, mutationData.modifyResource);
                            ParseResource(mutationNode["effect"], mutationData.resourcePath, mutationData.modifyResource);
                            ParseResource(mutationNode["entityTemplate"], mutationData.resourcePath, mutationData.modifyResource);

                            ParseName(mutationNode["appearance"], mutationData.appearanceName, mutationData.modifyAppearance);
                            ParseName(mutationNode["appearanceName"], mutationData.appearanceName, mutationData.modifyAppearance);
                            ParseName(mutationNode["meshAppearance"], mutationData.appearanceName, mutationData.modifyAppearance);

                            ParseRecordID(mutationNode["recordID"], mutationData.recordID, mutationData.modifyRecordID);
                            ParseRecordID(mutationNode["recordId"], mutationData.recordID, mutationData.modifyRecordID);
                            ParseRecordID(mutationNode["objectRecordId"], mutationData.recordID, mutationData.modifyRecordID);

                            sectorData.nodeMutations.emplace_back(std::move(mutationData));
                        }
                    }
                }

                if (sectorData.nodeDeletions.empty() && sectorData.nodeMutations.empty())
                {
                    continue;
                }

                sectors.emplace_back(std::move(sectorData));
            }
        }
    }
}

bool App::WorldStreamingConfig::ParseResource(const YAML::Node& aNode, Red::ResourcePath& aValue, bool& aFlag)
{
    if (!aNode.IsDefined() || !aNode.IsScalar())
    {
        return false;
    }

    aValue = aNode.Scalar().data();
    aFlag = true;

    return true;
}

bool App::WorldStreamingConfig::ParseName(const YAML::Node& aNode, Red::CName& aValue, bool& aFlag)
{
    if (!aNode.IsDefined() || !aNode.IsScalar())
    {
        return false;
    }

    aValue = aNode.Scalar().data();
    aFlag = true;

    return true;
}

bool App::WorldStreamingConfig::ParseRecordID(const YAML::Node& aNode, Red::TweakDBID& aValue, bool& aFlag)
{
    if (!aNode.IsDefined() || !aNode.IsScalar())
    {
        return false;
    }

    aValue = Red::TweakDBID{aNode.Scalar().data(), aNode.Scalar().size()};
    aFlag = true;

    return true;
}

bool App::WorldStreamingConfig::ParseSubDeletions(const YAML::Node& aDeletionsNode, const YAML::Node& aCountNode,
                                                  Core::Vector<int64_t>& aDeletions, int64_t& aExpectedCount)
{
    if (!aDeletionsNode.IsDefined() || !aDeletionsNode.IsSequence())
    {
        return false;
    }

    if (!aCountNode.IsDefined() || !aCountNode.IsScalar())
    {
        return false;
    }

    if (!ParseInt(aCountNode.Scalar(), aExpectedCount))
    {
        return false;
    }

    if (aExpectedCount <= 0)
    {
        return false;
    }

    for (const auto& subDeletionNode : aDeletionsNode)
    {
        if (!subDeletionNode.IsScalar())
        {
            aDeletions.clear();
            return false;
        }

        int64_t subIndex;
        if (!ParseInt(subDeletionNode.Scalar(), subIndex))
        {
            aDeletions.clear();
            return false;
        }

        if (subIndex < 0 || subIndex >= aExpectedCount)
        {
            aDeletions.clear();
            return false;
        }

        aDeletions.push_back(subIndex);
    }

    return true;
}
