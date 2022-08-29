#pragma once

#include "stdafx.hpp"

namespace App
{
using OverrideTagDefinition = Core::Map<RED4ext::CName, uint64_t>;

class OverrideTagManager
{
public:
    OverrideTagManager() noexcept;

    [[nodiscard]] OverrideTagDefinition& GetOverrides(RED4ext::CName aTag);

    [[nodiscard]] static Core::SharedPtr<OverrideTagManager>& Get();

private:
    Core::Map<RED4ext::CName, OverrideTagDefinition> m_definitions;
};
}
