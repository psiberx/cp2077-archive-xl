#pragma once

#include <RED4ext/Handle.hpp>

namespace Engine
{
// TODO: Move to RED4ext.SDK

// It seems to be a smart pointer using the same counter used in the handles,
// but the pointer can be anything, not just ISerializable.
struct ResourceTokenPtr
{
    ResourceTokenPtr()
        : instance(nullptr)
        , refCount(nullptr) {}

    void* instance;
    RED4ext::RefCnt* refCount;
};
static_assert(sizeof(ResourceTokenPtr) == 0x10);
}
