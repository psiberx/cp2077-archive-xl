#pragma once

#include "Core/Win.hpp"
#include "Core/Foundation/Feature.hpp"

namespace Core
{
class RuntimeProvider : public Feature
{
public:
    struct Options
    {
        HMODULE handle = nullptr;
        int exePathDepth = 0;
    };

    explicit RuntimeProvider(Options&& aOptions);
};
}
