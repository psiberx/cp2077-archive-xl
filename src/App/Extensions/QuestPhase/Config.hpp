#pragma once

#include "App/Extensions/ExtensionBase.hpp"

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

struct QuestPhaseConfig : ExtensionConfig
{
    using ExtensionConfig::ExtensionConfig;

    bool IsDefined() override;
    void LoadYAML(const YAML::Node& aNode) override;
    bool FillConnection(const YAML::Node& aNode, QuestPhaseConnection& aConnection);

    Core::Vector<QuestPhaseMod> phases;
};
}
