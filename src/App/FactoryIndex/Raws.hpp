#pragma once

#include "stdafx.hpp"
#include "Core/Raw.hpp"
#include "Reverse/Addresses.hpp"

namespace App::Raw
{
using LoadFactoryAsync = Core::RawFunc<
    /* address = */ Reverse::Addresses::FactoryIndex_LoadFactoryAsync,
    /* signature = */ void (*)(uintptr_t aIndex, RED4ext::ResourcePath aPath, uintptr_t aContext)>;
}
