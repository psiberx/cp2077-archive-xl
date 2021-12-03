#include "stdafx.hpp"
#include "FactoryIndexExt.hpp"
#include "Reverse/Addresses.hpp"

void AXL::FactoryIndexExtension::Configure(const std::string& aName, const YAML::Node& aNode)
{
    auto definition = FactoryIndexDefinition::FromConfig(aName, aNode);

    if (definition.IsDefined())
    {
        m_definitions.emplace_back(std::move(definition));
    }
}

void AXL::FactoryIndexExtension::Attach()
{
    if (!m_definitions.empty())
    {
        Hook(Addresses::FactoryIndex_LoadFactoryAsync, this,
             &FactoryIndexExtension::CatchLoadFactoryAsync,
             &FactoryIndexExtension::CallLoadFactoryAsync);
    }
}

void AXL::FactoryIndexExtension::CatchLoadFactoryAsync(uintptr_t aThis, const ResourcePath* aResourcePath, uintptr_t aContext)
{
    CallLoadFactoryAsync(aThis, aResourcePath, aContext);

    if (aResourcePath->hash == LastFactory)
    {
        for (const auto& definition : m_definitions)
        {
            for (const auto& resourcePath : definition.factories)
                CallLoadFactoryAsync(aThis, &resourcePath, aContext);
        }

        m_handledContext = aContext;
    }
}
