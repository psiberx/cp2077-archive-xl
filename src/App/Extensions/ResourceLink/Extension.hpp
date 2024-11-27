#pragma once

#include "App/Extensions/ExtensionBase.hpp"
#include "App/Extensions/ResourceLink/Config.hpp"
#include "Red/ResourceDepot.hpp"
#include "Red/ResourceLoader.hpp"

namespace App
{
class ResourceLinkExtension : public ConfigurableExtensionImpl<ResourceLinkConfig>
{
public:
    std::string_view GetName() override;
    bool Load() override;
    bool Unload() override;
    void Configure() override;

private:
    static void OnResourceRequest(Red::ResourceLoader* aLoader, Red::SharedPtr<Red::ResourceToken<>>& aToken,
                                  Red::ResourceRequest& aRequest);
    static bool OnResourceCheck(Red::ResourceDepot* aDepot, Red::ResourcePath aPath);

    inline static Core::Map<Red::ResourcePath, Red::ResourcePath> s_links;
    inline static Core::Map<Red::ResourcePath, std::string> s_paths;
};
}
