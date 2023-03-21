#pragma once

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
struct std::hash<RED4ext::ResourcePath>
{
    std::size_t operator()(RED4ext::ResourcePath aKey) const
    {
        return static_cast<size_t>(aKey.hash);
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
