#pragma once

#include "Core/Raw.hpp"
#include "Red/Addresses.hpp"
#include "Red/TagList.hpp"

namespace Red
{
using AppearanceNameVisualTagsPreset = game::AppearanceNameVisualTagsPreset;
}

namespace Raw::AppearanceNameVisualTagsPreset
{
constexpr auto GetVisualTags = Core::RawFunc<
    /* addr = */ Red::Addresses::AppearanceNameVisualTagsPreset_GetVisualTags,
    /* type = */ void (*)(Red::AppearanceNameVisualTagsPreset& aPreset,
                          Red::ResourcePath aEntityPath,
                          Red::CName aAppearanceName,
                          Red::TagList& aOutTags)>();
}
