#pragma once

#include "App/Extensions/ModuleBase.hpp"

namespace App
{
struct LocalizationUnit : ConfigurableUnit
{
    bool IsDefined() override;
    void LoadYAML(const YAML::Node& aNode) override;

    static bool ReadOptions(const YAML::Node& aNode, Core::Map<Red::CName, Core::Vector<std::string>>& aOptions,
                            Red::CName& aFallback, Core::Vector<std::string>& aIssues);

    Red::CName fallback;
    Core::Map<Red::CName, Core::Vector<std::string>> onscreens;
    Core::Map<Red::CName, Core::Vector<std::string>> subtitles;
    Core::Map<Red::CName, Core::Vector<std::string>> lipmaps;
    Core::Map<Red::CName, Core::Vector<std::string>> vomaps;
    std::string extend;
};
}
