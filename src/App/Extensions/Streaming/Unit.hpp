#pragma once

#include "App/Extensions/ModuleBase.hpp"

namespace App
{
struct WorldNodeDeletion
{
    uint32_t nodeIndex;
    Red::CName nodeType;
};

struct StreamingSectorMod
{
    std::string mod;
    std::string path;
    uint32_t expectedNodes;
    Core::Vector<WorldNodeDeletion> deletions;
};

struct StreamingUnit : ConfigurableUnit
{
    using ConfigurableUnit::ConfigurableUnit;

    bool IsDefined() override;
    void LoadYAML(const YAML::Node& aNode) override;

    Core::Vector<std::string> blocks;
    Core::Vector<StreamingSectorMod> sectors;
};
}
