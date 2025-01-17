#pragma once

#include "App/Extensions/ExtensionBase.hpp"
#include "App/Extensions/FactoryIndex/Config.hpp"
#include "App/Shared/ResourcePathRegistry.hpp"

namespace App
{
class FactoryIndexExtension : public ConfigurableExtensionImpl<FactoryIndexConfig>
{
public:
    std::string_view GetName() override;
    bool Load() override;
    bool Unload() override;
    void Configure() override;

private:
    static void OnLoadFactoryAsync(uintptr_t aIndex, Red::ResourcePath aPath, uintptr_t aContext);

    inline static Core::Map<Red::ResourcePath, std::string> s_factories;
    inline static Core::SharedPtr<ResourcePathRegistry> s_resourcePathRegistry;
};
}
