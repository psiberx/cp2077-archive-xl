#pragma once

#include "Core/Raw.hpp"
#include "Red/Addresses.hpp"

namespace Raw::TPPRepresentationComponent
{
constexpr auto OnInitialize = Core::RawFunc<
    /* addr = */ Red::Addresses::TPPRepresentationComponent_OnInitialize,
    /* type = */ void (*)(Red::game::TPPRepresentationComponent* aComponent)>();
}
