#include "Module.hpp"
#include "Utils.hpp"

#include <RED4ext/NativeTypes.hpp>
#include <RED4ext/ResourceLoader.hpp>
#include <RED4ext/RTTISystem.hpp>

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
    if (!HookAfter<Raw::GetVisualTags>(&OnGetVisualTags))
        throw std::runtime_error("Failed to hook [AppearanceNameVisualTagsPreset::GetVisualTags].");

    return true;
}

bool App::VisualTagsModule::Unload()
{
    Unhook<Raw::GetVisualTags>();

    return true;
}

void App::VisualTagsModule::OnGetVisualTags(RED4ext::game::AppearanceNameVisualTagsPreset& aPreset,
                                            RED4ext::ResourcePath aEntityPath,
                                            RED4ext::CName aAppearanceName,
                                            RED4ext::red::TagList& aOutTags)
{
    static Core::Map<uint64_t, RED4ext::red::TagList> s_autoTagsCache;
    static RED4ext::red::TagList s_emptyTags;

    if (aOutTags.tags.size > 0 || !aAppearanceName)
        return;

    auto cacheKey = RED4ext::FNV1a64(aAppearanceName.ToString(), aEntityPath.hash);
    auto cachedTagsIt = s_autoTagsCache.find(cacheKey);

    if (cachedTagsIt != s_autoTagsCache.end())
    {
        Utils::CopyTags(aOutTags, cachedTagsIt->second);
        return;
    }

    auto loader = RED4ext::ResourceLoader::Get();

    auto entityToken = loader->FindToken<RED4ext::ent::EntityTemplate>(aEntityPath);

    if (!entityToken || !entityToken->IsLoaded())
    {
        s_autoTagsCache.emplace(cacheKey, s_emptyTags);
        return;
    }

    auto appearance = Utils::FindAppearanceTemplate(*entityToken, aAppearanceName);

    if (!appearance)
    {
        s_autoTagsCache.emplace(cacheKey, s_emptyTags);
        return;
    }

    auto appearancePath = appearance->appearanceResource.path;
    auto appearanceToken = loader->FindToken<RED4ext::appearance::AppearanceResource>(appearancePath);

    if (!appearanceToken || !appearanceToken->IsLoaded())
    {
        s_autoTagsCache.emplace(cacheKey, s_emptyTags);
        return;
    }

    auto appearanceDefinition = Utils::FindAppearanceDefinition(*appearanceToken, appearance->appearanceName);

    if (!appearanceDefinition)
    {
        s_autoTagsCache.emplace(cacheKey, s_emptyTags);
        return;
    }

    RED4ext::red::TagList autoTags;
    Utils::CopyTags(autoTags, appearanceDefinition->visualTags);

    cachedTagsIt = s_autoTagsCache.emplace(cacheKey, autoTags).first;

    Utils::CopyTags(aOutTags, cachedTagsIt->second);
}
