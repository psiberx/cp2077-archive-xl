#include "Utils.hpp"

RED4ext::ent::TemplateAppearance* App::Utils::FindAppearanceTemplate(
    RED4ext::ent::EntityTemplate* aResource, RED4ext::CName aName)
{
    return Raw::FindAppearanceTemplate::Invoke(aResource, aName);
}

RED4ext::Handle<RED4ext::appearance::AppearanceDefinition> App::Utils::FindAppearanceDefinition(
    RED4ext::appearance::AppearanceResource* aResource, RED4ext::CName aName)
{
    RED4ext::Handle<RED4ext::appearance::AppearanceDefinition> result;
    Raw::FindAppearanceDefinition::Invoke(aResource, &result, aName, 0, 0);
    return std::move(result);
}

void App::Utils::CopyTags(RED4ext::red::TagList& aDst, const RED4ext::red::TagList& aSrc)
{
    for (const auto& tag : aSrc.tags)
        aDst.tags.EmplaceBack(tag);
}

void App::Utils::MergeTags(const RED4ext::red::TagList* aDst, const RED4ext::red::TagList* aSrc)
{
    Raw::MergeTagLists::Invoke(aDst, aSrc);
}
