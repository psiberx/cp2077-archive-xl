#pragma once

#include "Core/Raw.hpp"
#include "Red/Addresses.hpp"

namespace Red
{
struct GarmentComputeData
{
    uint64_t unk00;                                  // 00
    uint64_t unk08;                                  // 08
    DynArray<Handle<ent::IComponent>> components;    // 10
    DynArray<Handle<CMesh>> meshes;                  // 20
    DynArray<Handle<ent::EntityTemplate>> templates; // 30
    DynArray<ResourcePath> resources;                // 40
};
RED4EXT_ASSERT_OFFSET(GarmentComputeData, components, 0x10);
RED4EXT_ASSERT_OFFSET(GarmentComputeData, resources, 0x40);
}

namespace Raw::AppearanceChanger
{
constexpr auto ComputePlayerGarment = Core::RawFunc<
    /* addr = */ Red::Addresses::AppearanceChanger_ComputePlayerGarment,
    /* type = */ void (*)(Red::Handle<Red::ent::Entity>&, Red::DynArray<int32_t>&,
                          Red::SharedPtr<Red::GarmentComputeData>&, uintptr_t, uintptr_t, uintptr_t, bool)>();
}

namespace Raw::AppearanceChangeRequest
{
constexpr auto LoadEntityTemplate = Core::RawFunc<
    /* addr = */ Red::Addresses::AppearanceChangeRequest_LoadEntityTemplate,
    /* type = */ bool (*)(uintptr_t)>();
}
