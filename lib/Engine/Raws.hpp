#pragma once

#include "Addresses.hpp"
#include "Core/Raw.hpp"

namespace Engine::Raw
{
using Core::RawFunc;

using Main = RawFunc<Addresses::Engine_Main, int32_t (*)()>;
}
