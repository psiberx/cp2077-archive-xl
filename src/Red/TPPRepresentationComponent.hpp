#pragma once

#include "Core/Raw.hpp"
#include "Red/Addresses.hpp"

namespace Raw::TPPRepresentationComponent
{
using SlotListener = Core::OffsetPtr<0x148, Red::Handle<Red::game::IAttachmentSlotsListener>>;

constexpr auto OnAttach = Core::RawFunc<
    /* addr = */ Red::Addresses::TPPRepresentationComponent_OnAttach,
    /* type = */ void (*)(Red::game::TPPRepresentationComponent* aComponent, uintptr_t a2)>();

// using SlotListenerCallback = void (*)(Red::game::TPPRepresentationComponent* aComponent,
//                                       Red::TweakDBID aItemID,
//                                       Red::TweakDBID aSlotID);
//
// constexpr auto OnItemEquipped = Core::RawFunc<
//     /* addr = */ Red::Addresses::TPPRepresentationComponent_OnItemEquipped,
//     /* type = */ SlotListenerCallback>();
//
// constexpr auto OnItemUnequipped = Core::RawFunc<
//     /* addr = */ Red::Addresses::TPPRepresentationComponent_OnItemUnequipped,
//     /* type = */ SlotListenerCallback>();

constexpr auto IsAffectedSlot = Core::RawFunc<
    /* addr = */ Red::Addresses::TPPRepresentationComponent_IsAffectedSlot,
    /* type = */ bool (*)(Red::TweakDBID aSlotID)>();
}
