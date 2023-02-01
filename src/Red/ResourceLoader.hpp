#pragma once

#include "Core/Raw.hpp"
#include "Red/Addresses.hpp"

namespace Raw::ResourceLoader
{
constexpr auto OnUpdate = Core::RawFunc<
    /* addr = */ Red::Addresses::ResourceLoader_OnUpdate,
    /* type = */ void (*)(Red::ResourceLoader& aLoader)>();
}
