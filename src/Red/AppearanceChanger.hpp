#pragma once

#include "Core/Raw.hpp"
#include "Red/Addresses.hpp"
#include "Red/EntityTemplate.hpp"

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

struct AppearanceDescriptor
{
    using Hash = uint64_t;

    operator Hash() const noexcept
    {
        return Red::FNV1a64(reinterpret_cast<const uint8_t*>(&appearance.hash), sizeof(appearance.hash), resource.hash);
    }

    ResourcePath resource; // 00
    CName appearance;      // 08
};
RED4EXT_ASSERT_SIZE(AppearanceDescriptor, 0x10);
RED4EXT_ASSERT_OFFSET(AppearanceDescriptor, resource, 0x0);
RED4EXT_ASSERT_OFFSET(AppearanceDescriptor, appearance, 0x8);

template<typename T>
struct SomeIterator
{
    constexpr operator bool() const noexcept
    {
        return ptr != end;
    }

    constexpr operator T*() const noexcept
    {
        return ptr;
    }

    constexpr operator T&() const noexcept
    {
        return *ptr;
    }

    T* ptr; // 00
    T* end; // 08
};
RED4EXT_ASSERT_SIZE(SomeIterator<AppearanceDescriptor>, 0x10);
RED4EXT_ASSERT_OFFSET(SomeIterator<AppearanceDescriptor>, ptr, 0x0);
RED4EXT_ASSERT_OFFSET(SomeIterator<AppearanceDescriptor>, end, 0x8);
}

namespace Raw::AppearanceChanger
{
constexpr auto GetBaseMeshOffset = Core::RawFunc<
    /* addr = */ Red::Addresses::AppearanceChanger_GetBaseMeshOffset,
    /* type = */ int64_t (*)(Red::Handle<Red::IComponent>& aComponent,
                             Red::Handle<Red::EntityTemplate>& aTemplate)>();

constexpr auto ComputePlayerGarment = Core::RawFunc<
    /* addr = */ Red::Addresses::AppearanceChanger_ComputePlayerGarment,
    /* type = */ void (*)(Red::Handle<Red::ent::Entity>& aEntity,
                          Red::DynArray<int32_t>& aOffsets,
                          Red::SharedPtr<Red::GarmentComputeData>& aData,
                          uintptr_t a4,
                          uintptr_t a5,
                          uintptr_t a6,
                          bool a7)>();
}

namespace Raw::AppearanceChangeRequest
{
constexpr auto LoadEntityTemplate = Core::RawFunc<
    /* addr = */ Red::Addresses::AppearanceChangeRequest_LoadEntityTemplate,
    /* type = */ bool (*)(uintptr_t aRequest)>();
}

namespace Raw::RuntimeSystemEntityAppearanceChanger
{
constexpr auto ChangeAppearance = Core::RawFunc<
    /* addr = */ Red::Addresses::AppearanceChangeSystem_ChangeAppearance,
    /* type = */ void (*)(Red::world::RuntimeSystemEntityAppearanceChanger& aSystem,
                          Red::Handle<Red::game::Puppet>& aTarget,
                          Red::SomeIterator<Red::AppearanceDescriptor>& aOldApp,
                          Red::SomeIterator<Red::AppearanceDescriptor>& aNewApp,
                          uintptr_t a5,
                          uint8_t a6)>();
}
