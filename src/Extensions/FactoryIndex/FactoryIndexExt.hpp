#pragma once

#include "Extension.hpp"
#include "FactoryIndexDef.hpp"
#include "Reverse/ResourcePath.hpp"

namespace AXL
{
class FactoryIndexExtension: public Extension
{
public:
    void Configure(const std::string& aName, const YAML::Node& aNode) override;
    void Attach() override;

private:
    static constexpr uint64_t LastFactory = RED4ext::FNV1a64(R"(base\gameplay\factories\vehicles\vehicles.csv)");

    inline static void (*CallLoadFactoryAsync)(uintptr_t aThis, const ResourcePath* aResourcePath, uintptr_t aContext);
    void CatchLoadFactoryAsync(uintptr_t aThis, const ResourcePath* aResourcePath, uintptr_t aContext);

    std::vector<FactoryIndexDefinition> m_definitions;
    uintptr_t m_handledContext{ 0 };
};
}
