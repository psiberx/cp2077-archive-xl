#pragma once

namespace Raw::ExtensionService
{
constexpr auto Bootstrap = Core::RawFunc<
    /* addr = */ Red::AddressLib::ExtensionService_Bootstrap,
    /* type = */ void (*)(void* aService)>();

constexpr auto Shutdown = Core::RawFunc<
    /* addr = */ Red::AddressLib::ExtensionService_Shutdown,
    /* type = */ void (*)(void* aService)>();
}

namespace Raw::ExtensionLoader
{
constexpr auto Configure = Core::RawFunc<
    /* addr = */ Red::AddressLib::ExtensionLoader_Configure,
    /* type = */ void (*)(void* aLoader)>();

constexpr auto AddConfig = Core::RawFunc<
    /* addr = */ Red::AddressLib::ExtensionLoader_AddConfig,
    /* type = */ void (*)(void* aLoader, void* aPath, void* aDir)>();
}
