#pragma once

#include "ChunkMask.hpp"

namespace App
{
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
