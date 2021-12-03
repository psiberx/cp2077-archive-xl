#pragma once

#include "Extension.hpp"
#include "LanguageCode.hpp"
#include "LocalizationDef.hpp"
#include "Reverse/ResourcePath.hpp"
#include "RED4ext/Scripting/Natives/Generated/loc/alization/PersistenceOnScreenEntries.hpp"

namespace AXL
{
class LocalizationExtension: public Extension
{
public:
    void Configure(const std::string& aName, const YAML::Node& aNode) override;
    void Attach() override;

private:
    using OnScreenEntriesHandle = RED4ext::Handle<RED4ext::loc::alization::PersistenceOnScreenEntries>;

    inline static uint64_t (*CallLoadOnscreens)(OnScreenEntriesHandle* aResult, ResourcePath* aResourcePath);
    uint64_t CatchLoadOnscreens(OnScreenEntriesHandle* aResult, ResourcePath* aResourcePath);
    void AppendOnscreensEntries(OnScreenEntriesHandle& aResult, ResourcePath aResourcePath);

    std::vector<LocalizationDefinition> m_definitions;
};
}
