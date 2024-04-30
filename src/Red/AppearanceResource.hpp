#pragma once

#include "Red/Package.hpp"

namespace Red
{
using AppearanceResource = appearance::AppearanceResource;
using AppearanceDefinition = appearance::AppearanceDefinition;
}

namespace Raw::AppearanceResource
{
using Mutex = Core::OffsetPtr<0xF0, Red::SharedMutex>;
using PackageData = Core::OffsetPtr<0x110, Red::PackageData>;

constexpr auto OnLoad = Core::RawFunc<
    /* addr = */ Red::AddressLib::AppearanceResource_OnLoad,
    /* type = */ void (*)(Red::AppearanceResource* aResource)>();

constexpr auto FindAppearance = Core::RawFunc<
    /* addr = */ Red::AddressLib::AppearanceResource_FindAppearanceDefinition,
    /* type = */ uintptr_t (*)(Red::AppearanceResource* aResource,
                               Red::Handle<Red::AppearanceDefinition>* aDefinition,
                               Red::CName aName,
                               uint32_t,
                               uint8_t)>();
}
