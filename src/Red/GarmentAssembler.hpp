#pragma once

#include "Core/Raw.hpp"
#include "Red/Addresses.hpp"

namespace Red
{
using EntityTemplate = ent::EntityTemplate;
using AppearanceDefinition = appearance::AppearanceDefinition;
using AppearancePartOverrides = appearance::AppearancePartOverrides;

struct GarmentProcessor {};

struct GarmentItemAddRequest
{
    Handle<AppearanceDefinition> apperance; // 00
    uint64_t hash;                          // 10
    int32_t offset;                         // 18
};
RED4EXT_ASSERT_SIZE(GarmentItemAddRequest, 0x20);
RED4EXT_ASSERT_OFFSET(GarmentItemAddRequest, apperance, 0x00);
RED4EXT_ASSERT_OFFSET(GarmentItemAddRequest, hash, 0x10);
RED4EXT_ASSERT_OFFSET(GarmentItemAddRequest, offset, 0x18);

struct GarmentItemAddCustomRequest
{
    Handle<AppearanceDefinition> apperance;      // 00
    DynArray<AppearancePartOverrides> overrides; // 10
    uint64_t hash;                               // 20
    int32_t offset;                              // 28
};
RED4EXT_ASSERT_SIZE(GarmentItemAddCustomRequest, 0x30);
RED4EXT_ASSERT_OFFSET(GarmentItemAddCustomRequest, apperance, 0x00);
RED4EXT_ASSERT_OFFSET(GarmentItemAddCustomRequest, overrides, 0x10);
RED4EXT_ASSERT_OFFSET(GarmentItemAddCustomRequest, hash, 0x20);
RED4EXT_ASSERT_OFFSET(GarmentItemAddCustomRequest, offset, 0x28);

struct GarmentItemChangeRequest
{
    Handle<AppearanceDefinition> apperance; // 00
    uint64_t hash;                          // 10
};
RED4EXT_ASSERT_SIZE(GarmentItemChangeRequest, 0x18);
RED4EXT_ASSERT_OFFSET(GarmentItemChangeRequest, apperance, 0x00);
RED4EXT_ASSERT_OFFSET(GarmentItemChangeRequest, hash, 0x10);

struct GarmentItemChangeCustomRequest
{
    Handle<AppearanceDefinition> apperance;      // 00
    DynArray<AppearancePartOverrides> overrides; // 10
    uint64_t hash;                               // 20
};
RED4EXT_ASSERT_SIZE(GarmentItemChangeCustomRequest, 0x28);
RED4EXT_ASSERT_OFFSET(GarmentItemChangeCustomRequest, apperance, 0x00);
RED4EXT_ASSERT_OFFSET(GarmentItemChangeCustomRequest, overrides, 0x10);
RED4EXT_ASSERT_OFFSET(GarmentItemChangeCustomRequest, hash, 0x20);

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

constexpr auto AddCustomItem = Core::RawFunc<
    /* addr = */ Red::Addresses::GarmentAssembler_AddCustomItem,
    /* type = */ bool (*)(uintptr_t, Red::WeakHandle<Red::ent::Entity>&, Red::GarmentItemAddCustomRequest&)>();

constexpr auto ChangeItem = Core::RawFunc<
    /* addr = */ Red::Addresses::GarmentAssembler_ChangeItem,
    /* type = */ bool (*)(uintptr_t, Red::WeakHandle<Red::ent::Entity>&, Red::GarmentItemChangeRequest&)>();

constexpr auto ChangeCustomItem = Core::RawFunc<
    /* addr = */ Red::Addresses::GarmentAssembler_ChangeCustomItem,
    /* type = */ bool (*)(uintptr_t, Red::WeakHandle<Red::ent::Entity>&, Red::GarmentItemChangeCustomRequest&)>();

constexpr auto RemoveItem = Core::RawFunc<
    /* addr = */ Red::Addresses::GarmentAssembler_RemoveItem,
    /* type = */ bool (*)(uintptr_t, Red::WeakHandle<Red::ent::Entity>&, Red::GarmentItemRemoveRequest&)>();

constexpr auto ProcessGarment = Core::RawFunc<
    /* addr = */ Red::Addresses::GarmentAssembler_ProcessGarment,
    /* type = */ uintptr_t (*)(Red::SharedPtr<Red::GarmentProcessor>& aProcessor,
                               uintptr_t a2, uintptr_t a3,
                               Red::Handle<Red::Entity>& aEntity)>();

constexpr auto ProcessSkinnedMesh = Core::RawFunc<
    /* addr = */ Red::Addresses::GarmentAssembler_ProcessSkinnedMesh,
    /* type = */ void (*)(Red::GarmentProcessor* aProcessor,
                          uint32_t aIndex,
                          Red::Handle<Red::EntityTemplate>& aPartTemplate,
                          Red::SharedPtr<Red::ResourceToken<Red::CMesh>>& aMeshToken,
                          Red::Handle<Red::IComponent>& aComponent,
                          Red::JobGroup& aJobGroup)>();

constexpr auto ProcessMorphedMesh = Core::RawFunc<
    /* addr = */ Red::Addresses::GarmentAssembler_ProcessMorphedMesh,
    /* type = */ void (*)(Red::GarmentProcessor* aProcessor,
                          uint32_t aIndex,
                          Red::Handle<Red::EntityTemplate>& aPartTemplate,
                          Red::SharedPtr<Red::ResourceToken<Red::CMesh>>& aMeshToken,
                          Red::Handle<Red::IComponent>& aComponent,
                          Red::JobGroup& aJobGroup)>();

constexpr auto OnGameDetach = Core::RawFunc<
    /* addr = */ Red::Addresses::GarmentAssembler_OnGameDetach,
    /* type = */ void (*)(uintptr_t)>();
}
