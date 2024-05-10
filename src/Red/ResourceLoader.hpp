#pragma once

namespace Red
{
struct ResourceRequest
{
    ResourcePath path; // 00
    uint64_t unk08{};  // 08
    uint64_t unk10{};  // 10
    int32_t unk18{-1}; // 18
    int32_t unk1C{};   // 1C
    uint64_t unk20{};  // 20
};
RED4EXT_ASSERT_SIZE(ResourceRequest, 0x28);
RED4EXT_ASSERT_OFFSET(ResourceRequest, path, 0x0);
}

namespace Raw::ResourceLoader
{
constexpr auto RequestResource = Core::RawFunc<
    /* addr = */ Red::AddressLib::ResourceLoader_RequestResource,
    /* type = */ void (*)(Red::ResourceLoader* aLoader,
                          Red::SharedPtr<Red::ResourceToken<>>& aToken,
                          Red::ResourceRequest& aRequest)>();

constexpr auto OnUpdate = Core::RawFunc<
    /* addr = */ Red::AddressLib::ResourceLoader_OnUpdate,
    /* type = */ void (*)(Red::ResourceLoader* aLoader)>();
}
