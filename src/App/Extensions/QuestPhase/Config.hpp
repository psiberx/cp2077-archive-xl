#pragma once

#include "App/Extensions/ModuleBase.hpp"

namespace App
{
struct QuestPhaseConnection
{
    Core::Vector<uint16_t> nodePath;
    Red::CName socketName;
};

struct QuestPhaseMod
{
    std::string mod;
    std::string phasePath;
    std::string parentPath;
    QuestPhaseConnection input;
    QuestPhaseConnection output;
};

struct QuestPhaseConfig : ModuleConfig
{
    using ModuleConfig::ModuleConfig;

    bool IsDefined() override;
    void LoadYAML(const YAML::Node& aNode) override;
    bool FillConnection(const YAML::Node& aNode, QuestPhaseConnection& aConnection);

    Core::Vector<QuestPhaseMod> phases;
};
}
