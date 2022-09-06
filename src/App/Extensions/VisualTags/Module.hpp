#pragma once

#include "Raws.hpp"
#include "App/Module/ModuleBase.hpp"

namespace App
{
class VisualTagsModule : public Module
{
public:
    std::string_view GetName() override;
    bool Load() override;
    bool Unload() override;

private:
    static void OnGetVisualTags(RED4ext::game::AppearanceNameVisualTagsPreset& aPreset,
                                RED4ext::ResourcePath aEntityPath,
                                RED4ext::CName aAppearanceName,
                                RED4ext::red::TagList& aOutTags);
};
}
