#include "Application.hpp"

void Core::Application::Bootstrap()
{
    if (m_booted)
        return;

    m_booted = true;

    for (const auto& registered : GetRegistered())
        registered->OnBootstrap();
}

void Core::Application::Shutdown()
{
    if (!m_booted)
        return;

    for (const auto& registered : GetRegistered())
        registered->OnShutdown();

    m_booted = false;
}

void Core::Application::OnRegistered(const Core::UniquePtr<Core::Feature>& aRegistered)
{
    if (m_booted)
        aRegistered->OnBootstrap();
}
