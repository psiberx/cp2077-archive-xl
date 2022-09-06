#include "Application.hpp"

void Core::Application::Bootstrap()
{
    if (m_booted)
        return;

    m_booted = true;

    for (const auto& feature : GetRegistered())
    {
        feature->OnBootstrap();
    }

    OnBootstrap();
}

void Core::Application::Shutdown()
{
    if (!m_booted)
        return;

    OnShutdown();

    for (const auto& feature : GetRegistered())
    {
        feature->OnShutdown();
    }

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
