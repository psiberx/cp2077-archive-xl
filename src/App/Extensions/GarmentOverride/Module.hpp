#pragma once

#include "App/Extensions/ModuleBase.hpp"
#include "App/Extensions/GarmentOverride/Dynamic.hpp"
#include "App/Extensions/GarmentOverride/States.hpp"
#include "App/Extensions/GarmentOverride/Config.hpp"
#include "Red/AppearanceChanger.hpp"
#include "Red/EntityTemplate.hpp"
#include "Red/GarmentAssembler.hpp"
#include "Red/VisualTagsPreset.hpp"

namespace App
{
class GarmentOverrideModule : public ConfigurableModuleImpl<GarmentOverrideConfig>
{
public:
    static constexpr auto ForceHairTag = Red::CName("force_Hair");
    static constexpr auto ForceFlatFeetTag = Red::CName("force_FlatFeet");

    bool Load() override;
    bool Unload() override;
    void Configure() override;
    std::string_view GetName() override;

    static void EnableGarmentOffsets();
    static void DisableGarmentOffsets();

    static Core::SharedPtr<DynamicAppearanceController>& GetDynamicAppearanceController();
    static Core::SharedPtr<OverrideTagManager>& GetTagManager();

private:
    static void OnLoadAppearanceResource(Red::ItemFactoryRequest* aRequest);
    static void OnChangeAppearanceResource(Red::ItemFactoryAppearanceChangeRequest* aRequest);
    static Red::TemplateAppearance* OnResolveAppearance(Red::EntityTemplate* aTemplate, Red::CName aSelector);
    static void OnResolveDefinition(Red::AppearanceResource* aResource,
                                    Red::Handle<Red::AppearanceDefinition>* aDefinition,
                                    Red::CName aAppearanceSelector, uint32_t a4, uint8_t a5);
    static void OnGetVisualTags(Red::AppearanceNameVisualTagsPreset& aPreset,
                                Red::ResourcePath aEntityPath,
                                Red::CName aAppearanceName,
                                Red::TagList& aFinalTags);
    static void OnFindState(uintptr_t, Red::GarmentAssemblerState* aState, Red::WeakHandle<Red::Entity>& aEntityWeak);
    static void OnAddItem(Red::GarmentAssemblerState* aState, Red::GarmentItemAddRequest&);
    static void OnAddCustomItem(Red::GarmentAssemblerState* aState, Red::GarmentItemAddCustomRequest&);
    static void OnChangeItem(Red::GarmentAssemblerState* aState, Red::GarmentItemChangeRequest&);
    static void OnChangeCustomItem(Red::GarmentAssemblerState* aState, Red::GarmentItemChangeCustomRequest&);
    static void OnRemoveItem(uintptr_t, Red::WeakHandle<Red::Entity>&, Red::GarmentItemRemoveRequest&);
    static void OnRegisterPart(uintptr_t, Red::Handle<Red::EntityTemplate>& aPart,
                               Red::Handle<Red::ComponentsStorage>& aComponents,
                               Red::Handle<Red::AppearanceDefinition>& aAppearance);
    static uintptr_t OnProcessGarment(Red::SharedPtr<Red::GarmentProcessor>& aProcessor, uintptr_t a2, uintptr_t a3,
                                      Red::GarmentProcessorParams* aParams);
    static void OnProcessGarmentMesh(Red::GarmentProcessor* aProcessor, uint32_t,
                                     Red::Handle<Red::EntityTemplate>& aPartTemplate,
                                     Red::SharedPtr<Red::ResourceToken<Red::CMesh>>& aMeshToken,
                                     Red::Handle<Red::IComponent>& aComponent, Red::JobGroup& aJobGroup);
    // static int32_t OnGetBaseMeshOffset(Red::Handle<Red::IComponent>& aComponent,
    //                                    Red::Handle<Red::EntityTemplate>& aTemplate);
    static void OnComputeGarment(uintptr_t, Red::Handle<Red::Entity>&, Red::DynArray<int32_t>&,
                                 Red::SharedPtr<Red::GarmentComputeData>&, uintptr_t, uintptr_t, uintptr_t, bool);
    static void OnReassembleAppearance(Red::Entity*, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t);
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

    static bool PrepareDynamicAppearanceName(Red::WeakHandle<Red::Entity>& aEntity,
                                             Red::ResourceTokenPtr<Red::EntityTemplate>& aTemplateToken,
                                             Red::CName& aAppearanceName);
    static void SelectDynamicAppearance(Core::SharedPtr<EntityState>& aEntityState,
                                        DynamicAppearanceName& aSelector,
                                        Red::EntityTemplate* aResource,
                                        Red::TemplateAppearance*& aAppearance);
    static void SelectDynamicAppearance(Core::SharedPtr<EntityState>& aEntityState,
                                        DynamicAppearanceName& aSelector,
                                        Red::AppearanceResource* aResource,
                                        Red::Handle<Red::AppearanceDefinition>& aDefinition);

    static void ApplyDynamicAppearance(Core::SharedPtr<EntityState>& aEntityState);
    static void ApplyDynamicAppearance(Core::SharedPtr<EntityState>& aEntityState,
                                        Red::DynArray<Red::Handle<Red::IComponent>>& aComponents);
    static void ApplyComponentOverrides(Core::SharedPtr<EntityState>& aEntityState, bool aForceUpdate);
    static void ApplyComponentOverrides(Core::SharedPtr<EntityState>& aEntityState,
                                        Red::DynArray<Red::Handle<Red::IComponent>>& aComponents, bool aForceUpdate);
    static void ApplyOffsetOverrides(Core::SharedPtr<EntityState>& aEntityState, Red::DynArray<int32_t>& aOffsets,
                                     Red::DynArray<Red::ResourcePath>& aResourcePaths);

    static void UpdateDynamicAttributes(Core::SharedPtr<EntityState>& aEntityState);
    static void UpdateDynamicAttributes();

    static bool IsUniqueAppearanceName(Red::CName aName);

    static void PatchHeadGarmentOverrides(Core::SharedPtr<EntityState>& aEntityState,
                                          Red::Handle<Red::AppearanceDefinition>& aDefinition);

    static inline Core::UniquePtr<Red::TemplateAppearance> s_emptyAppearance;
    static inline Core::SharedPtr<DynamicAppearanceController> s_dynamicAppearance;
    static inline Core::UniquePtr<OverrideStateManager> s_stateManager;
    static inline Core::SharedPtr<OverrideTagManager> s_tagManager;
    static inline bool s_garmentOffsetsEnabled;
    static inline Red::SharedSpinLock s_mutex;
};
}
