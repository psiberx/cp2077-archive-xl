#pragma once

#include "ResourcePath.hpp"
#include "Engine/Addresses.hpp"

#include <RED4ext/Relocation.hpp>
#include <RED4ext/Scripting/Natives/Generated/CResource.hpp>

namespace Engine
{
// TODO: Move to RED4ext.SDK
struct ResourceReference
{
    ResourceReference()
        : path(0ull)
    {
    }

    explicit ResourceReference(ResourcePath aPath)
        : path(aPath)
    {
    }

    ~ResourceReference()
    {
        Reset();
    }

    /**
     * Load the resource.
     */
    void Load()
    {
        using Load_t = void (*)(ResourceReference*);
        static RED4ext::RelocFunc<Load_t> Load_(Addresses::ResourceReference_Load);

        Load_(this);
    }

    /**
     * Load if it's not already loaded and get the resource.
     * @return The loaded resource.
     */
    RED4ext::Handle<RED4ext::CResource>& Fetch()
    {
        using Fetch_t = RED4ext::Handle<RED4ext::CResource>& (*)(ResourceReference*);
        static RED4ext::RelocFunc<Fetch_t> Fetch_(Addresses::ResourceReference_Fetch);

        return Fetch_(this);
    }

    /**
     * Reset the path and associated token.
     */
    void Reset()
    {
        using Reset_t = void (*)(ResourceReference*);
        static RED4ext::RelocFunc<Reset_t> Reset_(Addresses::ResourceReference_Reset);

        Reset_(this);
    }

    // This seems to be a smart pointer using the same counter used in Handle and WeakHandle,
    // but the instance can be anything, not just ISerializable. With a reversed token type
    // it'd be something like SharedPtr<ResourceToken>.
    // Until then we can keep it here and use Load() and Reset() to control the token.
    struct ResourceTokenPtr
    {
        void* instance{ nullptr };
        RED4ext::RefCnt* refCount{ nullptr };
    };

    ResourcePath path;
    ResourceTokenPtr token; // Filled after Load() or Fetch()
};
static_assert(sizeof(ResourceReference) == 0x18);
static_assert(sizeof(ResourceReference::ResourceTokenPtr) == 0x10);
}
