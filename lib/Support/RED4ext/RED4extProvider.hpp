#pragma once

#include "Core/Foundation/Feature.hpp"
#include "Core/Hooking/HookingDriver.hpp"
#include "Core/Logging/LoggingDriver.hpp"
#include "Core/Memory/AddressResolver.hpp"

namespace Support
{
class RED4extProvider
    : public Core::Feature
    , public Core::LoggingDriver
    , public Core::HookingDriver
    , public Core::AddressResolver
{
public:
    RED4extProvider(RED4ext::PluginHandle aPlugin, const RED4ext::Sdk* aSdk) noexcept;

    void LogInfo(const std::string_view& aMessage) override;
    void LogWarning(const std::string_view& aMessage) override;
    void LogError(const std::string_view& aMessage) override;
    void LogDebug(const std::string_view& aMessage) override;
    void LogFlush() override;

    bool HookAttach(uintptr_t aAddress, void* aCallback) override;
    bool HookAttach(uintptr_t aAddress, void* aCallback, void** aOriginal) override;
    bool HookDetach(uintptr_t aAddress) override;

    uintptr_t ResolveAddress(uint32_t aAddressID) override;

    auto EnableLogging() noexcept
    {
        m_enableLogging = true;
        return Defer(this);
    }

    auto EnableHooking() noexcept
    {
        m_enableHooking = true;
        return Defer(this);
    }

    auto EnableAddressLibrary() noexcept
    {
        m_enableAddressLibrary = true;
        return Defer(this);
    }

    auto RegisterScripts(const std::filesystem::path& aPath) noexcept
    {
        m_sdk->scripts->Add(m_plugin, aPath.c_str());
        return Defer(this);
    }

protected:
    void OnInitialize() override;

    RED4ext::PluginHandle m_plugin;
    const RED4ext::Sdk* m_sdk;
    bool m_enableLogging;
    bool m_enableHooking;
    bool m_enableAddressLibrary;
};
}
