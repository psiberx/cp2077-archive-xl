#pragma once

#include "Red/Rtti/Resolving.hpp"

namespace Red
{
template<typename T, typename U = T>
requires std::is_base_of_v<IGameSystem, T> && std::is_base_of_v<T, U>
U* GetGameSystem()
{
    static const auto s_type = GetType<T>();
    auto& gameInstance = CGameEngine::Get()->framework->gameInstance;
    return reinterpret_cast<U*>(gameInstance->GetInstance(s_type));
}
}
