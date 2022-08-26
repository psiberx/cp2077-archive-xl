#pragma once

#include "Addresses.hpp"
#include "Core/Raw.hpp"

#include <RED4ext/GameEngine.hpp>

namespace Engine::Raw
{
using Main = Core::RawFunc<
    /* address = */ Addresses::Engine_Main,
    /* signature = */ int32_t (*)()>;

using Init = Core::RawFunc<
    /* address = */ Addresses::Engine_Init,
    /* signature = */ bool (*)(RED4ext::CBaseEngine* aEngine, RED4ext::CGameOptions* aOptions)>;
}
