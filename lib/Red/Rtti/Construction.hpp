#pragma once

#include "Resolving.hpp"

template<typename T>
requires std::is_abstract_v<T> && Red::Detail::HasGeneratedTypeName<T>
struct RED4ext::Detail::AllocatorHook<T> : std::true_type
{
    inline static Memory::IAllocator* Get()
    {
        return Red::GetClass<T>()->GetAllocator();
    }
};

template<typename T>
requires std::is_abstract_v<T> && Red::Detail::HasGeneratedTypeName<T>
struct RED4ext::Detail::ConstructorHook<T> : std::true_type
{
    inline static void Apply(T* aInstance)
    {
        Red::GetClass<T>()->ConstructCls(aInstance);
    }
};
