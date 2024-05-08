#pragma once

#include "App/Extensions/ModuleBase.hpp"

namespace App
{
struct WorldNodeDeletion
{
    int64_t nodeIndex;
    Red::CName nodeType;
    int64_t expectedSubNodes;
    Core::Vector<int64_t> subNodeDeletions;
};

struct WorldSectorMod
{
    std::string mod;
    std::string path;
    int64_t expectedNodes;
    Core::Vector<WorldNodeDeletion> nodeDeletions;
};

struct WorldStreamingConfig : ModuleConfig
{
    using ModuleConfig::ModuleConfig;

    bool IsDefined() override;
    void LoadYAML(const YAML::Node& aNode) override;
    bool ParseSubDeletions(const YAML::Node& aNode, WorldNodeDeletion& aDeletionData);

    Core::Vector<std::string> blocks;
    Core::Vector<WorldSectorMod> sectors;
};
}
