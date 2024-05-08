#include "Config.hpp"

bool App::QuestPhaseConfig::IsDefined()
{
    return !phases.empty();
}

void App::QuestPhaseConfig::LoadYAML(const YAML::Node& aNode)
{
    const auto& questNode = aNode["quest"];

    if (!questNode.IsDefined())
        return;

    const auto& phasesNode = questNode["phases"];
    if (phasesNode.IsDefined() && phasesNode.IsSequence())
    {
        for (const auto& phaseNode : phasesNode)
        {
            if (!phaseNode.IsMap())
            {
                continue;
            }

            const auto& pathNode = phaseNode["path"];
            const auto& parentNode = phaseNode["parent"];

            if (!pathNode.IsDefined() || !pathNode.IsScalar() || !parentNode.IsDefined() || !parentNode.IsScalar())
            {
                continue;
            }

            QuestPhaseMod phaseData{};
            phaseData.mod = name;
            phaseData.phasePath = pathNode.Scalar();
            phaseData.parentPath = parentNode.Scalar();

            FillConnection(phaseNode["connection"], phaseData.input);
            FillConnection(phaseNode["input"], phaseData.input);
            FillConnection(phaseNode["output"], phaseData.output);

            if (phaseData.input.nodePath.empty())
                continue;

            phases.emplace_back(std::move(phaseData));
        }
    }
}

bool App::QuestPhaseConfig::FillConnection(const YAML::Node& aNode, App::QuestPhaseConnection& aConnection)
{
    if (aNode.IsDefined())
    {
        if (aNode.IsMap())
        {
            aConnection.nodePath = aNode["node"].as<Core::Vector<uint16_t>>();
            aConnection.socketName = aNode["socket"].Scalar().data();
            return true;
        }
        if (aNode.IsSequence())
        {
            aConnection.nodePath = aNode.as<Core::Vector<uint16_t>>();
            return true;
        }
    }

    return false;
}
