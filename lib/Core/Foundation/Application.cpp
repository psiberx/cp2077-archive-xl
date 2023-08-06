#include "Application.hpp"

void Core::Application::Bootstrap()
{
    if (m_booted)
        return;

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

void Core::Application::OnRegistered(const Core::SharedPtr<Core::Feature>& aFeature)
{
    aFeature->OnRegister();

    if (m_booted)
    {
        aFeature->OnBootstrap();
    }
}
