#pragma once

#include "App/Extensions/ModuleBase.hpp"
#include "App/Extensions/QuestPhase/Unit.hpp"
#include "Red/QuestLoader.hpp"
#include "Red/QuestPhase.hpp"

namespace App
{
class QuestPhaseModule : public ConfigurableUnitModule<QuestPhaseUnit>
{
public:
    std::string_view GetName() override;
    bool Load() override;
    void Reload() override;
    bool Unload() override;

private:
    using ConnectionPoint = std::pair<Red::Handle<Red::questGraphDefinition>, Red::Handle<Red::questNodeDefinition>>;

    void PreparePhases();
    static void OnPhasePreload(void* aLoader, Red::ResourcePath aPhasePath,
                               Red::Handle<Red::questQuestPhaseResource>& aPhaseResource);
    // static void OnPhaseReady(Red::questPhaseNodeDefinition* aPhaseNode,
    //                          Red::Handle<Red::questQuestPhaseResource>& aPhaseResource);

    static void PatchPhase(Red::Handle<Red::questQuestPhaseResource>& aPhaseResource);
    static bool PatchPhase(Red::Handle<Red::questQuestPhaseResource>& aPhaseResource, const QuestPhaseMod& aPhaseMod);
    static ConnectionPoint FindConnectionPoint(const Red::Handle<Red::questGraphDefinition>& aPhaseGraph,
                                               const Core::Vector<uint16_t>& aNodePath, uint32_t aStep = 0);
    static Red::Handle<Red::questSocketDefinition> ResolveSocket(Red::Handle<Red::questNodeDefinition>& aNode,
                                                                 Red::questSocketType aSocketType,
                                                                 Red::CName aSocketName);
    static void AddConnection(Red::Handle<Red::questNodeDefinition>& aOutNode, Red::CName aOutSocket,
                              Red::Handle<Red::questNodeDefinition>& aInNode, Red::CName aInSocket);
    static Red::Handle<Red::questPhaseNodeDefinition> CreatePhaseNode(
        const Red::Handle<Red::questGraphDefinition>& aPhaseGraph, const QuestPhaseMod& aPhaseMod, uint16_t aParentId);
    static uint16_t GeneratePhaseNodeID(const char* aData, uint32_t aLength);

    inline static Core::Map<Red::ResourcePath, Core::Vector<QuestPhaseMod>> s_phases;
};
}
