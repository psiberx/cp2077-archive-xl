#pragma once

#include "App/Extensions/ExtensionBase.hpp"

namespace App
{
struct ResourcePatch
{
    Red::ResourcePath source;
    Core::Set<Red::CName> props;
    Core::Set<Red::ResourcePath> includes;
    Core::Set<Red::ResourcePath> excludes;
    int32_t order;
};

struct ResourcePatchConfig : ExtensionConfig
{
    using ExtensionConfig::ExtensionConfig;

    bool IsDefined() override;
    void LoadYAML(const YAML::Node& aNode) override;

    Core::Vector<ResourcePatch> patches;
    Core::Map<Red::ResourcePath, std::string> paths;
};
}
