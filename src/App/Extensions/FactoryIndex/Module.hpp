#pragma once

#include "App/Extensions/ModuleBase.hpp"
#include "App/Extensions/FactoryIndex/Unit.hpp"

namespace App
{
class FactoryIndexModule : public ConfigurableUnitModule<FactoryIndexUnit>
{
public:
    std::string_view GetName() override;
    bool Load() override;
    bool Unload() override;

private:
    void OnLoadFactoryAsync(uintptr_t aIndex, Red::ResourcePath aPath, uintptr_t aContext);
};
}
