#pragma once

#include "ResourcePath.hpp"

namespace Engine
{
// TODO: Move to RED4ext.SDK
struct ResourceAsyncReference
{
    explicit ResourceAsyncReference(ResourcePath aPath = "")
        : path(aPath) {}

    ResourcePath path;
};
static_assert(sizeof(ResourceAsyncReference) == 0x8);
}
