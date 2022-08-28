#include "RTTILocator.hpp"

Engine::RTTILocator::RTTILocator(RED4ext::CName aName)
    : m_name(aName)
    , m_type(nullptr)
{
}

void Engine::RTTILocator::Resolve()
{
    m_type = RED4ext::CRTTISystem::Get()->GetType(m_name);
}

Engine::RTTILocator::operator const RED4ext::CBaseRTTIType*()
{
    if (!m_type)
    {
        Resolve();
    }

    return m_type;
}

Engine::RTTILocator::operator const RED4ext::CClass*()
{
    if (!m_type)
    {
        Resolve();
    }

    if (m_type->GetType() != RED4ext::ERTTIType::Class)
    {
        return nullptr;
    }

    return reinterpret_cast<RED4ext::CClass*>(m_type);
}

Engine::RTTILocator::operator bool()
{
    return m_type;
}
