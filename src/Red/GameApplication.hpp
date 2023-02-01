#pragma once

#include "Core/Raw.hpp"
#include "Red/Addresses.hpp"

namespace Raw::GameApplication
{
constexpr auto InitResourceDepot = Core::RawFunc<
    /* addr = */ Red::Addresses::GameApplication_InitResourceDepot,
    /* type = */ void (*)(uintptr_t aApp, uintptr_t aParams)>();
}
