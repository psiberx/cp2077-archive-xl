#pragma once

#include "Red/Addresses.hpp"

namespace Raw::AnimatedComponent
{
constexpr auto InitializeAnimations = Core::RawFunc<
    /* addr = */ Red::Addresses::AnimatedComponent_InitializeAnimations,
    /* type = */ void (*)(Red::entAnimatedComponent* aComponent)>();
}
