#include "Module.hpp"

namespace
{
constexpr auto ModuleName = "QuestPhase";
}

std::string_view App::QuestPhaseModule::GetName()
{
    return ModuleName;
}

bool App::QuestPhaseModule::Load()
{
    HookBefore<Raw::QuestLoader::ProcessPhaseResource>(&OnPhasePreload).OrThrow();
    HookBefore<Raw::QuestsSystem::OnGameRestored>(&OnGameRestored).OrThrow();

    return true;
}

bool App::QuestPhaseModule::Unload()
{
    Unhook<Raw::QuestLoader::ProcessPhaseResource>();
    Unhook<Raw::QuestsSystem::OnGameRestored>();

    return true;
}

void App::QuestPhaseModule::Configure()
{
    s_phases.clear();

    auto depot = Red::ResourceDepot::Get();
    Core::Set<Red::ResourcePath> invalidPaths;

    for (auto& unit : m_configs)
    {
        for (auto& phaseMod : unit.phases)
        {
            auto phasePath = Red::ResourcePath(phaseMod.phasePath.data());

            if (!depot->ResourceExists(phasePath))
            {
                if (!invalidPaths.contains(phasePath))
                {
                    LogWarning("|{}| Phase \"{}\" doesn't exist. Skipped.", ModuleName, phaseMod.phasePath);
                    invalidPaths.insert(phasePath);
                }
                continue;
            }

            auto parentPath = Red::ResourcePath(phaseMod.parentPath.data());

            if (!depot->ResourceExists(parentPath))
            {
                if (!invalidPaths.contains(parentPath))
                {
                    LogWarning("|{}| Phase \"{}\" doesn't exist. Skipped.", ModuleName, phaseMod.parentPath);
                    invalidPaths.insert(parentPath);
                }
                continue;
            }

            s_phases[parentPath].emplace_back(std::move(phaseMod));
        }

        unit.phases.clear();
    }
}

void App::QuestPhaseModule::OnPhasePreload(void* aLoader, Red::ResourcePath aPhasePath,
                                           Red::Handle<Red::questQuestPhaseResource>& aPhaseResource)
{
    const auto& phaseMods = s_phases.find(aPhaseResource->path);

    if (phaseMods == s_phases.end())
        return;

    LogInfo("|{}| Patching phase \"{}\"...", ModuleName, phaseMods.value().begin()->parentPath);

    for (const auto& phaseMod : phaseMods.value())
    {
        PatchPhase(aPhaseResource, phaseMod);
    }
}

void App::QuestPhaseModule::OnGameRestored(Red::QuestsSystem* aSystem)
{
    if (s_forced.empty())
        return;

    auto& factManager = Raw::QuestsSystem::FactManager::Ref(aSystem);
    const auto& questList = Raw::QuestsSystem::QuestList::Ref(aSystem);

    for (const auto& [questPath, phaseNodeIds] : s_forced)
    {
        Red::QuestNodeID questId = 0;

        for (; questId < questList.size; ++questId)
        {
            if (questList[questId] == questPath)
                break;
        }

        if (questId >= questList.size)
            continue;

        Red::QuestNodePath questNodePath{questId};
        Red::DynArray<Red::CName> phaseNodeSockets{"In1"};

        for (const auto& phaseNodeId : phaseNodeIds)
        {
            Red::QuestNodeKey phaseNodeKey{questNodePath, phaseNodeId};
            Red::FactID phaseFactId{phaseNodeKey};

            if (!factManager.GetFact(phaseFactId))
            {
                Raw::QuestsSystem::ForceStartNode(aSystem, phaseNodeKey, phaseNodeSockets);

                factManager.SetFact(phaseFactId, 1);
            }
        }
    }
}

