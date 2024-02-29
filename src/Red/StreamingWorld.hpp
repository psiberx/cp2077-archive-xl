#pragma once

namespace Raw::StreamingWorld
{
constexpr auto OnLoad = Core::RawFunc<
    /* addr = */ Red::AddressLib::StreamingWorld_OnLoad,
    /* type = */ void (*)(Red::world::StreamingWorld* aWorld, Red::BaseStream* aStream)>();
}
