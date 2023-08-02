#pragma once

#include "ChunkMask.hpp"

namespace App
{
using OverrideTagDefinition = Core::Map<Red::CName, ChunkMask>;

class OverrideTagManager
{
public:
    OverrideTagManager() noexcept;

    void DefineTag(Red::CName aTag, OverrideTagDefinition aDefinition);
    [[nodiscard]] OverrideTagDefinition& GetOverrides(Red::CName aTag);

private:
    constexpr auto Hide()
    {
        return ChunkMask(false);
    }

    constexpr auto Hide(std::initializer_list<uint8_t> aChunks)
    {
        return ChunkMask(false, aChunks);
    }

    constexpr auto Show(std::initializer_list<uint8_t> aChunks)
    {
        return ChunkMask(true, aChunks);
    }

    Core::Map<Red::CName, OverrideTagDefinition> m_definitions;
};
}
