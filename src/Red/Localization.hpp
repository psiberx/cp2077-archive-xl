#pragma once

#include "Core/Raw.hpp"
#include "Red/Addresses.hpp"

namespace Red
{
constexpr auto LocKeyPrefix = "LocKey#";
constexpr auto LocKeyPrefixLength = std::char_traits<char>::length(LocKeyPrefix);
}

namespace Raw::Localization
{
constexpr auto LoadOnScreens = Core::RawFunc<
    /* addr = */ Red::Addresses::Localization_LoadOnScreens,
    /* type = */ uint64_t (*)(Red::Handle<Red::localization::PersistenceOnScreenEntries>& aOut,
                              Red::ResourcePath aPath)>();

constexpr auto LoadSubtitles = Core::RawFunc<
    /* addr = */ Red::Addresses::Localization_LoadSubtitles,
    /* type = */ uint64_t (*)(Red::Handle<Red::localization::PersistenceSubtitleMap>& aOut,
                              Red::ResourcePath aPath)>();
}
