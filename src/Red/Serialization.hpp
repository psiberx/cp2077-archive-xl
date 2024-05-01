#pragma once

namespace Red
{
struct ResourceSerializerRequest
{
    uint64_t unk00;         // 00
    uint64_t unk08;         // 08
    ResourceLoader* loader; // 10
    uint64_t unk18;         // 18
    uint64_t unk20;         // 20
    ResourcePath path;      // 28
    ResourcePath unk30;     // 30 - Same as path
    uint8_t unk38;          // 38
    uint64_t unk40;         // 40
    uint64_t unk48;         // 48
    uint64_t unk50;         // 50
    uint8_t flags;          // 58
};
RED4EXT_ASSERT_SIZE(ResourceSerializerRequest, 0x60);
RED4EXT_ASSERT_OFFSET(ResourceSerializerRequest, path, 0x28);
RED4EXT_ASSERT_OFFSET(ResourceSerializerRequest, flags, 0x58);
}

namespace Raw::ResourceSerializer
{
constexpr auto Deserialize = Core::RawFunc<
    /* addr = */ Red::AddressLib::ResourceSerializer_Deserialize,
    /* type = */ void (*)(void* aSerializer, uint64_t a2, uint64_t a3,
                          Red::JobHandle& aJob, Red::ResourceSerializerRequest& aRequest, uint64_t a6,
                          Red::DynArray<Red::Handle<Red::ISerializable>>& aResults, uint64_t a8)>();
}
