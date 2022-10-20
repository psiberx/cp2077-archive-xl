#pragma once

#include "Core/Container/Container.hpp"

namespace Core
{
template<typename T>
inline Core::SharedPtr<T> Resolve()
{
    return Container::Get<T>();
}

template<typename T>
inline bool Resolvable()
{
    return Container::Get<T>();
}
}
