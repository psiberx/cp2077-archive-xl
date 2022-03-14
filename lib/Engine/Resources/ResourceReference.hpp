#pragma once

#include "ResourcePath.hpp"
#include "ResourceTokenPtr.hpp"
#include "Engine/Addresses.hpp"

#include <RED4ext/Relocation.hpp>
#include <RED4ext/Scripting/Natives/Generated/CResource.hpp>

namespace Engine
{
// TODO: Move to RED4ext.SDK
struct ResourceReference
{
    explicit ResourceReference(ResourcePath aPath = "")
        : path(aPath)
        , token() {}

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

    ResourcePath path;
    ResourceTokenPtr token; // Filled after Load or Fetch
};
static_assert(sizeof(ResourceReference) == 0x18);
}
