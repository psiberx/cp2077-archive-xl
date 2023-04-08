#pragma once

#include "TypeInfo/Resolving.hpp"

template<>
struct std::hash<RED4ext::CName>
{
    std::size_t operator()(RED4ext::CName aKey) const
    {
        return aKey.hash;
    }
};

template<>
struct std::hash<RED4ext::TweakDBID>
{
    std::size_t operator()(RED4ext::TweakDBID aKey) const
    {
        return aKey.value;
    }
};

template<>
struct std::hash<RED4ext::ResourcePath>
{
    std::size_t operator()(RED4ext::ResourcePath aKey) const
    {
        return aKey.hash;
    }
};

template<typename T>
requires std::is_class_v<T> && std::is_convertible_v<T, size_t> && Red::Detail::HasGeneratedTypeName<T>
struct std::hash<T>
{
    std::size_t operator()(T aKey) const
    {
        return static_cast<size_t>(aKey);
    }
};

template<typename T>
struct RED4ext::HashMapHash<T, std::enable_if_t<std::is_same_v<T, uint64_t>>>
{
    uint32_t operator()(const T& aKey) const noexcept
    {
        return static_cast<uint32_t>(aKey) ^ ((aKey >> 32) & 0xFFFFFFFF);
    }
};
