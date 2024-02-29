#pragma once

namespace Raw::FactoryIndex
{
constexpr auto LoadFactoryAsync = Core::RawFunc<
    /* addr = */ Red::AddressLib::FactoryIndex_LoadFactoryAsync,
    /* type = */ void (*)(uintptr_t aIndex, Red::ResourcePath aPath, uintptr_t aContext)>();

constexpr auto ResolveResource = Core::RawFunc<
    /* addr = */ Red::AddressLib::FactoryIndex_ResolveResource,
    /* type = */ void (*)(uintptr_t aIndex, Red::ResourcePath& aPath, Red::CName aName)>();
}
