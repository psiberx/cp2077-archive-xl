#include "Unit.hpp"
#include "Language.hpp"

#include <fmt/format.h>

namespace
{
constexpr auto LocalizationNodeKey = "localization";
constexpr auto OnScreensNodeKey = "onscreens";
}

bool App::LocalizationUnit::IsDefined()
{
    return !fallback.IsNone();
}

void App::LocalizationUnit::LoadYAML(const YAML::Node& aNode)
{
    const auto& localizationNode = aNode[LocalizationNodeKey];

    if (!localizationNode.IsDefined())
        return;

    bool malformed = false;

    const auto& onscreensNode = localizationNode[OnScreensNodeKey];

    if (onscreensNode.IsDefined())
    {
        switch (onscreensNode.Type())
        {
        case YAML::NodeType::Map:
        {
            for (const auto& it : onscreensNode)
            {
                const auto& key = it.first.Scalar();
                const auto language = RED4ext::CName(key.c_str());

                if (!Language::IsKnown(language))
                {
                    issues.emplace_back(fmt::format("Unknown language code [{}].", key));
                    continue;
                }

                const auto& data = it.second;
                auto paths = Core::Vector<std::string>();

                if (data.IsSequence())
                {
                    for (const auto& item : data)
                    {
                        if (item.IsScalar())
                            paths.emplace_back(item.Scalar());
                        else
                            malformed = true;
                    }
                }
                else if (data.IsScalar())
                {
                    paths.emplace_back(data.Scalar());
                }
                else
                {
                    malformed = true;
                }

                if (!paths.empty())
                {
                    onscreens.insert({ language, paths });

                    if (fallback.IsNone())
                        fallback = language;
                }
            }
            break;
        }
        case YAML::NodeType::Sequence:
        {
            const auto language = Language::English;
            auto paths = Core::Vector<std::string>();

            for (const auto& itemNode : onscreensNode)
            {
                if (itemNode.IsScalar())
                    paths.emplace_back(itemNode.Scalar());
                else
                    malformed = true;
            }

            if (!paths.empty())
            {
                onscreens.insert({ language, paths });
                fallback = language;
            }
            break;
        }
        case YAML::NodeType::Scalar:
        {
            const auto language = Language::English;

            onscreens.insert({ language, { onscreensNode.Scalar() } });
            fallback = language;
            break;
        }
        default:
        {
            malformed = true;
        }
        }
    }

    if (malformed)
        issues.emplace_back("Bad format. Expected resource path or list of paths.");
}
