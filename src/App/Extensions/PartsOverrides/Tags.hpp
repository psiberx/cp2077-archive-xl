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
    Core::Map<Red::CName, OverrideTagDefinition> m_definitions;
};
}
