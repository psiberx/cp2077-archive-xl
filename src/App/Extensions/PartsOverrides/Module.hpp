#pragma once

#include "App/Extensions/ModuleBase.hpp"
#include "App/Extensions/PartsOverrides/States.hpp"
#include "App/Extensions/PartsOverrides/Unit.hpp"
#include "Red/AppearanceChanger.hpp"
#include "Red/GarmentAssembler.hpp"

namespace App
{
class PartsOverridesModule : public ConfigurableUnitModule<PartsOverridesUnit>
{
public:
    bool Load() override;
    void Reload() override;
    bool Unload() override;
    std::string_view GetName() override;

private:
    void ConfigureTags();

    static void OnAddItem(uintptr_t, Red::WeakHandle<Red::ent::Entity>&, Red::GarmentItemAddRequest&);
    static void OnAddCustomItem(uintptr_t, Red::WeakHandle<Red::ent::Entity>&, Red::GarmentItemAddCustomRequest&);
    static void OnChangeItem(uintptr_t, Red::WeakHandle<Red::ent::Entity>&, Red::GarmentItemChangeRequest&);
    static void OnChangeCustomItem(uintptr_t, Red::WeakHandle<Red::ent::Entity>&, Red::GarmentItemChangeCustomRequest&);
    static void OnRemoveItem(uintptr_t, Red::WeakHandle<Red::ent::Entity>&, Red::GarmentItemRemoveRequest&);
    static void OnComputeGarment(Red::Handle<Red::ent::Entity>&, Red::DynArray<int32_t>&,
                                 Red::SharedPtr<Red::GarmentComputeData>&, uintptr_t, uintptr_t, uintptr_t, bool);
    static void OnReassembleAppearance(Red::ent::Entity*, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t);
    static void OnGameDetach();

    static void RegisterOffsetOverrides(Core::SharedPtr<EntityState>& aEntityState, uint64_t aHash,
                                        Red::Handle<Red::AppearanceDefinition>& aApperance,
                                        int32_t aOffset);
    static void RegisterPartsOverrides(Core::SharedPtr<EntityState>& aEntityState, uint64_t aHash,
                                       Red::Handle<Red::AppearanceDefinition>& aApperance);
    static void RegisterPartsOverrides(Core::SharedPtr<EntityState>& aEntityState, uint64_t aHash,
                                       Red::DynArray<Red::AppearancePartOverrides>& aOverrides);
    static void UnregisterOffsetOverrides(Core::SharedPtr<EntityState>& aEntityState, uint64_t aHash);
    static void UnregisterPartsOverrides(Core::SharedPtr<EntityState>& aEntityState, uint64_t aHash);
    static void ApplyResourceOverrides(Core::SharedPtr<EntityState>& aEntityState, Red::DynArray<int32_t>& aOffsets,
                                       Red::DynArray<Red::ResourcePath>& aResourcePaths);
    static void ApplyComponentOverrides(Core::SharedPtr<EntityState>& aEntityState,
                                        Red::DynArray<Red::Handle<Red::ent::IComponent>>&, bool aVerbose = false);
    static void ApplyComponentOverrides(Core::SharedPtr<EntityState>& aEntityState, bool aVerbose = false);

    static inline Core::UniquePtr<OverrideStateManager> s_stateManager;
    static inline Core::SharedPtr<OverrideTagManager> s_tagManager;
    static inline std::shared_mutex s_mutex;
};
}
