#pragma once

#include "Core/Raw.hpp"
#include "Red/Addresses.hpp"

namespace Raw::ResourcePath
{
constexpr auto Create = Core::RawFunc<
    /* addr = */ Red::Addresses::ResourcePath_Create,
    /* type = */ Red::ResourcePath* (*)(Red::ResourcePath* aOut, const std::string_view* aPath)>();
}
