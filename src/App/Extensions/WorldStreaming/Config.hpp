#pragma once

#include "App/Extensions/ExtensionBase.hpp"

namespace App
{
struct WorldNodeMutation
{
    int64_t nodeIndex;
    Red::CName nodeType;
    Red::Vector4 position;
    Red::Quaternion orientation;
    Red::Vector3 scale;
};

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
    Core::Vector<WorldNodeMutation> nodeMutations;
    Core::Vector<WorldNodeDeletion> nodeDeletions;
};

struct WorldStreamingConfig : ExtensionConfig
{
    using ExtensionConfig::ExtensionConfig;

    bool IsDefined() override;
    void LoadYAML(const YAML::Node& aNode) override;
    bool ParseSubDeletions(const YAML::Node& aNode, WorldNodeDeletion& aDeletionData);

    Core::Vector<std::string> blocks;
    Core::Vector<WorldSectorMod> sectors;
};
}
