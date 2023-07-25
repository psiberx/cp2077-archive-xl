#pragma once

#include "App/Extensions/ModuleBase.hpp"
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

private:
    static void OnInitializeSlots(Red::game::AttachmentSlots* aComponent, Red::DynArray<Red::TweakDBID>& aSlotIDs);
    static bool OnSlotSpawningCheck(Red::game::AttachmentSlots* aComponent, Red::TweakDBID aSlotID);
    static void OnAttachTPP(Red::game::TPPRepresentationComponent* aComponent, uintptr_t);
    static void OnItemChangeTPP(Raw::TPPRepresentationComponent::SlotListenerCallback aCallback,
                                Red::game::TPPRepresentationComponent* aComponent,
                                Red::TweakDBID aItemID, Red::TweakDBID aSlotID);
    static void OnCheckFeetState(Red::game::ui::CharacterCustomizationFeetController* aComponent,
                                 Red::CharacterFeetState& aLiftedState,
                                 Red::CharacterFeetState& aFlatState);

    static inline Core::Map<Red::TweakDBID, Core::Set<Red::TweakDBID>> s_extraSlots;
    static inline Core::Map<Red::TweakDBID, Red::TweakDBID> s_baseSlots;
    static inline std::shared_mutex s_slotsMutex;
};
}
