#pragma once

#include "Raws.hpp"

namespace App::Utils
{
RED4ext::ent::TemplateAppearance* FindAppearanceTemplate(
    RED4ext::ent::EntityTemplate* aResource, RED4ext::CName aName);

RED4ext::Handle<RED4ext::appearance::AppearanceDefinition> FindAppearanceDefinition(
    RED4ext::appearance::AppearanceResource* aResource, RED4ext::CName aName);

void CopyTags(RED4ext::red::TagList& aDst, const RED4ext::red::TagList& aSrc);
void MergeTags(const RED4ext::red::TagList* aDst, const RED4ext::red::TagList* aSrc);
}
