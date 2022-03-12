#pragma once

#include <RED4ext/Hashing/FNV1a.hpp>

namespace Engine
{
// TODO: Move to RED4ext.SDK
struct LocKeyWrapper
{
    explicit LocKeyWrapper(const uint64_t& aPrimaryKey = 0)
        : primaryKey(aPrimaryKey)
    {
    }

    explicit LocKeyWrapper(const char* aSecondaryKey)
        : primaryKey(RED4ext::FNV1a64(aSecondaryKey))
    {
    }

    uint64_t primaryKey;
};
}
