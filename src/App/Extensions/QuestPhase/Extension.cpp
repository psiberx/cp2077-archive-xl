#include "Extension.hpp"
#include "Core/Facades/Container.hpp"
#include "App/Extensions/ResourceMeta/Extension.hpp"

namespace
{
constexpr auto ExtensionName = "QuestPhase";
}

std::string_view App::QuestPhaseExtension::GetName()
{
    return ExtensionName;
}

bool App::QuestPhaseExtension::Load()
{
    HookBefore<Raw::QuestLoader::ProcessPhaseResource>(&OnPhasePreload).OrThrow();
    HookBefore<Raw::QuestsSystem::OnGameRestored>(&OnGameRestored).OrThrow();
    HookBefore<Raw::QuestRootInstance::Start>(&OnQuestStart).OrThrow();

    s_resourcePathRegistry = Core::Resolve<ResourcePathRegistry>();

    return true;
}

bool App::QuestPhaseExtension::Unload()
{
    Unhook<Raw::QuestLoader::ProcessPhaseResource>();
    Unhook<Raw::QuestsSystem::OnGameRestored>();
    Unhook<Raw::QuestRootInstance::Start>();

    return true;
}

void App::QuestPhaseExtension::Configure()
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
                    LogWarning("[{}] Phase \"{}\" doesn't exist. Skipped.", ExtensionName, phaseMod.phasePath);
                    invalidPaths.insert(phasePath);
                }
                continue;
            }

            for (const auto& parentPathStr : ResourceMetaExtension::ExpandList(phaseMod.parentPaths))
            {
                auto parentPath = Red::ResourcePath(parentPathStr.data());

                if (!depot->ResourceExists(parentPath))
                {
                    if (!invalidPaths.contains(parentPath))
                    {
                        LogWarning("[{}] Phase \"{}\" doesn't exist. Skipped.", ExtensionName, parentPathStr);
                        invalidPaths.insert(parentPath);
                    }
                    continue;
                }

                s_phases[parentPath].emplace_back(phaseMod);
            }
        }

        unit.phases.clear();
    }
}

void App::QuestPhaseExtension::OnPhasePreload(void* aLoader, Red::ResourcePath aPhasePath,
                                              Red::Handle<Red::questQuestPhaseResource>& aPhaseResource)
{
    const auto& phaseMods = s_phases.find(aPhaseResource->path);

    if (phaseMods == s_phases.end())
        return;

    LogInfo("[{}] Patching phase \"{}\"...", ExtensionName,
            s_resourcePathRegistry->ResolvePathOrHash(aPhaseResource->path));

    for (const auto& phaseMod : phaseMods.value())
    {
        PatchPhase(aPhaseResource, phaseMod);
    }
}

void App::QuestPhaseExtension::OnGameRestored(Red::QuestsSystem* aSystem)
{
    if (s_forced.empty())
        return;

    auto& factManager = Raw::QuestsSystem::FactManager::Ref(aSystem);
    auto& nodeHashMap = Raw::QuestsSystem::NodeHashMap::Ref(aSystem);
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
        Red::QuestNodePathHash questNodeHash = Red::MakeQuestNodePathHash(questNodePath);

        if (!nodeHashMap.Get(questNodeHash))
        {
            nodeHashMap.Insert(questNodeHash, questNodePath);
        }

        for (const auto& phaseNodeId : phaseNodeIds)
        {
            Red::QuestNodeKey phaseNodeKey{questNodeHash, phaseNodeId};
            Red::FactID phaseFactId{phaseNodeKey};

            if (!factManager->GetFact(phaseFactId))
            {
                Red::DynArray<Red::CName> phaseNodeSockets{"In1"};
                Raw::QuestsSystem::ForceStartNode(aSystem, phaseNodeKey, phaseNodeSockets);

                factManager->SetFact(phaseFactId, 1);
            }
        }
    }
}

void App::QuestPhaseExtension::OnQuestStart(Red::questRootInstance* aInstance, Red::QuestContext* aContext,
                                         const Red::Handle<Red::questQuestResource>& aResource)
{
    if (s_forced.empty())
        return;

    auto& factManager = Raw::QuestsSystem::FactManager::Ref(aContext->phaseContext.questsSystem);
    const auto& questList = Raw::QuestsSystem::QuestList::Ref(aContext->phaseContext.questsSystem);

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
        Red::QuestNodePathHash questNodeHash = Red::MakeQuestNodePathHash(questNodePath);

        for (const auto& phaseNodeId : phaseNodeIds)
        {
            Red::QuestNodeKey phaseNodeKey{questNodeHash, phaseNodeId};
            Red::FactID phaseFactId{phaseNodeKey};

            factManager->SetFact(phaseFactId, 1);
        }
    }
}

bool App::QuestPhaseExtension::PatchPhase(Red::Handle<Red::questQuestPhaseResource>& aPhaseResource,
                                       const App::QuestPhaseMod& aPhaseMod)
{
    auto& rootPhaseGraph = Red::Cast<Red::questGraphDefinition>(aPhaseResource->graph);
    if (!rootPhaseGraph)
    {
        LogWarning(R"([{}] Can't merge phase "{}" from "{}", parent phase is not loaded.)",
                   ExtensionName, aPhaseMod.phasePath, aPhaseMod.mod);
        return false;
    }

    auto [inputPhaseGraph, inputNode] = FindConnectionPoint(rootPhaseGraph, aPhaseMod.input.nodePath);
    if (!inputPhaseGraph || !inputNode)
    {
        LogWarning(R"([{}] Can't merge phase "{}" from "{}", input node doesn't exist.)",
                   ExtensionName, aPhaseMod.phasePath, aPhaseMod.mod);
        return false;
    }

    auto modPhaseNode = CreatePhaseNode(inputPhaseGraph, aPhaseMod, inputNode->id);
    if (!modPhaseNode)
    {
        LogWarning(R"([{}] Can't merge phase "{}" from "{}", node with the same id already exists.)",
                   ExtensionName, aPhaseMod.phasePath, aPhaseMod.mod);
        return false;
    }

    if (!aPhaseMod.output.nodePath.empty())
    {
        auto [outputPhaseGraph, outputNode] = FindConnectionPoint(rootPhaseGraph, aPhaseMod.output.nodePath);
        if (!outputPhaseGraph || !outputNode)
        {
            LogWarning(R"([{}] Can't merge phase "{}" from "{}", output node doesn't exist.)",
                       ExtensionName, aPhaseMod.phasePath, aPhaseMod.mod);
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

    LogInfo(R"([{}] Merged phase "{}" from "{}".)", ExtensionName, aPhaseMod.phasePath, aPhaseMod.mod);

    return true;
}

App::QuestPhaseExtension::ConnectionPoint App::QuestPhaseExtension::FindConnectionPoint(
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

Red::Handle<Red::questSocketDefinition> App::QuestPhaseExtension::ResolveSocket(
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

void App::QuestPhaseExtension::AddConnection(Red::Handle<Red::questNodeDefinition>& aOutNode, Red::CName aOutSocket,
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

void App::QuestPhaseExtension::RemoveConnection(Red::Handle<Red::questNodeDefinition>& aOutNode,
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

Red::Handle<Red::questPhaseNodeDefinition> App::QuestPhaseExtension::CreatePhaseNode(
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

uint16_t App::QuestPhaseExtension::GeneratePhaseNodeID(const char* aData, uint32_t aLength)
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
