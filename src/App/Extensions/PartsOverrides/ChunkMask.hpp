#pragma once

struct ChunkMask
{
    constexpr ChunkMask(bool aSet, uint64_t aMask) noexcept
        : set(aSet)
        , mask(aMask)
    {
    }

    constexpr ChunkMask(uint64_t aMask) noexcept
        : ChunkMask(false, aMask)
    {
    }

    constexpr ChunkMask(bool aSet, std::initializer_list<uint8_t> aChunks) noexcept
    {
        Set(aSet, aChunks);
    }

    constexpr ChunkMask(std::initializer_list<uint8_t> aChunks) noexcept
    {
        Set(false, aChunks);
    }

    ChunkMask(const std::vector<uint8_t>& aChunks) noexcept
    {
        Set(false, aChunks);
    }

    constexpr ChunkMask() noexcept
        : set(false)
        , mask(0ull)
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

    template<typename T>
    constexpr void Set(bool aSet, const T& aChunks) noexcept
    {
        set = aSet;
        mask = 0ull;

        for (const auto& chunk : aChunks)
        {
            mask |= 1 << chunk;
        }

        if (!set && mask)
        {
            mask = ~mask;
        }
    }

    bool set;
    uint64_t mask;
};
