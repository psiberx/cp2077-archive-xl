#include "Module.hpp"
#include "App/Extensions/Localization/Language.hpp"
#include "Red/Localization.hpp"

namespace
{
constexpr auto ModuleName = "Localization";
}

std::string_view App::LocalizationModule::GetName()
{
    return ModuleName;
}

bool App::LocalizationModule::Load()
{
    if (!HookAfter<Raw::Localization::LoadOnScreens>(&LocalizationModule::OnLoadTexts))
        throw std::runtime_error("Failed to hook [Localization::LoadOnScreens].");

    if (!HookAfter<Raw::Localization::LoadSubtitles>(&LocalizationModule::OnLoadSubtitles))
        throw std::runtime_error("Failed to hook [Localization::LoadSubtitles].");

    return true;
}

bool App::LocalizationModule::Unload()
{
    Unhook<Raw::Localization::LoadOnScreens>();
    Unhook<Raw::Localization::LoadSubtitles>();

    return true;
}

void App::LocalizationModule::OnLoadTexts(Red::Handle<TextResource>& aOnScreens,
                                              Red::ResourcePath aPath)
{
    auto language = Language::ResolveFromTextResource(aPath);

    LogInfo("|{}| Initializing translations for \"{}\" language...", ModuleName, language.ToString());

    auto mergedAny = true;
    auto successAll = true;

    if (!m_units.empty())
    {
        auto& entryList = aOnScreens->entries;
        auto originalCount = entryList.size;
        auto originalMaxKey = (entryList.End() - 1)->primaryKey;
        auto usedKeyMap = TextEntryMap();

        for (const auto& unit : m_units)
        {
            if (unit.onscreens.empty())
                continue;

            auto paths = unit.onscreens.find(language);
            auto fallback = false;

            if (paths != unit.onscreens.end())
            {
                LogInfo("|{}| Processing \"{}\"...", ModuleName, unit.name);
            }
            else
            {
                fallback = true;
                paths = unit.onscreens.find(unit.fallback);
                LogInfo("|{}| Processing \"{}\" using fallback language \"{}\"...",
                        ModuleName, unit.name, unit.fallback.ToString());
            }

            for (const auto& path : paths->second)
            {
                successAll &= MergeTextResource(path, entryList, usedKeyMap, originalCount, originalMaxKey, fallback);
            }

            if (!fallback && language != unit.fallback)
            {
                fallback = true;
                paths = unit.onscreens.find(unit.fallback);

                for (const auto& path : paths->second)
                {
                    successAll &=
                        MergeTextResource(path, entryList, usedKeyMap, originalCount, originalMaxKey, fallback);
                }
            }

            mergedAny = true;
        }
    }

    if (!mergedAny)
        LogInfo("|{}| No translations to merge.", ModuleName);
    else if (successAll)
        LogInfo("|{}| All translations merged.", ModuleName);
    else
        LogWarning("|{}| Some translations merged with issues.", ModuleName);
}

bool App::LocalizationModule::MergeTextResource(const std::string& aPath, TextEntryList& aFinalList,
                                                TextEntryMap& aUsedKeyMap, uint32_t aOriginalCount,
                                            uint64_t aOriginalMaxKey, bool aFallback)
{
    Red::Handle<TextResource> resource;
    Raw::Localization::LoadOnScreens(resource, aPath.c_str());

    if (!resource.instance)
    {
        LogError("|{}| Resource \"{}\" failed to load.", ModuleName, aPath);
        return false;
    }

    LogInfo("|{}| Merging entries from \"{}\"...", ModuleName, aPath);

    bool success = true;
    auto& newEntries = resource.GetPtr()->entries;

    for (uint32_t i = 0; i < newEntries.size; ++i)
    {
        auto& newEntry = newEntries[i];

        if (newEntry.primaryKey == 0)
        {
            if (newEntry.secondaryKey.Length() == 0)
            {
                LogWarning("|{}| Item #{} has no primary or secondary key. Skipped.", ModuleName, i);
                success = false;
                continue;
            }

            newEntry.primaryKey = Red::FNV1a32(newEntry.secondaryKey.c_str());
            MergeTextEntry(aFinalList, newEntry, i, aUsedKeyMap, aOriginalCount, aOriginalMaxKey, aFallback);

            newEntry.primaryKey = Red::FNV1a64(newEntry.secondaryKey.c_str());
            newEntry.secondaryKey = "";
            MergeTextEntry(aFinalList, newEntry, i, aUsedKeyMap, aOriginalCount, aOriginalMaxKey, aFallback);
        }
        else
        {
            MergeTextEntry(aFinalList, newEntry, i, aUsedKeyMap, aOriginalCount, aOriginalMaxKey, aFallback);
        }
    }

    return success;
}

