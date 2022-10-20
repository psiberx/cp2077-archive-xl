#pragma once

#include "Core/Raw.hpp"
#include "Red/Addresses.hpp"

namespace Raw::FactoryIndex
{
constexpr auto LoadFactoryAsync = Core::RawFunc<
    /* addr = */ Red::Addresses::FactoryIndex_LoadFactoryAsync,
    /* type = */ void (*)(uintptr_t aIndex, Red::ResourcePath aPath, uintptr_t aContext)>();
}
