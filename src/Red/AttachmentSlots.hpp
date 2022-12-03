#pragma once

#include "Core/Raw.hpp"
#include "Red/Addresses.hpp"

namespace Raw::AttachmentSlots
{
constexpr auto InitializeSlots = Core::RawFunc<
    /* addr = */ Red::Addresses::AttachmentSlots_InitializeSlots,
    /* type = */ void (*)(Red::game::AttachmentSlots*, Red::DynArray<Red::TweakDBID>&)>();

constexpr auto IsSlotEmpty = Core::RawFunc<
    /* addr = */ Red::Addresses::AttachmentSlots_IsSlotEmpty,
    /* type = */ bool (*)(Red::game::AttachmentSlots*, Red::TweakDBID)>();

constexpr auto IsSlotSpawning = Core::RawFunc<
    /* addr = */ Red::Addresses::AttachmentSlots_IsSlotSpawning,
    /* type = */ bool (*)(Red::game::AttachmentSlots*, Red::TweakDBID)>();
}
