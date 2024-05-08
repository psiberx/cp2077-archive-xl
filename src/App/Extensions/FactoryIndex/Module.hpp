#pragma once

#include "App/Extensions/ModuleBase.hpp"
#include "App/Extensions/FactoryIndex/Config.hpp"

namespace App
{
class FactoryIndexModule : public ConfigurableModuleImpl<FactoryIndexConfig>
{
public:
    std::string_view GetName() override;
    bool Load() override;
    bool Unload() override;

private:
    void OnLoadFactoryAsync(uintptr_t aIndex, Red::ResourcePath aPath, uintptr_t aContext);
};
}
