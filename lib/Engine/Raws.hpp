#pragma once

#include "Addresses.hpp"
#include "Core/Raw.hpp"

namespace Engine::Raw
{
using Main = Core::RawFunc<
    /* address = */ Addresses::Engine_Main,
    /* signature = */ int32_t (*)()>;
}
