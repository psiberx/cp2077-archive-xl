#pragma once

namespace Raw::TPPRepresentationComponent
{
using SlotListener = Core::OffsetPtr<0x148, Red::Handle<Red::game::IAttachmentSlotsListener>>;

constexpr auto OnAttach = Core::RawFunc<
    /* addr = */ Red::AddressLib::TPPRepresentationComponent_OnAttach,
    /* type = */ void (*)(Red::game::TPPRepresentationComponent* aComponent, uintptr_t a2)>();

constexpr auto RegisterAffectedItem = Core::RawFunc<
    /* addr = */ Red::AddressLib::TPPRepresentationComponent_RegisterAffectedItem,
    /* type = */ void (*)(Red::game::TPPRepresentationComponent* aComponent,
                          Red::TweakDBID aItemID,
                          const Red::Handle<Red::ItemObject>& aItemObject)>();

constexpr auto IsAffectedSlot = Core::RawFunc<
    /* addr = */ Red::AddressLib::TPPRepresentationComponent_IsAffectedSlot,
    /* type = */ bool (*)(Red::TweakDBID aSlotID)>();
}
