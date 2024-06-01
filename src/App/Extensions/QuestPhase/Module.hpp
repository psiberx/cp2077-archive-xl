#pragma once

#include "App/Extensions/ModuleBase.hpp"
#include "App/Extensions/QuestPhase/Config.hpp"
#include "Red/QuestLoader.hpp"
#include "Red/QuestPhase.hpp"
#include "Red/QuestsSystem.hpp"

namespace App
{
class QuestPhaseModule : public ConfigurableModuleImpl<QuestPhaseConfig>
{
public:
    std::string_view GetName() override;
    bool Load() override;
    bool Unload() override;
    void Configure() override;

private:
    using ConnectionPoint = std::pair<Red::Handle<Red::questGraphDefinition>, Red::Handle<Red::questNodeDefinition>>;

    static void OnPhasePreload(void* aLoader, Red::ResourcePath aPhasePath,
                               Red::Handle<Red::questQuestPhaseResource>& aPhaseResource);
    static void OnGameRestored(Red::QuestsSystem* aSystem);

    static bool PatchPhase(Red::Handle<Red::questQuestPhaseResource>& aPhaseResource, const QuestPhaseMod& aPhaseMod);
    static ConnectionPoint FindConnectionPoint(const Red::Handle<Red::questGraphDefinition>& aPhaseGraph,
                                               const Core::Vector<uint16_t>& aNodePath, uint32_t aStep = 0);
    static Red::Handle<Red::questSocketDefinition> ResolveSocket(Red::Handle<Red::questNodeDefinition>& aNode,
                                                                 Red::questSocketType aSocketType,
                                                                 Red::CName aSocketName);
    static void AddConnection(Red::Handle<Red::questNodeDefinition>& aOutNode, Red::CName aOutSocket,
                              Red::Handle<Red::questNodeDefinition>& aInNode, Red::CName aInSocket);
    static void RemoveConnection(Red::Handle<Red::questNodeDefinition>& aOutNode,
                                 Red::Handle<Red::questNodeDefinition>& aInNode);
    static Red::Handle<Red::questPhaseNodeDefinition> CreatePhaseNode(
        const Red::Handle<Red::questGraphDefinition>& aPhaseGraph, const QuestPhaseMod& aPhaseMod, uint16_t aParentID);
    static uint16_t GeneratePhaseNodeID(const char* aData, uint32_t aLength);

    inline static Core::Map<Red::ResourcePath, Core::Vector<QuestPhaseMod>> s_phases;
    inline static Core::Map<Red::ResourcePath, Core::Set<uint16_t>> s_forced;
};
}
