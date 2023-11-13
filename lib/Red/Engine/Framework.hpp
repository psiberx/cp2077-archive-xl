#pragma once

#include "Macros/Framework.hpp"

namespace Red
{
template<typename T>
struct RuntimeSystemMapping : public std::false_type {};

namespace Detail
{
template<typename T>
concept HasRuntimeSystemMapping = RuntimeSystemMapping<T>::value
                                      && RuntimeSystemMapping<T>::offset >= 0
                                      && RuntimeSystemMapping<T>::offset <= 64;
}

template<typename T, typename U = T>
requires std::is_base_of_v<IGameSystem, T> && std::is_base_of_v<T, U>
U* GetGameSystem()
{
    static const auto s_type = GetType<T>();
    auto& gameInstance = CGameEngine::Get()->framework->gameInstance;
    return reinterpret_cast<U*>(gameInstance->GetSystem(s_type));
}

template<typename T, typename U = T>
requires std::is_base_of_v<worldIRuntimeSystem, T> && std::is_base_of_v<T, U> && Detail::HasRuntimeSystemMapping<T>
U* GetRuntimeSystem()
{
    constexpr auto systemOffset = RuntimeSystemMapping<T>::offset * sizeof(Handle<T>);
    const auto& runtimeSceneAddr = CGameEngine::Get()->framework->unk18;
    return reinterpret_cast<Handle<U>*>(runtimeSceneAddr + systemOffset)->instance;
}
}
