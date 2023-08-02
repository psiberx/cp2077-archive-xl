#pragma once

struct ChunkMask
{
    constexpr ChunkMask(bool aSet, std::initializer_list<uint8_t> aChunks) noexcept
    {
        Set(aSet, aChunks);
    }

    constexpr explicit ChunkMask(std::initializer_list<uint8_t> aChunks) noexcept
    {
        Set(false, aChunks);
    }

    ChunkMask(bool aSet, const std::vector<uint8_t>& aChunks) noexcept
    {
        Set(aSet, aChunks);
    }

    explicit ChunkMask(const std::vector<uint8_t>& aChunks) noexcept
    {
        Set(false, aChunks);
    }

    constexpr ChunkMask(bool aSet, uint64_t aMask) noexcept
        : set(aSet)
        , mask(aMask)
    {
    }

    constexpr explicit ChunkMask(uint64_t aMask) noexcept
        : ChunkMask(false, aMask)
    {
    }

    constexpr explicit ChunkMask(bool aSet) noexcept
        : set(aSet)
        , mask(aSet ? ~0ull : 0ull)
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
