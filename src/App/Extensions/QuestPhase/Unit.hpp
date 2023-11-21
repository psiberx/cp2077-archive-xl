#pragma once

#include "App/Extensions/ModuleBase.hpp"

namespace App
{
struct QuestPhaseMod
{
    std::string mod;
    std::string phasePath;
    std::string parentPath;
    Core::Vector<uint16_t> connection;
};

struct QuestPhaseUnit : ConfigurableUnit
{
    using ConfigurableUnit::ConfigurableUnit;

    bool IsDefined() override;
    void LoadYAML(const YAML::Node& aNode) override;

    Core::Vector<QuestPhaseMod> phases;
};
}
