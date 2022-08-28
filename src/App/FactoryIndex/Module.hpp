#pragma once

#include "Raws.hpp"
#include "Unit.hpp"
#include "App/Common/ModuleBase.hpp"

namespace App
{
class FactoryIndexModule : public ConfigurableUnitModule<FactoryIndexUnit>
{
public:
    std::string_view GetName() override;
    bool Load() override;
    bool Unload() override;

private:
    void OnLoadFactoryAsync(uintptr_t aIndex, RED4ext::ResourcePath aPath, uintptr_t aContext);

    inline static Raw::LoadFactoryAsync::type LoadFactoryAsync;
};
}
