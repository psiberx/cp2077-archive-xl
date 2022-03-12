#pragma once

#include "Resources/ResourcePath.hpp"

#include <functional>
#include <RED4ext/NativeTypes.hpp>

template<>
struct std::hash<RED4ext::CName>
{
    std::size_t operator()(RED4ext::CName aKey) const
    {
        return static_cast<size_t>(aKey.hash);
    }
};

template<>
struct std::hash<RED4ext::TweakDBID>
{
    std::size_t operator()(RED4ext::TweakDBID aKey) const
    {
        return static_cast<size_t>(aKey.value);
    }
};

template<>
struct std::hash<Engine::ResourcePath>
{
    std::size_t operator()(Engine::ResourcePath aKey) const
    {
        return static_cast<size_t>(aKey.hash);
    }
};
