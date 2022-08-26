#pragma once

#include "Raws.hpp"
#include "App/Common/ModuleBase.hpp"

namespace App
{
class VisualTagsModule : public Module
{
public:
    bool Attach() override;
    bool Detach() override;
    std::string_view GetName() override;

private:
    static void OnGetVisualTags(RED4ext::game::AppearanceNameVisualTagsPreset& aPreset,
                                RED4ext::ResourcePath aEntityPath,
                                RED4ext::CName aAppearanceName,
                                RED4ext::red::TagList& aOutTags);
};
}
