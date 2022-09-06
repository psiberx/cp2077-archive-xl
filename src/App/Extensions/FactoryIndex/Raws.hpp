#pragma once

#include "Core/Raw.hpp"
#include "App/Addresses.hpp"

namespace App::Raw
{
using LoadFactoryAsync = Core::RawFunc<
    /* address = */ Addresses::FactoryIndex_LoadFactoryAsync,
    /* signature = */ void (*)(uintptr_t aIndex, RED4ext::ResourcePath aPath, uintptr_t aContext)>;
}
