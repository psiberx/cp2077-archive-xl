#pragma once

#include "App/Extensions/ExtensionBase.hpp"

namespace App
{
struct WorldNodeMutation
{
    int64_t nodeIndex;
    Red::CName nodeType;

    bool modifyPosition;
    Red::Vector4 position;

    bool modifyOrientation;
    Red::Quaternion orientation;

    bool modifyScale;
    Red::Vector3 scale;

    bool modifyResource;
    Red::ResourcePath resourcePath;

    bool modifyAppearance;
    Red::CName appearanceName;

    bool modifyRecordID;
    Red::TweakDBID recordID;

    bool modifyProxyNodes;
    int32_t nbNodesUnderProxyDiff;
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
    bool autoUpdateNodesUnderProxies;
};

struct WorldStreamingConfig : ExtensionConfig
{
    using ExtensionConfig::ExtensionConfig;

    bool IsDefined() override;
    void LoadYAML(const YAML::Node& aNode) override;
    bool ParseResource(const YAML::Node& aNode, Red::ResourcePath& aValue, bool& aFlag);
    bool ParseName(const YAML::Node& aNode, Red::CName& aValue, bool& aFlag);
    bool ParseRecordID(const YAML::Node& aNode, Red::TweakDBID& aValue, bool& aFlag);
    bool ParseSubDeletions(const YAML::Node& aDeletionsNode, const YAML::Node& aCountNode,
                           Core::Vector<int64_t>& aDeletions, int64_t& aExpectedCount);

    Core::Vector<std::string> blocks;
    Core::Vector<WorldSectorMod> sectors;
};
}
