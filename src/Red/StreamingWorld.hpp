#pragma once

#include "Core/Raw.hpp"
#include "Red/Addresses.hpp"

namespace Raw::StreamingWorld
{
constexpr auto OnLoad = Core::RawFunc<
    /* addr = */ Red::Addresses::StreamingWorld_OnLoad,
    /* type = */ void (*)(Red::world::StreamingWorld* aWorld, Red::BaseStream* aStream)>();
}
