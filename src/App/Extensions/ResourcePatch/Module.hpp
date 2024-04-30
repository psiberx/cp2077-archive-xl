#pragma once

#include "App/Extensions/ModuleBase.hpp"
#include "App/Extensions/ResourcePatch/Unit.hpp"
#include "Red/AppearanceResource.hpp"
#include "Red/EntityTemplate.hpp"

namespace App
{
class ResourcePatchModule : public ConfigurableUnitModule<ResourcePatchUnit>
{
public:
    std::string_view GetName() override;
    bool Load() override;
    void Reload() override;
    bool Unload() override;

private:
    void PreparePatches();

    static void OnResourceRequest(Red::ResourceDepot*, const uintptr_t* aOut, Red::ResourcePath aPath, const int32_t*);
    static void OnEntityTemplateLoad(Red::EntityTemplate* aTemplate, void*);
    static void OnEntityTemplateExtract(void** aEntityBuilder, void*);
    static void OnAppearanceResourceLoad(Red::AppearanceResource* aResource);
    static void OnMeshResourceLoad(Red::CMesh* aMesh, void*);

    template<typename T>
    static Red::Handle<T> GetPatchResource(Red::ResourcePath aPath);

    template<typename T>
    static Red::SharedPtr<Red::ResourceToken<T>> GetPatchToken(Red::ResourcePath aPath);

    inline static Core::Map<Red::ResourcePath, Core::Set<Red::ResourcePath>> s_patches;
    inline static Core::Map<Red::ResourcePath, Red::SharedPtr<Red::ResourceToken<>>> s_tokens;
    inline static std::shared_mutex s_tokenLock;
};
}
