#include "Application.hpp"

void Core::Application::Register(Core::UniquePtr<Feature> aFeature)
{
    if (m_booted)
        aFeature->Bootstrap();

    m_features.emplace_back(std::move(aFeature));
}

void Core::Application::Bootstrap()
{
    if (m_booted)
        return;

    for (const auto& feature : m_features)
        feature->Bootstrap();

    m_booted = true;
}

void Core::Application::Shutdown()
{
    if (!m_booted)
        return;

    for (const auto& feature : m_features)
        feature->Shutdown();

    m_booted = false;
}
