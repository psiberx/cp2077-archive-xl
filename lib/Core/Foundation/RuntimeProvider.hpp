#pragma once

#include "Core/Foundation/Feature.hpp"
#include "Core/Win.hpp"

namespace Core
{
class RuntimeProvider : public Feature
{
public:
    explicit RuntimeProvider(HMODULE aHandle) noexcept;

    auto SetBaseImagePathDepth(int aDepth) noexcept
    {
        m_basePathDepth = aDepth;
        return Defer(this);
    }

protected:
    void OnInitialize() override;

    HMODULE m_handle;
    int m_basePathDepth;
};
}
