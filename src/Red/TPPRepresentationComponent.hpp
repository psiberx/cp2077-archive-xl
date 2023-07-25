#pragma once

#include "Core/Raw.hpp"
#include "Red/Addresses.hpp"

namespace Raw::TPPRepresentationComponent
{
constexpr auto OnAttach = Core::RawFunc<
    /* addr = */ Red::Addresses::TPPRepresentationComponent_OnAttach,
    /* type = */ void (*)(Red::game::TPPRepresentationComponent* aComponent, uintptr_t a2)>();

using SlotListenerCallback = void (*)(Red::game::TPPRepresentationComponent* aComponent,
                                      Red::TweakDBID aItemID,
                                      Red::TweakDBID aSlotID);

constexpr auto OnItemEquipped = Core::RawFunc<
    /* addr = */ Red::Addresses::TPPRepresentationComponent_OnItemEquipped,
    /* type = */ SlotListenerCallback>();

constexpr auto OnItemUnequipped = Core::RawFunc<
    /* addr = */ Red::Addresses::TPPRepresentationComponent_OnItemUnequipped,
    /* type = */ SlotListenerCallback>();
}
