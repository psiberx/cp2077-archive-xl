#pragma once

#include "Core/Foundation/Feature.hpp"
#include "Core/Logging/LoggingDriver.hpp"
#include "Core/Hooking/HookingDriver.hpp"

#include <RED4ext/Api/Sdk.hpp>

namespace Vendor
{
class RED4extProvider : public Core::Feature, public Core::LoggingDriver, public Core::HookingDriver
{
public:
    struct Options
    {
        RED4ext::PluginHandle plugin = nullptr;
        const RED4ext::Sdk* sdk = nullptr;
        bool logging = false;
        bool hooking = false;
    };

    explicit RED4extProvider(Options&& aOptions);

private:
    void LogInfo(const std::string& aMessage) override;
    void LogWarning(const std::string& aMessage) override;
    void LogError(const std::string& aMessage) override;
    void LogFlush() override;

    bool HookAttach(uintptr_t aAddress, void* aCallback) override;
    bool HookAttach(uintptr_t aAddress, void* aCallback, void** aOriginal) override;
    bool HookDetach(uintptr_t aAddress) override;

    RED4ext::PluginHandle m_plugin;
    const RED4ext::Sdk* m_sdk;
};
}
