#pragma once

#include "Red/Addresses.hpp"
#include "Red/Common.hpp"
#include "Red/EntityTemplate.hpp"

namespace Red
{
using EntityTemplate = Red::ent::EntityTemplate;
using ComponentsStorage = Red::ent::ComponentsStorage;
using AppearanceResource = Red::appearance::AppearanceResource;
using AppearanceDefinition = Red::appearance::AppearanceDefinition;

template<typename T>
using ResourceTokenPtr = Red::SharedPtr<Red::ResourceToken<T>>;

struct ItemFactoryRequest {};

struct ItemFactoryAppearanceChangeRequest {};

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
}

namespace Raw::AppearanceChanger
{
constexpr auto GetSuffixValue = Core::RawFunc<
    /* addr = */ Red::Addresses::AppearanceChanger_GetSuffixValue,
    /* type = */ bool (*)(Red::ItemID aItemID,
                          uint64_t a2, // Must be non-zero
                          Red::Handle<Red::GameObject>& aOwner,
                          Red::TweakDBID aSuffixRecordID,
                          Red::CString& aResult)>();
using GetSuffixValuePtr = decltype(GetSuffixValue)::Callable;

constexpr auto RegisterPart = Core::RawFunc<
    /* addr = */ Red::Addresses::AppearanceChanger_RegisterPart,
    /* type = */ void (*)(uintptr_t,
                          Red::Handle<Red::EntityTemplate>& aPart,
                          Red::Handle<Red::ComponentsStorage>& aComponents,
                          Red::Handle<Red::AppearanceDefinition>& aAppearance)>();

constexpr auto GetBaseMeshOffset = Core::RawFunc<
    /* addr = */ Red::Addresses::AppearanceChanger_GetBaseMeshOffset,
    /* type = */ int64_t (*)(Red::Handle<Red::IComponent>& aComponent,
                             Red::Handle<Red::EntityTemplate>& aTemplate)>();

constexpr auto ComputePlayerGarment = Core::RawFunc<
    /* addr = */ Red::Addresses::AppearanceChanger_ComputePlayerGarment,
    /* type = */ void (*)(uintptr_t a1,
                          Red::Handle<Red::ent::Entity>& aEntity,
                          Red::DynArray<int32_t>& aOffsets,
                          Red::SharedPtr<Red::GarmentComputeData>& aData,
                          uintptr_t a5,
                          uintptr_t a6,
                          uintptr_t a7,
                          bool a8)>();

constexpr auto SelectAppearanceName = Core::RawFunc<
    /* addr = */ Red::Addresses::AppearanceChanger_SelectAppearanceName,
    /* type = */ void* (*)(Red::CName* aOut,
                           const Red::Handle<Red::TweakDBRecord>& aItemRecord,
                           const Red::ItemID& aItemID,
                           const Red::Handle<Red::AppearanceResource>& aAppearanceResource,
                           uint64_t a5,
                           Red::CName aAppearanceName)>();
}

namespace Raw::ItemFactoryRequest
{
using Entity = Core::OffsetPtr<0x28, Red::WeakHandle<Red::Entity>>;
using EntityTemplate = Core::OffsetPtr<0x138, Red::ResourceTokenPtr<Red::EntityTemplate>>;
using AppearanceToken = Core::OffsetPtr<0x148, Red::ResourceTokenPtr<Red::AppearanceResource>>;
using AppearanceName = Core::OffsetPtr<0x158, Red::CName>;
using ItemRecord = Core::OffsetPtr<0x160, Red::gamedataTweakDBRecord*>;

constexpr auto LoadAppearance = Core::RawFunc<
    /* addr = */ Red::Addresses::ItemFactoryRequest_LoadAppearance,
    /* type = */ bool (*)(Red::ItemFactoryRequest* aRequest)>();

// constexpr auto ApplyAppearance = Core::RawFunc<
//     /* addr = */ Red::Addresses::ItemFactoryRequest_ApplyAppearance,
//     /* type = */ bool (*)(Red::ItemFactoryRequest* aRequest)>();
};

namespace Raw::ItemFactoryAppearanceChangeRequest
{
using Entity = Core::OffsetPtr<0x80, Red::WeakHandle<Red::Entity>>;
using EntityTemplate = Core::OffsetPtr<0xA8, Red::ResourceTokenPtr<Red::EntityTemplate>>;
using AppearanceToken = Core::OffsetPtr<0xB8, Red::ResourceTokenPtr<Red::AppearanceResource>>;
using AppearanceName = Core::OffsetPtr<0x48, Red::CName>;
using ItemRecord = Core::OffsetPtr<0x100, Red::gamedataTweakDBRecord*>;

constexpr auto LoadTemplate = Core::RawFunc<
    /* addr = */ Red::Addresses::ItemFactoryAppearanceChangeRequest_LoadTemplate,
    /* type = */ bool (*)(Red::ItemFactoryAppearanceChangeRequest* aRequest)>();

constexpr auto LoadAppearance = Core::RawFunc<
    /* addr = */ Red::Addresses::ItemFactoryAppearanceChangeRequest_LoadAppearance,
    /* type = */ bool (*)(Red::ItemFactoryAppearanceChangeRequest* aRequest)>();

// constexpr auto ApplyAppearance = Core::RawFunc<
//     /* addr = */ Red::Addresses::ItemFactoryAppearanceChangeRequest_ApplyAppearance,
//     /* type = */ bool (*)(Red::ItemFactoryAppearanceChangeRequest* aRequest)>();
}

namespace Raw::RuntimeSystemEntityAppearanceChanger
{
constexpr auto ChangeAppearance = Core::RawFunc<
    /* addr = */ Red::Addresses::AppearanceChangeSystem_ChangeAppearance,
    /* type = */ void (*)(Red::world::RuntimeSystemEntityAppearanceChanger& aSystem,
                          Red::Handle<Red::game::Puppet>& aTarget,
                          Red::Range<Red::AppearanceDescriptor>& aOldApp,
                          Red::Range<Red::AppearanceDescriptor>& aNewApp,
                          uintptr_t a5,
                          uint8_t a6)>();
}
