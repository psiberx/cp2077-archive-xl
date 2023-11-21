#include "Unit.hpp"

bool App::QuestPhaseUnit::IsDefined()
{
    return !phases.empty();
}

void App::QuestPhaseUnit::LoadYAML(const YAML::Node& aNode)
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
            const auto& connectionNode = phaseNode["connection"];

            if (!pathNode.IsDefined() || !pathNode.IsScalar()
                || !parentNode.IsDefined() || !parentNode.IsScalar()
                || !connectionNode.IsDefined() || !connectionNode.IsSequence())
            {
                continue;
            }

            QuestPhaseMod phaseData{};
            phaseData.mod = name;
            phaseData.phasePath = pathNode.Scalar();
            phaseData.parentPath = parentNode.Scalar();
            phaseData.connection = connectionNode.as<Core::Vector<uint16_t>>();

            if (phaseData.connection.empty())
                continue;

            phases.emplace_back(std::move(phaseData));
        }
    }
}

