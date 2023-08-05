#pragma once

#include "Core/Raw.hpp"
#include "Red/Addresses.hpp"

namespace Raw::ItemObject
{
using ItemID = Core::OffsetPtr<0x288, Red::ItemID>;

constexpr auto GetAppearanceName = Core::RawVFunc<
    /* offset = */ 0x280,
    /* type = */ Red::CName* (Red::ItemObject::*)(Red::CName& aOut)>();
}
