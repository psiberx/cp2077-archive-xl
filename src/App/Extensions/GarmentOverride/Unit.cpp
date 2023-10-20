#include "Unit.hpp"
#include "App/Utils/Num.hpp"

bool App::GarmentOverrideUnit::IsDefined()
{
    return !tags.empty();
}

void App::GarmentOverrideUnit::LoadYAML(const YAML::Node& aNode)
{
    const auto& overridesNode = aNode["overrides"];

    if (!overridesNode.IsDefined())
        return;

    const auto& tagsNode = overridesNode["tags"];

    if (!tagsNode.IsDefined())
        return;

    if (!tagsNode.IsMap())
    {
        issues.emplace_back("Bad format. Expected list of tags.");
        return;
    }

    bool malformed = false;

    for (const auto& tagNode : tagsNode)
    {
        const auto& tagName = tagNode.first.Scalar();
        const auto& componentsNode = tagNode.second;

        if (!componentsNode.IsMap())
        {
            malformed = true;
            continue;
        }

        OverrideTagDefinition tagDefinition;

        for (const auto& componentNode : componentsNode)
        {
            const auto& componentName = componentNode.first.Scalar();
            const auto& chunksNode = componentNode.second;

            switch (chunksNode.Type())
            {
                case YAML::NodeType::Map:
                {
                    bool success = false;
                    for (const auto& op : {std::pair{"hide", false}, {"show", true}})
                    {
                        const auto& opNode = chunksNode[op.first];
                        if (opNode.IsDefined())
                        {
                            if (opNode.IsSequence())
                            {
                                const auto mask = opNode.as<std::vector<uint8_t>>();
                                tagDefinition.emplace(componentName.data(), ChunkMask(op.second, mask));
                                success = true;
                            }
                            else
                            {
                                malformed = true;
                            }
                            break;
                        }
                    }
                    if (!success)
                    {
                        malformed = true;
                    }
                    break;
                }
                case YAML::NodeType::Sequence:
                {
                    const auto mask = chunksNode.as<std::vector<uint8_t>>();
                    tagDefinition.emplace(componentName.data(), mask);
                    break;
                }
                case YAML::NodeType::Scalar:
                {
                    uint64_t mask;
                    if (ParseInt(chunksNode.Scalar(), mask))
                    {
                        tagDefinition.emplace(componentName.data(), mask);
                    }
                    else
                    {
                        malformed = true;
                    }
                    break;
                }
                default:
                {
                    malformed = true;
                }
            }
        }

        if (!tagDefinition.empty())
        {
            tags.emplace(tagName, std::move(tagDefinition));
        }
    }

    if (malformed)
    {
        issues.emplace_back("Bad format. Expected list of tags.");
    }
}
