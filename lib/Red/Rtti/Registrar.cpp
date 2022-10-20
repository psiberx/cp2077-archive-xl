#include "Registrar.hpp"

namespace
{
std::vector<Red::Rtti::Registrar*> s_pending;
}

Red::Rtti::Registrar::Registrar(CallbackFunc aRegFunc, CallbackFunc aBuildFunc)
    : m_registered(false)
    , m_regFunc(aRegFunc)
    , m_buildFunc(aBuildFunc)
{
    s_pending.push_back(this);
}

void Red::Rtti::Registrar::Register()
{
    if (!m_registered)
    {
        RED4ext::RTTIRegistrator::Add(m_regFunc, m_buildFunc);
        m_registered = true;
    }
}

void Red::Rtti::Registrar::RegisterPending()
{
    for (const auto& pending : s_pending)
        pending->Register();

    s_pending.clear();
}
