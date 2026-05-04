#pragma once

namespace Red
{
struct WorldLoadJobData
{
    worldRuntimeSystemWorldStreaming* system;                 // 00
    SharedPtr<ResourceToken<Red::worldStreamingWorld>> token; // 08
};
}

namespace Raw::StreamingWorld
{
constexpr auto Serialize = Core::RawFunc<
    /* addr = */ Red::AddressLib::StreamingWorld_Serialize,
    /* type = */ void (*)(Red::worldStreamingWorld* aWorld, Red::BaseStream* aStream)>();
}

namespace Raw::RuntimeSystemWorldStreaming
{
using StreamingWorld = Core::OffsetPtr<0x270, Red::Handle<Red::worldStreamingWorld>>;

constexpr auto LoadWorldJob = Core::RawFunc<
    /* addr = */ Red::AddressLib::RuntimeSystemWorldStreaming_LoadWorldJob,
    /* type = */ void (*)(Red::WorldLoadJobData* aData, const Red::JobGroup& aJobGroup)>();
}
