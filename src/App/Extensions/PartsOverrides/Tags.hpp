#pragma once

namespace App
{
struct ChunkMask
{
    constexpr ChunkMask(uint64_t aMask = 0ull) noexcept
        : mask(aMask)
    {
    }

    constexpr ChunkMask(std::initializer_list<int8_t> aChunks) noexcept
    {
        mask = 0ull;

        for (const auto& chunk : aChunks)
            mask |= 1 << chunk;

        mask = ~mask;
    }

    constexpr operator uint64_t() const noexcept
    {
        return mask;
    }

    uint64_t mask;
};

using OverrideTagDefinition = Core::Map<Red::CName, ChunkMask>;

class OverrideTagManager
{
public:
    OverrideTagManager() noexcept;

    [[nodiscard]] OverrideTagDefinition& GetOverrides(Red::CName aTag);

    [[nodiscard]] static Core::SharedPtr<OverrideTagManager>& Get();

private:
    Core::Map<Red::CName, OverrideTagDefinition> m_definitions;
};
}
