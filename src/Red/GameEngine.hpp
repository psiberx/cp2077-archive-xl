#pragma once

#include "Core/Raw.hpp"
#include "Red/Addresses.hpp"

namespace Raw::CBaseEngine
{
constexpr auto LoadGatheredResources = Core::RawFunc<
    /* addr = */ Red::Addresses::CBaseEngine_LoadGatheredResources,
    /* type = */ bool (*)(Red::CBaseEngine& aEngine)>();
}
