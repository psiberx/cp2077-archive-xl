#pragma once

#include "Core/Raw.hpp"
#include "Red/Addresses.hpp"

namespace Red
{
using AppearanceDefinition = appearance::AppearanceDefinition;
using AppearancePartOverrides = appearance::AppearancePartOverrides;

struct GarmentItemAddRequest
{
    Handle<AppearanceDefinition> apperance; // 00
    uint64_t hash;                          // 10
    int32_t offset;                         // 18
};
RED4EXT_ASSERT_SIZE(GarmentItemAddRequest, 0x20);
RED4EXT_ASSERT_OFFSET(GarmentItemAddRequest, apperance, 0x00);
RED4EXT_ASSERT_OFFSET(GarmentItemAddRequest, hash, 0x10);

struct GarmentItemOverrideRequest
{
    Handle<AppearanceDefinition> apperance;      // 00
    DynArray<AppearancePartOverrides> overrides; // 10
    uint64_t hash;                               // 20
    int32_t offset;                              // 28
};
RED4EXT_ASSERT_SIZE(GarmentItemOverrideRequest, 0x30);
RED4EXT_ASSERT_OFFSET(GarmentItemOverrideRequest, apperance, 0x00);
RED4EXT_ASSERT_OFFSET(GarmentItemOverrideRequest, overrides, 0x10);
RED4EXT_ASSERT_OFFSET(GarmentItemOverrideRequest, hash, 0x20);

struct GarmentItemRemoveRequest
{
    uint64_t hash; // 00
};
RED4EXT_ASSERT_SIZE(GarmentItemRemoveRequest, 0x8);
RED4EXT_ASSERT_OFFSET(GarmentItemRemoveRequest, hash, 0x0);
}

namespace Raw::GarmentAssembler
{
constexpr auto AddItem = Core::RawFunc<
    /* addr = */ Red::Addresses::GarmentAssembler_AddItem,
    /* type = */ bool (*)(uintptr_t, Red::WeakHandle<Red::ent::Entity>&, Red::GarmentItemAddRequest&)>();

constexpr auto OverrideItem = Core::RawFunc<
    /* addr = */ Red::Addresses::GarmentAssembler_OverrideItem,
    /* type = */ bool (*)(uintptr_t, Red::WeakHandle<Red::ent::Entity>&, Red::GarmentItemOverrideRequest&)>();

constexpr auto RemoveItem = Core::RawFunc<
    /* addr = */ Red::Addresses::GarmentAssembler_RemoveItem,
    /* type = */ bool (*)(uintptr_t, Red::WeakHandle<Red::ent::Entity>&, Red::GarmentItemRemoveRequest&)>();

constexpr auto OnGameDetach = Core::RawFunc<
    /* addr = */ Red::Addresses::GarmentAssembler_OnGameDetach,
    /* type = */ void (*)(uintptr_t)>();
}
