#pragma once

struct ChunkMask
{
    // constexpr ChunkMask(bool aSet, uint64_t aMask = 0ull) noexcept
    //     : set(aSet)
    //     , mask(aMask)
    // {
    // }
    //
    // constexpr ChunkMask(uint64_t aMask = 0ull) noexcept
    //     : ChunkMask(false, aMask)
    // {
    // }

    constexpr ChunkMask(bool aSet, std::initializer_list<int8_t> aChunks) noexcept
        : set(aSet)
    {
        mask = 0ull;

        for (const auto& chunk : aChunks)
            mask |= 1 << chunk;

        if (!set && mask)
        {
            mask = ~mask;
        }
    }

    constexpr ChunkMask(std::initializer_list<int8_t> aChunks) noexcept
        : ChunkMask(false, aChunks)
    {
    }

    constexpr operator bool() const noexcept
    {
        return set;
    }

    constexpr operator uint64_t() const noexcept
    {
        return mask;
    }

    bool set;
    uint64_t mask;
};
