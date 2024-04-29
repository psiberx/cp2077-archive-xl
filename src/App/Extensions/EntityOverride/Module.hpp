#pragma once

#include "App/Extensions/ModuleBase.hpp"
#include "App/Extensions/EntityOverride/Unit.hpp"
#include "Red/AppearanceResource.hpp"
#include "Red/EntityTemplate.hpp"

namespace App
{
class EntityOverrideModule : public ConfigurableUnitModule<EntityOverrideUnit>
{
public:
    std::string_view GetName() override;
    bool Load() override;
    void Reload() override;
    bool Unload() override;

private:
    void PrepareOverrides();

    static void OnResourceRequest(Red::ResourceDepot*, const uintptr_t* aOut, Red::ResourcePath aPath, const int32_t*);
    static void OnEntityTemplateLoad(Red::EntityTemplate* aTemplate, void*);
    static void OnEntityTemplateExtract(void** aEntityBuilder, void*);
    static void OnAppearanceResourceLoad(Red::AppearanceResource* aResource);
    static void OnMeshResourceLoad(Red::CMesh* aMesh, void*);

    template<typename T>
    static Red::Handle<T> GetOverride(Red::ResourcePath aPath);

    template<typename T>
    static Red::SharedPtr<Red::ResourceToken<T>> GetOverrideToken(Red::ResourcePath aPath);

    inline static Core::Map<Red::ResourcePath, Core::Set<Red::ResourcePath>> s_overrides;
    inline static Core::Map<Red::ResourcePath, Red::SharedPtr<Red::ResourceToken<>>> s_tokens;
    inline static std::shared_mutex s_tokenLock;
};
}