void App::LocalizationModule::MergeTextEntry(TextEntryList& aFinalList, TextEntry& aNewEntry, uint32_t aIndex,
                                             TextEntryMap& aUsedKeyMap, uint32_t aOriginalCount,
                                         uint64_t aOriginalMaxKey, bool aFallback)
{
    const auto& existingIt = aUsedKeyMap.find(aNewEntry.primaryKey);

    if (existingIt == aUsedKeyMap.end())
    {
        if (aNewEntry.primaryKey <= aOriginalMaxKey)
        {
            auto* originalEntry = FindSameTextEntry(aNewEntry, aFinalList, aOriginalCount);
            if (originalEntry)
            {
                if (!aFallback)
                {
                    *originalEntry = aNewEntry;
                    //aUsedKeyMap.emplace(originalEntry->primaryKey, originalEntry);
                }
                return;
            }
        }

        aFinalList.EmplaceBack(aNewEntry);
        aUsedKeyMap.emplace(aNewEntry.primaryKey, aFinalList.End() - 1);
    }
    else if (!aFallback)
    {
        auto* originalEntry = existingIt.value();

        if (originalEntry->secondaryKey.Length() == 0)
            LogWarning("|{}| Item #{} overwrites entry {}.",
                       ModuleName, aIndex, originalEntry->primaryKey);
        else
            LogWarning("|{}| Item #{} overwrites entry {} aka {}.",
                       ModuleName, aIndex, originalEntry->primaryKey, originalEntry->secondaryKey.c_str());

        *originalEntry = aNewEntry;
    }
}

App::TextEntry* App::LocalizationModule::FindSameTextEntry(TextEntry& aEntry, TextEntryList& aList,
                                                           uint32_t aCount)
{
    auto end = aList.Begin() + aCount;
    auto it = std::lower_bound(aList.Begin(), end, aEntry,
        [](const TextEntry& a, const TextEntry& b) -> bool {
            return a.primaryKey < b.primaryKey;
        });

    if (it == end || it->primaryKey != aEntry.primaryKey)
        return nullptr;

    return it;
}

void App::LocalizationModule::OnLoadSubtitles(Red::Handle<SubtitleResource>& aSubtitles, Red::ResourcePath aPath)
{
    auto language = Language::ResolveFromTextResource(aPath);

    LogInfo("|{}| Initializing subtitles for \"{}\" language...", ModuleName, language.ToString());

    auto mergedAny = true;
    auto successAll = true;

    if (!m_units.empty())
    {
        auto& entryList = aSubtitles->entries;

        for (const auto& unit : m_units)
        {
            if (unit.subtitles.empty())
                continue;

            auto paths = unit.subtitles.find(language);
            auto fallback = false;

            if (paths != unit.subtitles.end())
            {
                LogInfo("|{}| Processing \"{}\"...", ModuleName, unit.name);
            }
            else
            {
                fallback = true;
                paths = unit.subtitles.find(unit.fallback);
                LogInfo("|{}| Processing \"{}\" using fallback language \"{}\"...",
                        ModuleName, unit.name, unit.fallback.ToString());
            }

            for (const auto& path : paths->second)
            {
                successAll &= MergeSubtitleResource(path, entryList);
            }

            mergedAny = true;
        }
    }

    if (!mergedAny)
        LogInfo("|{}| No subtitles to merge.", ModuleName);
    else if (successAll)
        LogInfo("|{}| All subtitles merged.", ModuleName);
    else
        LogWarning("|{}| Some subtitles merged with issues.", ModuleName);
}

bool App::LocalizationModule::MergeSubtitleResource(const std::string& aPath, App::SubtitleEntryList& aFinalList)
{
    Red::Handle<SubtitleResource> resource;
    Raw::Localization::LoadSubtitles(resource, aPath.c_str());

    if (!resource.instance)
    {
        LogError("|{}| Resource \"{}\" failed to load.", ModuleName, aPath);
        return false;
    }

    LogInfo("|{}| Merging entries from \"{}\"...", ModuleName, aPath);

    for (const auto& newEntry : resource->entries)
    {
        aFinalList.PushBack(newEntry);
    }

    return true;
}
