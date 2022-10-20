#pragma once

#include "Core/Raw.hpp"
#include "Red/Addresses.hpp"

namespace Raw::Localization
{
constexpr auto LoadOnScreens = Core::RawFunc<
    /* addr = */ Red::Addresses::Localization_LoadOnScreens,
    /* type = */ uint64_t (*)(Red::Handle<Red::loc::alization::PersistenceOnScreenEntries>& aOut,
                              Red::ResourcePath aPath)>();
}
