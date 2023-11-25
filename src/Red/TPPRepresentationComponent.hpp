#pragma once

#include "Core/Raw.hpp"
#include "Red/Addresses.hpp"

namespace Raw::TPPRepresentationComponent
{
using SlotListener = Core::OffsetPtr<0x148, Red::Handle<Red::game::IAttachmentSlotsListener>>;

constexpr auto OnAttach = Core::RawFunc<
    /* addr = */ Red::Addresses::TPPRepresentationComponent_OnAttach,
    /* type = */ void (*)(Red::game::TPPRepresentationComponent* aComponent, uintptr_t a2)>();

constexpr auto RegisterAffectedItem = Core::RawFunc<
    /* addr = */ Red::Addresses::TPPRepresentationComponent_RegisterAffectedItem,
    /* type = */ void (*)(Red::game::TPPRepresentationComponent* aComponent,
                          Red::TweakDBID aItemID,
                          const Red::Handle<Red::ItemObject>& aItemObject)>();

constexpr auto IsAffectedSlot = Core::RawFunc<
    /* addr = */ Red::Addresses::TPPRepresentationComponent_IsAffectedSlot,
    /* type = */ bool (*)(Red::TweakDBID aSlotID)>();
}
