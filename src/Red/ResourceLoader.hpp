#pragma once

namespace Raw::ResourceLoader
{
constexpr auto OnUpdate = Core::RawFunc<
    /* addr = */ Red::AddressLib::ResourceLoader_OnUpdate,
    /* type = */ void (*)(Red::ResourceLoader* aLoader)>();
}
