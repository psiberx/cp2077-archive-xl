#pragma once

#include "Red/TagList.hpp"

namespace Red
{
using AppearanceNameVisualTagsPreset = game::AppearanceNameVisualTagsPreset;
}

namespace Raw::AppearanceNameVisualTagsPreset
{
constexpr auto GetVisualTags = Core::RawFunc<
    /* addr = */ Red::AddressLib::AppearanceNameVisualTagsPreset_GetVisualTags,
    /* type = */ void (*)(Red::AppearanceNameVisualTagsPreset& aPreset, // FIXME: might not be a preset anymore
                          Red::ResourcePath aEntityPath,
                          Red::CName aAppearanceName,
                          Red::TagList& aOutTags)>();
}
