#pragma once

namespace Raw::AttachmentSlots
{
constexpr auto InitializeSlots = Core::RawFunc<
    /* addr = */ Red::AddressLib::AttachmentSlots_InitializeSlots,
    /* type = */ void (*)(Red::game::AttachmentSlots*, Red::DynArray<Red::TweakDBID>&)>();

constexpr auto IsSlotEmpty = Core::RawFunc<
    /* addr = */ Red::AddressLib::AttachmentSlots_IsSlotEmpty,
    /* type = */ bool (*)(Red::game::AttachmentSlots*, Red::TweakDBID)>();

constexpr auto IsSlotSpawning = Core::RawFunc<
    /* addr = */ Red::AddressLib::AttachmentSlots_IsSlotSpawning,
    /* type = */ bool (*)(Red::game::AttachmentSlots*, Red::TweakDBID)>();

// Holds the slot lock (+0x100) exclusively while it rebuilds item appearances, which queries suffixes
constexpr auto LockedUpdate = Core::RawFunc<
    /* addr = */ Red::AddressLib::AttachmentSlots_LockedUpdate,
    /* type = */ uintptr_t (*)(void*, void*, void*, void*)>();
using LockedUpdatePtr = decltype(LockedUpdate)::Callable;
}
