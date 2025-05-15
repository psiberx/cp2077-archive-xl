#pragma once

#include "App/Extensions/ExtensionBase.hpp"

namespace App
{
struct ResourcePatch
{
    [[nodiscard]] bool Modifies(Red::CName aProp) const
    {
        return props.empty() || props.contains(aProp);
    }

    [[nodiscard]] bool Modifies(Red::CName aProp, bool aOverwrite) const
    {
        return (!aOverwrite && props.empty()) || props.contains(aProp);
    }

    Core::Set<Red::CName> props;
    Core::Set<Red::ResourcePath> includes;
    Core::Set<Red::ResourcePath> excludes;
};

struct ResourcePatchConfig : ExtensionConfig
{
    using ExtensionConfig::ExtensionConfig;

    bool IsDefined() override;
    void LoadYAML(const YAML::Node& aNode) override;

    Core::Map<Red::ResourcePath, ResourcePatch> patches;
    Core::Map<Red::ResourcePath, std::string> paths;
};
}
