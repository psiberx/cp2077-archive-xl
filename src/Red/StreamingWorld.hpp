#pragma once

namespace Raw::StreamingWorld
{
constexpr auto Serialize = Core::RawFunc<
    /* addr = */ Red::AddressLib::StreamingWorld_Serialize,
    /* type = */ void (*)(Red::worldStreamingWorld* aWorld, Red::BaseStream* aStream)>();
}

namespace Raw::RuntimeSystemWorldStreaming
{
using StreamingWorld = Core::OffsetPtr<0x270, Red::Handle<Red::worldStreamingWorld>>;
}
