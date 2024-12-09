#pragma once

namespace Red
{
enum class EntityStatus : uint8_t
{
    Undefined = 0,
    Initializing = 1,
    Detached = 2,
    Attaching = 3,
    Attached = 4,
    Detaching = 5,
    Uninitializing = 6,
    Uninitialized = 7,
};

struct EntityInitializeRequest
{
    WorldTransform transform;                                     // 00
    EntityID entityID;                                            // 20
    CName appearanceName;                                         // 28
    CName appearanceInternalName;                                 // 30
    bool isDynamic;                                               // 38
    void* unk40;                                                  // 40
    void* unk48;                                                  // 48
    void* unk50;                                                  // 50
    uint64_t unk58;                                               // 58
    uint64_t unk60;                                               // 60
    uint64_t unk68;                                               // 68
    Handle<ent::EntityParametersStorage> entityParams;            // 70
    DynArray<void*> unk80;                                        // 80
    Handle<population::PopulationSpawnParameter> populationParam; // 90
    uint64_t unkA0;                                               // A0
    uint64_t unkA8;                                               // A8
    net::PeerID controllingPeerID;                                // B0
    uint64_t unkB8;                                               // B8
    TweakDBID recordID;                                           // C0
    uint32_t unkC8;                                               // C8
    uint16_t unkCC;                                               // CC
};
RED4EXT_ASSERT_SIZE(EntityInitializeRequest, 0xD0);
RED4EXT_ASSERT_OFFSET(EntityInitializeRequest, entityID, 0x20);
RED4EXT_ASSERT_OFFSET(EntityInitializeRequest, entityParams, 0x70);
RED4EXT_ASSERT_OFFSET(EntityInitializeRequest, recordID, 0xC0);
RED4EXT_ASSERT_OFFSET(EntityInitializeRequest, unkCC, 0xCC);
}

namespace Raw::Entity
{
// using EntityID = Core::OffsetPtr<0x48, Red::EntityID>;
// using AppearanceName = Core::OffsetPtr<0x50, Red::CName>;
// using TemplatePath = Core::OffsetPtr<0x60, Red::ResourcePath>;
// using ComponentsStorage = Core::OffsetPtr<0x70, Red::ent::ComponentsStorage>;
using TransformComponent = Core::OffsetPtr<0xB0, Red::IPlacedComponent*>;
using Scene = Core::OffsetPtr<0xB8, Red::world::RuntimeScene>;
using VisualTags = Core::OffsetPtr<0x138, Red::TagList>;
using Status = Core::OffsetPtr<0x156, Red::EntityStatus>;
using EntityTags = Core::OffsetPtr<0x230, Red::TagList>;

constexpr auto Assemble = Core::RawFunc<
    /* addr = */ Red::AddressLib::Entity_Assemble,
    /* type = */ bool (*)(Red::Entity* aEntity, uintptr_t)>();

constexpr auto Reassemble = Core::RawFunc<
    /* addr = */ Red::AddressLib::Entity_Reassemble,
    /* type = */ void (*)(Red::Entity* aEntity, uintptr_t a2, Red::CName aNewAppearance, uint64_t a4,
                          Red::DynArray<Red::Handle<Red::IComponent>>& aNewComponents,
                          Red::Handle<Red::ent::EntityParametersStorage>& aEntityParams)>();

constexpr auto ReassembleAppearance = Core::RawFunc<
    /* addr = */ Red::AddressLib::Entity_Reassemble,
    /* type = */ void (*)(Red::Entity*, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>();

constexpr auto Initialize = Core::RawFunc<
    /* addr = */ Red::AddressLib::Entity_Initialize,
    /* type = */ void (*)(Red::Entity* aEntity, uintptr_t, Red::EntityInitializeRequest* aRequest)>();

constexpr auto RequestComponents = Core::RawFunc<
    /* addr = */ Red::AddressLib::Entity_RequestComponents,
    /* type = */ void (*)(Red::Entity* aEntity, uintptr_t a2,
                          Red::DynArray<Red::Handle<Red::IComponent>>* aComponents)>();

constexpr auto Uninitialize = Core::RawFunc<
    /* addr = */ Red::AddressLib::Entity_Uninitialize,
    /* type = */ void (*)(Red::Entity* aEntity)>();

constexpr auto Dispose = Core::RawFunc<
    /* addr = */ Red::AddressLib::Entity_Dispose,
    /* type = */ void (*)(Red::Entity* aEntity)>();

constexpr auto Attach = Core::RawFunc<
    /* addr = */ Red::AddressLib::Entity_Attach,
    /* type = */ void (*)(Red::Entity* aEntity, uintptr_t)>();

constexpr auto Detach = Core::RawFunc<
    /* addr = */ Red::AddressLib::Entity_Detach,
    /* type = */ void (*)(Red::Entity* aEntity)>();

// constexpr auto GetComponents = Core::RawFunc<
//     /* addr = */ Red::AddressLib::Entity_GetComponents,
//     /* type = */ Red::DynArray<Red::Handle<Red::IComponent>>& (*)(Red::Entity* aEntity)>();
}

namespace Raw::IComponent
{
// using AppearanceName = Core::OffsetPtr<0x48, Red::CName>;
// using Owner = Core::OffsetPtr<0x50, Red::Entity*>;
// using AppearancePath = Core::OffsetPtr<0x68, Red::ResourcePath>;

constexpr auto UpdateRenderer = Core::RawVFunc<
    /* offset = */ 0x280,
    /* type = */ void (Red::IComponent::*)()>();
}

namespace Raw::IPlacedComponent
{
constexpr auto SetTransform = Core::RawFunc<
    /* addr = */ Red::AddressLib::IPlacedComponent_SetTransform,
    /* type = */ void (*)(Red::IPlacedComponent* aComponent, const Red::WorldTransform& aTransform)>();
}
