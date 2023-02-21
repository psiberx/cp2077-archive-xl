#pragma once

#include "Core/Raw.hpp"
#include "Red/Addresses.hpp"

namespace Raw::JobHandle
{
constexpr auto Wait = Core::RawFunc<
    /* addr = */ Red::Addresses::JobHandle_Wait,
    /* type = */ void (*)(Red::JobHandle& aJob)>();
}
