#pragma once

namespace Raw::ImpostorComponent
{
constexpr auto OnAttach = Core::RawFunc<
    /* addr = */ Red::AddressLib::ImpostorComponent_OnAttach,
    /* type = */ void (*)(Red::gameImpostorComponent* aComponent, uintptr_t a2)>();
}
