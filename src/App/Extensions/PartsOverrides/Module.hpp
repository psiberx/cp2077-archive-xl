#pragma once

#include "App/Extensions/ModuleBase.hpp"
#include "App/Extensions/PartsOverrides/State.hpp"
#include "Red/AppearanceChanger.hpp"
#include "Red/GarmentAssembler.hpp"

namespace App
{
class PartsOverridesModule : public Module
{
public:
    bool Load() override;
    bool Unload() override;
    std::string_view GetName() override;

private:
    static void OnAddGarmentItem(uintptr_t, Red::WeakHandle<Red::ent::Entity>&, Red::GarmentItemAddRequest&);
    static void OnOverrideGarmentItem(uintptr_t, Red::WeakHandle<Red::ent::Entity>&, Red::GarmentItemOverrideRequest&);
    static void OnRemoveGarmentItem(uintptr_t, Red::WeakHandle<Red::ent::Entity>&, Red::GarmentItemRemoveRequest&);
    static void OnComputeGarment(Red::Handle<Red::ent::Entity>&, uintptr_t, Red::SharedPtr<Red::GarmentComputeData>&,
                                 uintptr_t, uintptr_t, uintptr_t, bool);
    static void OnReassembleAppearance(Red::ent::Entity*, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t);
    static void OnGameDetach(uintptr_t);

    static void RegisterOverrides(Core::SharedPtr<EntityState>& aEntityState, uint64_t aHash,
                                  Red::Handle<Red::AppearanceDefinition>& aApperance);
    static void UnregisterOverrides(Core::SharedPtr<EntityState>& aEntityState, uint64_t aHash);
    static void ApplyOverrides(Core::SharedPtr<EntityState>& aEntityState,
                               Red::DynArray<Red::Handle<Red::ent::IComponent>>&, bool aVerbose = false);
    static void ApplyOverrides(Core::SharedPtr<EntityState>& aEntityState, bool aVerbose = false);

    static inline Core::UniquePtr<OverrideStateManager> s_states;
    static inline std::mutex s_lock;
};
}
