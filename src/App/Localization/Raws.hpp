#pragma once

#include "Core/Raw.hpp"
#include "Reverse/Addresses.hpp"

#include <RED4ext/Scripting/Natives/Generated/loc/alization/PersistenceOnScreenEntries.hpp>

namespace App::Raw
{
using OnScreenEntries = RED4ext::loc::alization::PersistenceOnScreenEntries;

using LoadOnScreens = Core::RawFunc<
    /* address = */ Reverse::Addresses::Localization_LoadOnScreens,
    /* signature = */ uint64_t (*)(RED4ext::Handle<OnScreenEntries>* aOut, RED4ext::ResourcePath aPath)>;
}
