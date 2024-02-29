#pragma once

namespace Raw::AnimatedComponent
{
constexpr auto InitializeAnimations = Core::RawFunc<
    /* addr = */ Red::AddressLib::AnimatedComponent_InitializeAnimations,
    /* type = */ void (*)(Red::entAnimatedComponent* aComponent)>();
}
