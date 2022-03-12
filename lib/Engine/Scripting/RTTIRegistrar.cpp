#include "RTTIRegistrar.hpp"

namespace
{
std::vector<Engine::RTTIRegistrar*> s_queued;
}

Engine::RTTIRegistrar::RTTIRegistrar(CallbackFunc aRegFunc, CallbackFunc aBuildFunc)
    : m_registered(false)
    , m_regFunc(aRegFunc)
    , m_buildFunc(aBuildFunc)
{
    s_queued.push_back(this);
}

void Engine::RTTIRegistrar::Register()
{
    if (!m_registered)
    {
        RED4ext::RTTIRegistrator::Add(m_regFunc, m_buildFunc);
        m_registered = true;
    }
}

void Engine::RTTIRegistrar::RegisterPending()
{
    for (const auto& pending : s_queued)
        pending->Register();

    s_queued.clear();
}
