#pragma once

#include "Core/Raw.hpp"
#include "Red/Addresses.hpp"

namespace Raw::TPPRepresentationComponent
{
constexpr auto OnAttach = Core::RawFunc<
    /* addr = */ Red::Addresses::TPPRepresentationComponent_OnAttach,
    /* type = */ void (*)(Red::game::TPPRepresentationComponent* aComponent, uintptr_t a2)>();
}
