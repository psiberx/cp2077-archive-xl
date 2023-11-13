#include "Application.hpp"

void Core::Application::Bootstrap()
{
    if (m_booted)
        return;

    if (!s_discoveryCallbacks.empty())
    {
        for (const auto& callback : s_discoveryCallbacks)
        {
            callback(*this);
        }
        s_discoveryCallbacks.clear();
    }

    m_booted = true;

    OnStarting();

    for (const auto& feature : GetRegistered())
    {
        feature->OnBootstrap();
    }

    OnStarted();
}

void Core::Application::Shutdown()
{
    if (!m_booted)
        return;

    OnStopping();

    for (const auto& feature : GetRegistered())
    {
        feature->OnShutdown();
    }

    OnStopped();

    m_booted = false;
}

void Core::Application::OnRegistered(const SharedPtr<Feature>& aFeature)
{
    aFeature->OnRegister();

    if (m_booted)
    {
        aFeature->OnBootstrap();
    }
}

bool Core::Application::Discover(AutoDiscoveryCallback aCallback)
{
    s_discoveryCallbacks.push_back(aCallback);
    return true;
}
