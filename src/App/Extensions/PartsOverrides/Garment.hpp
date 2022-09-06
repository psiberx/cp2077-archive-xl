#pragma once

#include <RED4ext/Common.hpp>
#include <RED4ext/Scripting/Natives/Generated/appearance/AppearanceDefinition.hpp>
#include <RED4ext/Scripting/Natives/Generated/appearance/AppearancePartOverrides.hpp>
#include <RED4ext/Scripting/Natives/Generated/ent/IComponent.hpp>

namespace App
{
using ItemHash = uint64_t;
using ItemAppearance = RED4ext::Handle<RED4ext::appearance::AppearanceDefinition>;
using ItemOverrides = RED4ext::DynArray<RED4ext::appearance::AppearancePartOverrides>;
using ItemComponents = RED4ext::DynArray<RED4ext::Handle<RED4ext::ent::IComponent>>;

struct GarmentItemAddRequest
{
    ItemAppearance apperance; // 00
    ItemHash hash;            // 10
    int32_t unk18;            // 18
};
RED4EXT_ASSERT_SIZE(GarmentItemAddRequest, 0x20);
RED4EXT_ASSERT_OFFSET(GarmentItemAddRequest, apperance, 0x00);
RED4EXT_ASSERT_OFFSET(GarmentItemAddRequest, hash, 0x10);

struct GarmentItemOverrideRequest
{
    ItemAppearance apperance; // 00
    ItemOverrides overrides;  // 10
    ItemHash hash;            // 20
    int32_t unk28;            // 28
};
RED4EXT_ASSERT_SIZE(GarmentItemOverrideRequest, 0x30);
RED4EXT_ASSERT_OFFSET(GarmentItemOverrideRequest, apperance, 0x00);
RED4EXT_ASSERT_OFFSET(GarmentItemOverrideRequest, hash, 0x20);

struct GarmentItemRemoveRequest
{
    ItemHash hash; // 00
};
RED4EXT_ASSERT_SIZE(GarmentItemRemoveRequest, 0x8);
RED4EXT_ASSERT_OFFSET(GarmentItemRemoveRequest, hash, 0x0);

struct GarmentComputeData
{
    uint64_t unk00;            // 00
    uint64_t unk08;            // 08
    ItemComponents components; // 10
};
RED4EXT_ASSERT_OFFSET(GarmentComputeData, components, 0x10);
}
