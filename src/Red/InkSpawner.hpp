#pragma once

namespace Red
{
struct InkSpawningRequest
{
    uint8_t unk00[0x48];                             // 00
    CName itemName;                                  // 48
    WeakHandle<ink::Widget> parentWidget;            // 50
    Handle<ink::Widget> rootWidget;                  // 60
    Handle<ink::IWidgetController> gameController;   // 70
    Handle<ink::WidgetLibraryResource> library;      // 80
    Handle<ink::WidgetLibraryItemInstance> instance; // 90
    ResourcePath externalLibrary;                    // A0
    // bool flag;                                    // F8
    // uint8_t status;                               // 184
};
RED4EXT_ASSERT_OFFSET(InkSpawningRequest, itemName, 0x48);
RED4EXT_ASSERT_OFFSET(InkSpawningRequest, rootWidget, 0x60);

struct InkSpawningContext
{
    virtual void sub_00() = 0;
    virtual void sub_08() = 0;
    virtual void sub_10() = 0;
    virtual void sub_18() = 0;
    virtual void sub_20() = 0;
    virtual void sub_28() = 0;

    SharedPtr<void>               unk08;   // 08
    SharedPtr<InkSpawningRequest> request; // 18
};
RED4EXT_ASSERT_SIZE(InkSpawningContext, 0x28);
RED4EXT_ASSERT_OFFSET(InkSpawningContext, request, 0x18);

struct InkSpawningInfo
{
    uint8_t unk00[0x38];         // 00
    InkSpawningContext* context; // 38
};
RED4EXT_ASSERT_SIZE(InkSpawningInfo, 0x40);
RED4EXT_ASSERT_OFFSET(InkSpawningInfo, context, 0x38);
}

namespace Raw::InkWidgetLibrary
{
constexpr auto AsyncSpawnFromExternal = Core::RawFunc<
    /* addr = */ Red::AddressLib::InkWidgetLibrary_AsyncSpawnFromExternal,
    /* type = */ bool (*)(
        Red::ink::WidgetLibraryResource& aLibrary,
        Red::InkSpawningInfo& aSpawningInfo,
        Red::ResourcePath aExternalPath,
        Red::CName aItemName)>();

constexpr auto AsyncSpawnFromLocal = Core::RawFunc<
    /* addr = */ Red::AddressLib::InkWidgetLibrary_AsyncSpawnFromLocal,
    /* type = */ bool (*)(
        Red::ink::WidgetLibraryResource& aLibrary,
        Red::InkSpawningInfo& aSpawningInfo,
        Red::CName aItemName)>();

constexpr auto SpawnFromExternal = Core::RawFunc<
    /* addr = */ Red::AddressLib::InkWidgetLibrary_SpawnFromExternal,
    /* type = */ uintptr_t (*)(
        Red::ink::WidgetLibraryResource& aLibrary,
        Red::Handle<Red::ink::WidgetLibraryItemInstance>& aInstance,
        Red::ResourcePath aExternalPath,
        Red::CName aItemName)>();

constexpr auto SpawnFromLocal = Core::RawFunc<
    /* addr = */ Red::AddressLib::InkWidgetLibrary_SpawnFromLocal,
    /* type = */ uintptr_t (*)(
        Red::ink::WidgetLibraryResource& aLibrary,
        Red::Handle<Red::ink::WidgetLibraryItemInstance>& aInstance,
        Red::CName aItemName)>();
}

namespace Raw::InkSpawner
{
constexpr auto FinishAsyncSpawn = Core::RawFunc<
    /* addr = */ Red::AddressLib::InkSpawner_FinishAsyncSpawn,
    /* type = */ bool (*)(
        Red::InkSpawningContext& aContext,
        Red::Handle<Red::ink::WidgetLibraryItemInstance>& aInstance)>();
}
