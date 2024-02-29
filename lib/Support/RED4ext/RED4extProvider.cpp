#include "RED4extProvider.hpp"

Support::RED4extProvider::RED4extProvider(RED4ext::PluginHandle aPlugin, const RED4ext::Sdk* aSdk) noexcept
    : m_plugin(aPlugin)
    , m_sdk(aSdk)
    , m_enableLogging(false)
    , m_enableHooking(false)
    , m_enableAddressLibrary(false)
{
}

void Support::RED4extProvider::OnInitialize()
{
    if (m_enableLogging)
    {
        LoggingDriver::SetDefault(*this);
    }

    if (m_enableHooking)
    {
        HookingDriver::SetDefault(*this);
    }

    if (m_enableAddressLibrary)
    {
        AddressResolver::SetDefault(*this);
    }
}

void Support::RED4extProvider::LogInfo(const std::string_view& aMessage)
{
    m_sdk->logger->Info(m_plugin, aMessage.data());
}

void Support::RED4extProvider::LogWarning(const std::string_view& aMessage)
{
    m_sdk->logger->Warn(m_plugin, aMessage.data());
}

void Support::RED4extProvider::LogError(const std::string_view& aMessage)
{
    m_sdk->logger->Error(m_plugin, aMessage.data());
}

void Support::RED4extProvider::LogDebug(const std::string_view& aMessage)
{
    m_sdk->logger->Debug(m_plugin, aMessage.data());
}

void Support::RED4extProvider::LogFlush()
{
}

bool Support::RED4extProvider::HookAttach(uintptr_t aAddress, void* aCallback)
{
    return m_sdk->hooking->Attach(m_plugin, reinterpret_cast<void*>(aAddress), aCallback, nullptr);
}

bool Support::RED4extProvider::HookAttach(uintptr_t aAddress, void* aCallback, void** aOriginal)
{
    return m_sdk->hooking->Attach(m_plugin, reinterpret_cast<void*>(aAddress), aCallback, aOriginal);
}

bool Support::RED4extProvider::HookDetach(uintptr_t aAddress)
{
    return m_sdk->hooking->Detach(m_plugin, reinterpret_cast<void*>(aAddress));
}

uintptr_t Support::RED4extProvider::ResolveAddress(uint32_t aAddressID)
{
    return RED4ext::UniversalRelocBase::Resolve(aAddressID);
}
