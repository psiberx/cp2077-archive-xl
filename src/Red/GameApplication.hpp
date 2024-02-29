#pragma once

namespace Raw::GameApplication
{
constexpr auto InitResourceDepot = Core::RawFunc<
    /* addr = */ Red::AddressLib::GameApplication_InitResourceDepot,
    /* type = */ void (*)(uintptr_t aApp, uintptr_t aParams)>();
}
