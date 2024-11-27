#pragma once

#include "App/Extensions/ExtensionBase.hpp"
#include "App/Extensions/FactoryIndex/Config.hpp"

namespace App
{
class FactoryIndexExtension : public ConfigurableExtensionImpl<FactoryIndexConfig>
{
public:
    std::string_view GetName() override;
    bool Load() override;
    bool Unload() override;

private:
    void OnLoadFactoryAsync(uintptr_t aIndex, Red::ResourcePath aPath, uintptr_t aContext);
};
}
