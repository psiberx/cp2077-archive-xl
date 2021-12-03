#include "LocalizationDef.hpp"

AXL::LocalizationDefinition AXL::LocalizationDefinition::FromConfig(const std::string& aName, const YAML::Node& aNode)
{
    LocalizationDefinition definition(aName);

    const auto& localizationNode = aNode["localization"];

    if (localizationNode.IsDefined())
    {
        const auto& onscreensNode = localizationNode["onscreens"];

        if (onscreensNode.IsDefined())
        {
            if (onscreensNode.Type() == YAML::NodeType::Scalar)
            {
                definition.fallback = onscreensNode.as<std::string>().c_str();
            }
            else if (onscreensNode.Type() == YAML::NodeType::Map)
            {
                for (const auto& languageNode : onscreensNode)
                {
                    LanguageCode languageCode = languageNode.first.as<std::string>().c_str();
                    ResourceList resourceList;

                    if (languageNode.second.Type() == YAML::NodeType::Scalar)
                    {
                        resourceList.emplace_back(languageNode.second.as<std::string>().c_str());
                    }
                    else if (languageNode.second.Type() == YAML::NodeType::Sequence)
                    {
                        for (const auto& resourceNode : languageNode.second)
                        {
                            if (resourceNode.Type() == YAML::NodeType::Scalar)
                                resourceList.emplace_back(resourceNode.as<std::string>().c_str());
                        }
                    }

                    if (!resourceList.empty())
                    {
                        definition.onscreens.insert(std::make_pair(languageCode, resourceList));

                        if (!definition.fallback)
                            definition.fallback = languageCode;
                    }
                }
            }
        }
    }

    return std::move(definition);
}

bool AXL::LocalizationDefinition::IsDefined()
{
    return fallback;
}
