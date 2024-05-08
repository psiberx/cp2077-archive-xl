#include "Config.hpp"
#include "Language.hpp"

namespace
{
constexpr auto LocalizationNodeKey = "localization";
constexpr auto OnScreensNodeKey = "onscreens";
constexpr auto SubtitlesNodeKey = "subtitles";
constexpr auto LipMapsNodeKey = "lipmaps";
constexpr auto VoiceOverMapsNodeKey = "vomaps";
}

bool App::LocalizationConfig::IsDefined()
{
    return !onscreens.empty() || !subtitles.empty() || !lipmaps.empty() || !vomaps.empty();
}

void App::LocalizationConfig::LoadYAML(const YAML::Node& aNode)
{
    const auto& rootNode = aNode[LocalizationNodeKey];

    if (!rootNode.IsDefined())
        return;

    bool malformed = false;

    if (!ReadOptions(rootNode[OnScreensNodeKey], onscreens, fallback, issues))
        malformed = true;

    if (!ReadOptions(rootNode[SubtitlesNodeKey], subtitles, fallback, issues))
        malformed = true;

    if (!ReadOptions(rootNode[LipMapsNodeKey], lipmaps, fallback, issues))
        malformed = true;

    if (!ReadOptions(rootNode[VoiceOverMapsNodeKey], vomaps, fallback, issues))
        malformed = true;

    const auto& extendNode = rootNode["extend"];
    if (extendNode.IsDefined() && extendNode.IsScalar())
    {
        extend = extendNode.Scalar();
    }

    if (malformed)
        issues.emplace_back("Bad format. Expected resource path or list of paths.");
}

bool App::LocalizationConfig::ReadOptions(const YAML::Node& aNode,
                                        Core::Map<Red::CName, Core::Vector<std::string>>& aOptions,
                                        Red::CName& aFallback, Core::Vector<std::string>& aIssues)
{
    if (!aNode.IsDefined())
        return true;

    bool malformed = false;

    switch (aNode.Type())
    {
    case YAML::NodeType::Map:
    {
        for (const auto& it : aNode)
        {
            const auto& key = it.first.Scalar();
            const auto language = Red::CName(key.c_str());

            if (!Language::IsKnown(language))
            {
                aIssues.emplace_back(std::format("Unknown language code \"{}\".", key));
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
                aOptions.insert({ language, paths });

                if (!aFallback)
                {
                    aFallback = language;
                }
            }
        }
        break;
    }
    case YAML::NodeType::Sequence:
    {
        const auto language = Language::English;
        auto paths = Core::Vector<std::string>();

        for (const auto& itemNode : aNode)
        {
            if (itemNode.IsScalar())
                paths.emplace_back(itemNode.Scalar());
            else
                malformed = true;
        }

        if (!paths.empty())
        {
            aOptions.insert({ language, paths });

            if (!aFallback)
            {
                aFallback = language;
            }
        }
        break;
    }
    case YAML::NodeType::Scalar:
    {
        const auto language = Language::English;

        aOptions.insert({language, {aNode.Scalar()}});

        if (!aFallback)
        {
            aFallback = language;
        }
        break;
    }
    default:
    {
        malformed = true;
    }
    }

    return !malformed;
}
