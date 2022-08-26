#pragma once

#include "Raws.hpp"
#include "Unit.hpp"
#include "App/Common/ModuleBase.hpp"

namespace App
{
class FactoryIndexModule : public ConfigurableUnitModule<FactoryIndexUnit>
{
public:
    bool Attach() override;
    bool Detach() override;
    std::string_view GetName() override;

private:
    void OnLoadFactoryAsync(uintptr_t aIndex, RED4ext::ResourcePath aPath, uintptr_t aContext);

    inline static Raw::LoadFactoryAsync::type LoadFactoryAsync;
};
}
