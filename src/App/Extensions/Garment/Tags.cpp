#include "Tags.hpp"

App::OverrideTagDefinition& App::OverrideTagManager::GetOverrides(Red::CName aTag)
{
    static OverrideTagDefinition s_emptyDefinition;

    const auto it = m_definitions.find(aTag);

    if (it == m_definitions.end())
        return s_emptyDefinition;

    return it.value();
}

void App::OverrideTagManager::DefineTag(Red::CName aTag, App::OverrideTagDefinition aDefinition)
{
    m_definitions[aTag] = std::move(aDefinition);
}
