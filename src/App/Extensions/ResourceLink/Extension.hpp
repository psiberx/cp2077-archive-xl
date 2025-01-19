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
    static void OnLoaderResourceRequest(Red::ResourceLoader* aLoader, Red::SharedPtr<Red::ResourceToken<>>& aToken,
                                        Red::ResourceRequest& aRequest);
    static uintptr_t* OnDepotResourceRequest(Red::ResourceDepot* aDepot, const uintptr_t* aResourceHandle,
                                             Red::ResourcePath aPath, const int32_t* aArchiveHandle);
    static bool OnDepotResourceCheck(Red::ResourceDepot* aDepot, Red::ResourcePath aPath);

    inline static Core::Map<Red::ResourcePath, Red::ResourcePath> s_mappings;
    inline static Core::Map<Red::ResourcePath, Red::ResourcePath> s_copies;
    inline static Core::Map<Red::ResourcePath, Red::ResourcePath> s_links;
};
}
