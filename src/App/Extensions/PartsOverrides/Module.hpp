#pragma once

#include "App/Extensions/ModuleBase.hpp"
#include "App/Extensions/PartsOverrides/Dynamic.hpp"
#include "App/Extensions/PartsOverrides/States.hpp"
#include "App/Extensions/PartsOverrides/Unit.hpp"
#include "Red/AppearanceChanger.hpp"
#include "Red/EntityTemplate.hpp"
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

    static void EnableGarmentOffsets();
    static void DisableGarmentOffsets();

private:
    void ConfigureTags();

    static void OnAddItem(uintptr_t, Red::WeakHandle<Red::Entity>&, Red::GarmentItemAddRequest&);
    static void OnAddCustomItem(uintptr_t, Red::WeakHandle<Red::Entity>&, Red::GarmentItemAddCustomRequest&);
    static void OnChangeItem(uintptr_t, Red::WeakHandle<Red::Entity>&, Red::GarmentItemChangeRequest&);
    static void OnChangeCustomItem(uintptr_t, Red::WeakHandle<Red::Entity>&, Red::GarmentItemChangeCustomRequest&);
    static void OnRemoveItem(uintptr_t, Red::WeakHandle<Red::Entity>&, Red::GarmentItemRemoveRequest&);
    static void OnRegisterPart(uintptr_t, Red::Handle<Red::EntityTemplate>& aPart,
                               Red::Handle<Red::ComponentsStorage>& aComponents,
                               Red::Handle<Red::AppearanceDefinition>& aAppearance);

    static int64_t OnGetBaseMeshOffset(Red::Handle<Red::IComponent>& aComponent,
                                       Red::Handle<Red::EntityTemplate>& aTemplate);
    static void OnComputeGarment(Red::Handle<Red::Entity>&, Red::DynArray<int32_t>&,
                                 Red::SharedPtr<Red::GarmentComputeData>&, uintptr_t, uintptr_t, uintptr_t, bool);
    static void OnReassembleAppearance(Red::Entity*, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t);
    static void OnCreateResourcePath(Red::ResourcePath* aPath, const std::string_view* aPathStr);
    static void OnGameDetach();

    static void RegisterOffsetOverrides(Core::SharedPtr<EntityState>& aEntityState, uint64_t aHash,
                                      Red::Handle<Red::AppearanceDefinition>& aApperance, int32_t aOffset);
    static void UnregisterOffsetOverrides(Core::SharedPtr<EntityState>& aEntityState, uint64_t aHash);

    static void RegisterComponentOverrides(Core::SharedPtr<EntityState>& aEntityState, uint64_t aHash,
                                           Red::Handle<Red::AppearanceDefinition>& aApperance);
    static void RegisterComponentOverrides(Core::SharedPtr<EntityState>& aEntityState, uint64_t aHash,
                                           Red::DynArray<Red::AppearancePartOverrides>& aOverrides);
    static void UnregisterComponentOverrides(Core::SharedPtr<EntityState>& aEntityState, uint64_t aHash);

    static void UpdatePartAttributes(Core::SharedPtr<EntityState>& aEntityState,
                                     Red::Handle<Red::AppearanceDefinition>& aApperance);
    static void UpdatePartAssignments(Core::SharedPtr<EntityState>& aEntityState,
                                      Red::DynArray<Red::Handle<Red::IComponent>>& aComponents,
                                      Red::DynArray<Red::ResourcePath>& aPartResources);
    static void UpdatePartAssignments(Red::DynArray<Red::Handle<Red::IComponent>>& aComponents,
                                      Red::ResourcePath aPartResource);

    static void UpdateDynamicAppearance(Core::SharedPtr<EntityState>& aEntityState);

    static void ApplyDynamicAppearance(Core::SharedPtr<EntityState>& aEntityState);
    static void ApplyDynamicAppearance(Core::SharedPtr<EntityState>& aEntityState,
                                        Red::DynArray<Red::Handle<Red::IComponent>>& aComponents);
    static void ApplyComponentOverrides(Core::SharedPtr<EntityState>& aEntityState, bool aVerbose = false);
    static void ApplyComponentOverrides(Core::SharedPtr<EntityState>& aEntityState,
                                        Red::DynArray<Red::Handle<Red::IComponent>>& aComponents,
                                        bool aVerbose = false);
    static void ApplyOffsetOverrides(Core::SharedPtr<EntityState>& aEntityState, Red::DynArray<int32_t>& aOffsets,
                                     Red::DynArray<Red::ResourcePath>& aResourcePaths);

    static inline Core::SharedPtr<DynamicAppearanceController> s_dynamicAppearance;
    static inline Core::UniquePtr<OverrideStateManager> s_stateManager;
    static inline Core::SharedPtr<OverrideTagManager> s_tagManager;
    static inline bool s_garmentOffsetsEnabled;
    static inline std::shared_mutex s_mutex;
};
}
