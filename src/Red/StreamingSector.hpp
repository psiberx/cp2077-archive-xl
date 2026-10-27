#pragma once

#include "Red/Common.hpp"

namespace Raw::StreamingSector
{
constexpr auto PostLoad = Core::RawFunc<
    /* addr = */ Red::AddressLib::StreamingSector_PostLoad,
    /* type = */ void (*)(Red::worldStreamingSector* aSector, uint64_t a2)>();
}
