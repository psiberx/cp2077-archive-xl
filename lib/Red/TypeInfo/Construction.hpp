#pragma once

#include "Resolving.hpp"

namespace Red
{
template<typename T>
inline T* Construct()
{
    auto type = GetType<T>();
    auto instance = reinterpret_cast<T*>(type->GetAllocator()->AllocAligned(type->GetSize(),
                                                                            type->GetAlignment()).memory);
    type->Construct(instance);
    return instance;
}

template<typename T>
inline void Destruct(T* aInstance)
{
    auto type = GetType<T>();
    type->Destruct(aInstance);
    type->GetAllocator()->Free(aInstance);
}
}

template<typename T>
requires (std::is_abstract_v<T> || !RED4ext::Detail::HasStaticAllocator<T>) && Red::Detail::HasGeneratedTypeName<T>
struct RED4ext::Detail::AllocatorHook<T> : std::true_type
{
    inline static Memory::IAllocator* Get()
    {
        return Red::GetClass<T>()->GetAllocator();
    }
};

template<typename T>
requires (std::is_abstract_v<T> || !RED4ext::Detail::HasStaticAllocator<T>) && Red::Detail::HasGeneratedTypeName<T>
struct RED4ext::Detail::ConstructorHook<T> : std::true_type
{
    inline static void Apply(T* aInstance)
    {
        Red::GetClass<T>()->ConstructCls(aInstance);
    }
};
