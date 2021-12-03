#pragma once

#include "Definition.hpp"
#include "LanguageCode.hpp"
#include "Reverse/ResourcePath.hpp"

namespace AXL
{
struct LocalizationDefinition : public Definition
{
    using ResourceList = std::vector<ResourcePath>;
    using LanguageResourceMap = std::unordered_map<LanguageHash,ResourceList>;

    LocalizationDefinition(const std::string& aName)
        : Definition(aName) {}

    bool IsDefined() override;

    static LocalizationDefinition FromConfig(const std::string& aName, const YAML::Node& aNode);

    ResourcePath fallback;
    LanguageResourceMap onscreens;
    LanguageResourceMap subtitles;
};
}
