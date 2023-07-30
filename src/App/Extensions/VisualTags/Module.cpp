#include "Module.hpp"
#include "App/Extensions/AppearanceSwap/Module.hpp"

namespace
{
constexpr auto ModuleName = "VisualTags";
}

std::string_view App::VisualTagsModule::GetName()
{
    return ModuleName;
}

bool App::VisualTagsModule::Load()
{
    if (!HookAfter<Raw::AppearanceNameVisualTagsPreset::GetVisualTags>(&OnGetVisualTags))
        throw std::runtime_error("Failed to hook [AppearanceNameVisualTagsPreset::GetVisualTags].");

    return true;
}

bool App::VisualTagsModule::Unload()
{
    Unhook<Raw::AppearanceNameVisualTagsPreset::GetVisualTags>();

    return true;
}

void App::VisualTagsModule::OnGetVisualTags(Red::AppearanceNameVisualTagsPreset& aPreset,
                                            Red::ResourcePath aEntityPath,
                                            Red::CName aAppearanceName,
                                            Red::TagList& aOutTags)
{
    static std::shared_mutex s_autoTagsLock;
    static Core::Map<uint64_t, Red::TagList> s_autoTagsCache;
    static Red::TagList s_emptyTags{};

    if (aOutTags.tags.size > 0 || !aAppearanceName)
        return;

    auto cacheKey = Red::FNV1a64(aAppearanceName.ToString(), aEntityPath.hash);

    {
        std::shared_lock _(s_autoTagsLock);
        auto cachedTagsIt = s_autoTagsCache.find(cacheKey);
        if (cachedTagsIt != s_autoTagsCache.end())
        {
            MergeTags(aOutTags, cachedTagsIt->second);
            return;
        }
    }

    auto loader = Red::ResourceLoader::Get();
    auto entityToken = loader->FindToken<Red::ent::EntityTemplate>(aEntityPath);

    if (!entityToken || !entityToken->IsLoaded())
        return;

    auto entityTemplate = entityToken->Get();
    auto appearanceTemplate = FindAppearance(*entityToken, aAppearanceName);

    if (!appearanceTemplate)
    {
        std::unique_lock _(s_autoTagsLock);
        s_autoTagsCache.emplace(cacheKey, s_emptyTags);
        return;
    }

    auto appearancePath = appearanceTemplate->appearanceResource.path;
    auto appearanceToken = loader->FindToken<Red::appearance::AppearanceResource>(appearancePath);

    if (!appearanceToken || !appearanceToken->IsLoaded())
        return;

    auto appearanceDefinition = FindDefinition(*appearanceToken, appearanceTemplate->appearanceName);

    if (!appearanceDefinition)
    {
        std::unique_lock _(s_autoTagsLock);
        s_autoTagsCache.emplace(cacheKey, s_emptyTags);
        return;
    }

    Red::TagList autoTags;
    MergeTags(autoTags, appearanceDefinition->visualTags);

    if (entityTemplate->visualTagsSchema)
    {
        MergeTags(autoTags, entityTemplate->visualTagsSchema->visualTags);
    }

    {
        std::unique_lock _(s_autoTagsLock);
        auto cachedTagsIt = s_autoTagsCache.emplace(cacheKey, autoTags).first;
        MergeTags(aOutTags, cachedTagsIt->second);
    }
}

Red::TemplateAppearance* App::VisualTagsModule::FindAppearance(Red::EntityTemplate* aResource, Red::CName aName)
{
    return AppearanceSwapModule::OnFindAppearance(aResource, aName);
    // return Raw::EntityTemplate::FindAppearance(aResource, aName);
}

Red::Handle<Red::AppearanceDefinition> App::VisualTagsModule::FindDefinition(Red::AppearanceResource* aResource,
                                                                             Red::CName aName)
{
    Red::Handle<Red::AppearanceDefinition> result;
    Raw::AppearanceResource::FindAppearance(aResource, &result, aName, 0, 0);
    AppearanceSwapModule::OnFindDefinition(aResource, &result, aName, 0, 0);

    return result;
}

void App::VisualTagsModule::MergeTags(Red::TagList& aDst, const Red::TagList& aSrc)
{
    Raw::TagList::Merge(aDst, aSrc);
}
