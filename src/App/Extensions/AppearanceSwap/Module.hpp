#pragma once

#include "App/Extensions/ModuleBase.hpp"
#include "Red/AppearanceResource.hpp"
#include "Red/EntityTemplate.hpp"

namespace App
{
class AppearanceSwapModule : public Module
{
public:
    bool Load() override;
    bool Unload() override;
    std::string_view GetName() override;

private:
    static Red::TemplateAppearance* OnFindAppearance(Red::EntityTemplate* aTemplate,
                                                     Red::CName aAppearanceName);
    static void OnFindDefinition(Red::AppearanceResource* aResource,
                                 Red::Handle<Red::AppearanceDefinition>* aDefinition,
                                 Red::CName aAppearanceName,
                                 uint32_t a4, uint8_t a5);
    static bool OnLoadEntityTemplate(uintptr_t aRequest);

    static inline Core::UniquePtr<Red::TemplateAppearance> s_emptyAppearance;
};
}