bool App::QuestPhaseModule::PatchPhase(Red::Handle<Red::questQuestPhaseResource>& aPhaseResource,
                                       const App::QuestPhaseMod& aPhaseMod)
{
    auto& rootPhaseGraph = Red::Cast<Red::questGraphDefinition>(aPhaseResource->graph);
    if (!rootPhaseGraph)
    {
        LogWarning(R"(|{}| Can't merge phase "{}" from "{}", parent phase is not loaded.)",
                   ModuleName, aPhaseMod.phasePath, aPhaseMod.mod);
        return false;
    }

    auto [inputPhaseGraph, inputNode] = FindConnectionPoint(rootPhaseGraph, aPhaseMod.input.nodePath);
    if (!inputPhaseGraph || !inputNode)
    {
        LogWarning(R"(|{}| Can't merge phase "{}" from "{}", input node doesn't exist.)",
                   ModuleName, aPhaseMod.phasePath, aPhaseMod.mod);
        return false;
    }

    auto modPhaseNode = CreatePhaseNode(inputPhaseGraph, aPhaseMod, inputNode->id);
    if (!modPhaseNode)
    {
        LogWarning(R"(|{}| Can't merge phase "{}" from "{}", node with the same id already exists.)",
                   ModuleName, aPhaseMod.phasePath, aPhaseMod.mod);
        return false;
    }

    if (!aPhaseMod.output.nodePath.empty())
    {
        auto [outputPhaseGraph, outputNode] = FindConnectionPoint(rootPhaseGraph, aPhaseMod.output.nodePath);
        if (!outputPhaseGraph || !outputNode)
        {
            LogWarning(R"(|{}| Can't merge phase "{}" from "{}", output node doesn't exist.)",
                       ModuleName, aPhaseMod.phasePath, aPhaseMod.mod);
            return false;
        }

        auto inSocketName =
            aPhaseMod.output.socketName
                ? aPhaseMod.output.socketName
                : (outputNode->GetType()->IsA(Red::GetClass<Red::questPhaseNodeDefinition>()) ? "In1" : "In");
        AddConnection(modPhaseNode, "Out", outputNode, inSocketName);

        RemoveConnection(outputNode, inputNode);
    }

    AddConnection(inputNode, aPhaseMod.input.socketName ? aPhaseMod.input.socketName : "Out", modPhaseNode, "In1");

    if (Red::IsInstanceOf<Red::questQuestResource>(aPhaseResource))
    {
        s_forced[aPhaseResource->path].insert(modPhaseNode->id);
    }

    LogInfo(R"(|{}| Merged phase "{}" from "{}".)", ModuleName, aPhaseMod.phasePath, aPhaseMod.mod);

    return true;
}

App::QuestPhaseModule::ConnectionPoint App::QuestPhaseModule::FindConnectionPoint(
    const Red::Handle<Red::questGraphDefinition>& aPhaseGraph, const Core::Vector<uint16_t>& aNodePath, uint32_t aStep)
{
    for (const auto& node : aPhaseGraph->nodes)
    {
        if (const auto& questNode = Red::Cast<Red::questNodeDefinition>(node))
        {
            if (aNodePath.empty())
            {
                if (Red::IsInstanceOf<Red::questStartNodeDefinition>(node))
                {
                    return {aPhaseGraph, questNode};
                }
            }
            else if (questNode->id == aNodePath[aStep])
            {
                if (aStep == aNodePath.size() - 1)
                {
                    return {aPhaseGraph, questNode};
                }

                if (const auto& phaseNode = Red::Cast<Red::questPhaseNodeDefinition>(node))
                {
                    if (phaseNode->phaseGraph)
                    {
                        return FindConnectionPoint(phaseNode->phaseGraph, aNodePath, aStep + 1);
                    }
                }

                break;
            }
        }
    }

    return {};
}

Red::Handle<Red::questSocketDefinition> App::QuestPhaseModule::ResolveSocket(
    Red::Handle<Red::questNodeDefinition>& aNode, Red::questSocketType aSocketType, Red::CName aSocketName)
{
    for (const auto& graphSocket : aNode->sockets)
    {
        if (const auto& questSocket = Red::Cast<Red::questSocketDefinition>(graphSocket))
        {
            if (questSocket->type == aSocketType && questSocket->name == aSocketName)
            {
                return questSocket;
            }
        }
    }

    auto questSocket = Red::MakeHandle<Red::questSocketDefinition>();
    questSocket->type = aSocketType;
    questSocket->name = aSocketName;

    aNode->sockets.PushBack(questSocket);

    Raw::QuestSocketDefinition::OwnerNode::Ref(questSocket.instance) = aNode;

    return questSocket;
}

