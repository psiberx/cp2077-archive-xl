#pragma once

#include "App/Extensions/ModuleBase.hpp"
#include "Red/AppearanceChanger.hpp"
#include "Red/AttachmentSlots.hpp"
#include "Red/CharacterCustomization.hpp"
#include "Red/TPPRepresentationComponent.hpp"

namespace App
{
class AttachmentSlotsModule : public Module
{
public:
    bool Load() override;
    bool Unload() override;
    std::string_view GetName() override;

    static Core::Set<Red::TweakDBID> GetExtraSlots(Red::TweakDBID aBaseSlotID);
    static Core::Set<Red::TweakDBID> GetRelatedSlots(Red::TweakDBID aBaseSlotID);
    static Core::Set<Red::TweakDBID> GetDependentSlots(Red::TweakDBID aBaseSlotID);

private:
    static void OnInitializeSlots(Red::game::AttachmentSlots* aComponent, Red::DynArray<Red::TweakDBID>& aSlotIDs);
    static bool OnSlotSpawningCheck(Red::game::AttachmentSlots* aComponent, Red::TweakDBID aSlotID);
    static void OnAttachTPP(Red::game::TPPRepresentationComponent* aComponent, uintptr_t);
    static void OnItemChangeTPP(Raw::TPPRepresentationComponent::SlotListenerCallback aCallback,
                                Red::game::TPPRepresentationComponent* aComponent,
                                Red::TweakDBID aItemID, Red::TweakDBID aSlotID);
    static void OnCheckHairState(Red::game::ui::CharacterCustomizationHairstyleController* aComponent,
                                 Red::CharacterBodyPartState& aHairState);
    static void OnCheckFeetState(Red::game::ui::CharacterCustomizationFeetController* aComponent,
                                 Red::CharacterBodyPartState& aLiftedState,
                                 Red::CharacterBodyPartState& aFlatState);
    static bool OnGetSuffixValue(Raw::AppearanceChanger::GetSuffixValuePtr aOriginalFunc,
                                 Red::ItemID aItemID, uint64_t a2, Red::Handle<Red::GameObject>& aOwner,
                                 Red::TweakDBID aSuffixRecordID, Red::CString& aResult);

    static bool IsVisualTagActive(Red::Handle<Red::Entity>& aOwner, Red::TweakDBID aBaseSlotID, Red::CName aVisualTag);
    static bool IsVisualTagActive(Red::ITransactionSystem* aTransactionSystem, Red::Handle<Red::Entity>& aOwner,
                                  Red::TweakDBID aSlotID, Red::CName aVisualTag);

    static inline thread_local bool s_skipVisualTagCheck = false;
    static inline Core::Map<Red::TweakDBID, Core::Set<Red::TweakDBID>> s_dependentSlots;
    static inline Core::Map<Red::TweakDBID, Core::Set<Red::TweakDBID>> s_extraSlots;
    static inline Core::Map<Red::TweakDBID, Red::TweakDBID> s_baseSlots;
    static inline std::shared_mutex s_slotsMutex;
};
}
