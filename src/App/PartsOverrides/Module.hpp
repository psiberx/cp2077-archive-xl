#pragma once

#include "Garment.hpp"
#include "State.hpp"
#include "Raws.hpp"
#include "App/Common/ModuleBase.hpp"

namespace App
{
class PartsOverridesModule : public Module
{
public:
    bool Load() override;
    bool Unload() override;
    std::string_view GetName() override;

private:
    static void OnAddGarmentItem(uintptr_t, RED4ext::WeakHandle<RED4ext::ent::Entity>&, GarmentItemAddRequest&);
    static void OnOverrideGarmentItem(uintptr_t, RED4ext::WeakHandle<RED4ext::ent::Entity>&, GarmentItemOverrideRequest&);
    static void OnRemoveGarmentItem(uintptr_t, RED4ext::WeakHandle<RED4ext::ent::Entity>&, GarmentItemRemoveRequest&);
    static void OnComputeGarment(RED4ext::Handle<RED4ext::ent::Entity>&, uintptr_t, RED4ext::SharedPtr<GarmentComputeData>&, uintptr_t, uintptr_t, uintptr_t, bool);
    static void OnReassembleAppearance(RED4ext::ent::Entity* aEntity, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t);
    static void OnGameDetach(uintptr_t);

    static void RegisterOverrides(Core::SharedPtr<EntityState>&, ItemHash, ItemAppearance&);
    static void UnregisterOverrides(Core::SharedPtr<EntityState>&, ItemHash);
    static void ApplyOverrides(Core::SharedPtr<EntityState>&, RED4ext::DynArray<RED4ext::Handle<RED4ext::ent::IComponent>>&);

    static inline Core::UniquePtr<StateStorage> s_states;
    static inline std::mutex s_lock;
};
}
