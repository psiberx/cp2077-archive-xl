#pragma once

#include "App/Extensions/ModuleBase.hpp"
#include "Red/AppearanceResource.hpp"
#include "Red/EntityTemplate.hpp"
#include "Red/VisualTagsPreset.hpp"

namespace App
{
class VisualTagsModule : public Module
{
public:
    std::string_view GetName() override;
    bool Load() override;
    bool Unload() override;

private:
    static void OnGetVisualTags(Red::AppearanceNameVisualTagsPreset& aPreset,
                                Red::ResourcePath aEntityPath,
                                Red::CName aAppearanceName,
                                Red::TagList& aOutTags);

    static Red::TemplateAppearance* FindAppearance(Red::EntityTemplate* aResource, Red::CName aName);
    static Red::Handle<Red::AppearanceDefinition> FindDefinition(Red::AppearanceResource* aResource, Red::CName aName);
    static void MergeTags(Red::TagList& aDst, const Red::TagList& aSrc);
};
}
