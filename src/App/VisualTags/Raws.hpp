#pragma once

#include "stdafx.hpp"
#include "Core/Raw.hpp"
#include "Reverse/Addresses.hpp"

#include <RED4ext/Scripting/Natives/Generated/ent/EntityTemplate.hpp>
#include <RED4ext/Scripting/Natives/Generated/ent/TemplateAppearance.hpp>
#include <RED4ext/Scripting/Natives/Generated/appearance/AppearanceResource.hpp>
#include <RED4ext/Scripting/Natives/Generated/appearance/AppearanceDefinition.hpp>
#include <RED4ext/Scripting/Natives/Generated/game/AppearanceNameVisualTagsPreset.hpp>
#include <RED4ext/Scripting/Natives/Generated/red/TagList.hpp>

namespace App::Raw
{
using GetVisualTags = Core::RawFunc<
    /* address = */ Reverse::Addresses::AppearanceNameVisualTagsPreset_GetVisualTags,
    /* signature = */ void (*)(RED4ext::game::AppearanceNameVisualTagsPreset& aPreset,
                               RED4ext::ResourcePath aEntityPath,
                               RED4ext::CName aAppearanceName,
                               RED4ext::red::TagList& aOutTags)>;

using FindAppearanceTemplate = Core::RawFunc<
    /* address = */ Reverse::Addresses::EntityTemplate_FindAppearance,
    /* signature = */ RED4ext::ent::TemplateAppearance* (*)(RED4ext::ent::EntityTemplate* aResource,
                                                            RED4ext::CName aName)>;

using FindAppearanceDefinition = Core::RawFunc<
    /* address = */ Reverse::Addresses::AppearanceResource_FindAppearanceDefinition,
    /* signature = */ uintptr_t (*)(RED4ext::appearance::AppearanceResource* aResource,
                                    RED4ext::Handle<RED4ext::appearance::AppearanceDefinition>* aDefinition,
                                    RED4ext::CName aName, uint32_t a4, uint8_t a5)>;

using MergeTagLists = Core::RawFunc<
    /* address = */ Reverse::Addresses::TagList_MergeWith,
    /* signature = */ uint64_t (*)(const RED4ext::red::TagList* aDst, const RED4ext::red::TagList* aSrc)>;
}
