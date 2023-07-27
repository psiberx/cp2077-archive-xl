#pragma once

#include "App/Extensions/ModuleBase.hpp"
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
    static Red::TemplateAppearance* OnFindAppearance(Red::EntityTemplate* aTemplate, Red::CName aName);
    static bool OnLoadEntityTemplate(uintptr_t aRequest);

    static inline Core::UniquePtr<Red::TemplateAppearance> s_emptyApperance;
};
}
