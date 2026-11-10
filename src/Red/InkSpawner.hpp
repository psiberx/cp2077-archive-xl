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
    // uint8_t status;                               // 1AC
};
RED4EXT_ASSERT_OFFSET(InkSpawningRequest, itemName, 0x48);
RED4EXT_ASSERT_OFFSET(InkSpawningRequest, rootWidget, 0x60);

struct InkSpawningContext
{
    WeakHandle<ISerializable> unk00;       // 00
    SharedPtr<InkSpawningRequest> request; // 10
};
RED4EXT_ASSERT_SIZE(InkSpawningContext, 0x20);
RED4EXT_ASSERT_OFFSET(InkSpawningContext, request, 0x10);

struct InkSpawningInfo
{
    uint8_t unk00[0x38];         // 00
    InkSpawningContext* context; // 38
};
RED4EXT_ASSERT_SIZE(InkSpawningInfo, 0x40);
RED4EXT_ASSERT_OFFSET(InkSpawningInfo, context, 0x38);
}

namespace Raw::InkSpawner
{
using ControllerInjectionEnabled = Core::RawPtr<Red::AddressLib::InkSpawner_ControllerInjectionEnabled, bool>;

constexpr auto FinishAsyncSpawn = Core::RawFunc<
    /* addr = */ Red::AddressLib::InkSpawner_FinishAsyncSpawn,
    /* type = */ bool (*)(Red::InkSpawningContext& aContext,
                          Red::Handle<Red::inkWidgetLibraryItemInstance>& aInstance)>();
}