void App::QuestPhaseModule::AddConnection(Red::Handle<Red::questNodeDefinition>& aOutNode, Red::CName aOutSocket,
                                          Red::Handle<Red::questNodeDefinition>& aInNode, Red::CName aInSocket)
{
    auto outSocket = ResolveSocket(aOutNode, Red::questSocketType::Output, aOutSocket);
    auto inSocket = ResolveSocket(aInNode, Red::questSocketType::Input, aInSocket);

    auto connection = Red::MakeHandle<Red::graphGraphConnectionDefinition>();
    connection->source = outSocket;
    connection->destination = inSocket;

    outSocket->connections.PushBack(connection);
    inSocket->connections.PushBack(connection);
}

void App::QuestPhaseModule::RemoveConnection(Red::Handle<Red::questNodeDefinition>& aOutNode,
                                             Red::Handle<Red::questNodeDefinition>& aInNode)
{
    for (auto& rawSocket : aOutNode->sockets)
    {
        auto& questSocket = Red::Cast<Red::questSocketDefinition>(rawSocket);
        if (questSocket->type == Red::questSocketType::Output)
        {
            for (auto i = static_cast<int32_t>(questSocket->connections.size) - 1; i >= 0; --i)
            {
                auto& destinationNode =
                    Raw::QuestSocketDefinition::OwnerNode::Ref(questSocket->connections[i]->destination.instance);
                if (destinationNode.instance == aInNode.instance)
                {
                    questSocket->connections.RemoveAt(i);
                }
            }
        }
    }

    for (auto& rawSocket : aInNode->sockets)
    {
        auto& questSocket = Red::Cast<Red::questSocketDefinition>(rawSocket);
        if (questSocket->type == Red::questSocketType::Input)
        {
            for (auto i = static_cast<int32_t>(questSocket->connections.size) - 1; i >= 0; --i)
            {
                auto& sourceNode =
                    Raw::QuestSocketDefinition::OwnerNode::Ref(questSocket->connections[i]->source.instance);
                if (sourceNode.instance == aOutNode.instance)
                {
                    questSocket->connections.RemoveAt(i);
                }
            }
        }
    }
}

Red::Handle<Red::questPhaseNodeDefinition> App::QuestPhaseModule::CreatePhaseNode(
    const Red::Handle<Red::questGraphDefinition>& aPhaseGraph, const QuestPhaseMod& aPhaseMod, uint16_t aParentID)
{
    auto phaseNodeKey = aPhaseMod.phasePath + ":" + std::to_string(aParentID);
    auto phaseNodeId = GeneratePhaseNodeID(phaseNodeKey.data(), phaseNodeKey.size());

    for (const auto& node : aPhaseGraph->nodes)
    {
        if (auto questNode = Red::Cast<Red::questNodeDefinition>(node))
        {
            if (questNode->id == phaseNodeId)
            {
                return {};
            }
        }
    }

    auto phaseNode = Red::MakeHandle<Red::questPhaseNodeDefinition>();
    phaseNode->phaseResource = {aPhaseMod.phasePath.data()};
    phaseNode->id = phaseNodeId;

    aPhaseGraph->nodes.PushBack(phaseNode);

    return phaseNode;
}

uint16_t App::QuestPhaseModule::GeneratePhaseNodeID(const char* aData, uint32_t aLength)
{
    uint8_t x;
    uint16_t crc = 0xFFFF;

    while (aLength--)
    {
        x = crc >> 8 ^ *aData++;
        x ^= x>>4;
        crc = (crc << 8) ^ ((uint16_t)(x << 12)) ^ ((uint16_t)(x <<5)) ^ ((uint16_t)x);
    }

    return crc;
}
