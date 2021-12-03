#include "stdafx.hpp"
#include "LocalizationExt.hpp"
#include "LanguageCode.hpp"
#include "Reverse/Addresses.hpp"

void AXL::LocalizationExtension::Configure(const std::string& aName, const YAML::Node& aNode)
{
    auto definition = LocalizationDefinition::FromConfig(aName, aNode);

    if (definition.IsDefined())
    {
        m_definitions.emplace_back(std::move(definition));
    }
}

void AXL::LocalizationExtension::Attach()
{
    if (!m_definitions.empty())
    {
        Hook(Addresses::Localization_LoadOnscreens, this,
             &LocalizationExtension::CatchLoadOnscreens,
             &LocalizationExtension::CallLoadOnscreens);
    }
}

uint64_t AXL::LocalizationExtension::CatchLoadOnscreens(OnScreenEntriesHandle* aResult, ResourcePath* aResourcePath)
{
    auto returnValue = CallLoadOnscreens(aResult, aResourcePath);
    auto languageCode = ResolveLanguageCode(*aResourcePath);

    for (const auto& definition : m_definitions)
    {
        const auto finalCode = definition.onscreens.contains(languageCode) ? languageCode : definition.fallback;

        for (const auto& resourcePath : definition.onscreens.at(finalCode))
            AppendOnscreensEntries(*aResult, resourcePath);
    }

    return returnValue;
}

void AXL::LocalizationExtension::AppendOnscreensEntries(OnScreenEntriesHandle& aResult, ResourcePath aResourcePath)
{
    OnScreenEntriesHandle resource;
    CallLoadOnscreens(&resource, &aResourcePath);

    for (const auto& entry : *(&resource->entries))
    {
        aResult->entries.EmplaceBack(entry);
    }
}
