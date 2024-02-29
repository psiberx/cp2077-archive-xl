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
}
