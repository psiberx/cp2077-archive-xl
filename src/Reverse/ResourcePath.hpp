#pragma once

#include "stdafx.hpp"

namespace AXL
{
using ResourceHash = uint64_t;

struct ResourcePath
{
    constexpr ResourcePath(ResourceHash aHash = 0) noexcept
        : hash(aHash)
    {
    }

    constexpr ResourcePath(const char* aPath) noexcept
        : hash(0)
    {
        // TODO: Sanitize input
        hash = RED4ext::FNV1a64(aPath);
    }

    constexpr operator uint64_t() const noexcept
    {
        return hash;
    }

    constexpr size_t operator()(const ResourcePath& aResource) const
    {
        return aResource.hash;
    }

    constexpr ResourcePath& operator=(const ResourceHash aRhs) noexcept
    {
        hash = aRhs;
        return *this;
    }

    constexpr ResourcePath& operator=(const char* aRhs) noexcept
    {
        *this = ResourcePath(aRhs);
        return *this;
    }

    constexpr ResourcePath& operator=(const ResourcePath& aRhs) noexcept
    {
        hash = aRhs.hash;
        return *this;
    }

    constexpr bool operator==(const ResourcePath& aRhs) const noexcept
    {
        return hash == aRhs.hash;
    }

    constexpr bool operator!=(const ResourcePath& aRhs) const noexcept
    {
        return !(*this == aRhs);
    }

    constexpr bool operator==(const ResourceHash aRhs) const noexcept
    {
        return hash == aRhs;
    }

    constexpr bool operator!=(const ResourceHash aRhs) const noexcept
    {
        return hash != aRhs;
    }

    constexpr bool IsEmpty() const noexcept
    {
        return hash == 0;
    }

    ResourceHash hash;
};
static_assert(sizeof(ResourcePath) == 0x8);
}
