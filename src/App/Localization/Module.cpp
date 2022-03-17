#include "Module.hpp"
#include "Language.hpp"

namespace
{
constexpr auto ModuleName = "Localization";
}

std::string_view App::LocalizationModule::GetName()
{
    return ModuleName;
}

bool App::LocalizationModule::Attach()
{
    if (!Hook<Raw::LoadOnScreens>(&LocalizationModule::OnLoadOnScreens, &LocalizationModule::LoadOnScreens))
    {
        LogError("|{}| Failed to hook localization loader.", ModuleName);
        return false;
    }

    return true;
}

bool App::LocalizationModule::Detach()
{
    if (!Unhook<Raw::LoadOnScreens>())
    {
        LogError("|{}| Failed to unhook localization loader.", ModuleName);
        return false;
    }

    return true;
}

uint64_t App::LocalizationModule::OnLoadOnScreens(RED4ext::Handle<OnScreenEntries>* aOnScreens,
                                                  Engine::ResourcePath aPath)
{
    auto result = LoadOnScreens(aOnScreens, aPath);
    auto language = Language::ResolveFromResource(aPath);

    LogInfo("|{}| The localization system is initializing for the language [{}]...", ModuleName, language.ToString());

    if (!m_units.empty())
    {
        auto successAll = true;
        auto& entryList = (*aOnScreens)->entries;
        auto originalCount = entryList.size;
        auto originalMaxKey = (entryList.End() - 1)->primaryKey;
        auto usedKeyMap = OnScreenEntryMap();

        for (const auto& unit : m_units)
        {
            auto paths = unit.onscreens.find(language);
            if (paths != unit.onscreens.end())
            {
                LogInfo("|{}| Processing [{}]...", ModuleName, unit.name);
            }
            else
            {
                paths = unit.onscreens.find(unit.fallback);
                LogInfo("|{}| Processing [{}] using fallback language [{}]...", ModuleName, unit.name, unit.fallback);
            }

            for (const auto& path : paths->second)
                successAll &= AppendEntries(path, entryList, usedKeyMap, originalCount, originalMaxKey);
        }

        if (successAll)
            LogInfo("|{}| All localization entries merged.", ModuleName);
        else
            LogWarning("|{}| Localization entries merged with issues.", ModuleName);
    }
    else
    {
        LogInfo("|{}| No localization entries to merge.", ModuleName);
    }

    return result;
}

bool App::LocalizationModule::AppendEntries(const std::string& aPath, OnScreenEntryList& aFinalList,
                                            OnScreenEntryMap& aUsedKeyMap, uint32_t aOriginalCount,
                                            uint64_t aOriginalMaxKey)
{
    RED4ext::Handle<OnScreenEntries> resource;
    LoadOnScreens(&resource, aPath.c_str());

    if (!resource.instance)
    {
        LogError("|{}| Resource [{}] failed to load.", ModuleName, aPath);
        return false;
    }

    LogInfo("|{}| Merging entries from [{}]...", ModuleName, aPath);

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

            newEntry.primaryKey = RED4ext::FNV1a64(newEntry.secondaryKey.c_str());
        }

        const auto& existingIt = aUsedKeyMap.find(newEntry.primaryKey);

        if (existingIt == aUsedKeyMap.end())
        {
            if (newEntry.primaryKey <= aOriginalMaxKey)
            {
                auto* originalEntry = FindEntry(newEntry, aFinalList, aOriginalCount);
                if (originalEntry)
                {
                    *originalEntry = newEntry;
                    aUsedKeyMap.emplace(originalEntry->primaryKey, originalEntry);
                    continue;
                }
            }

            aFinalList.EmplaceBack(newEntry);
            aUsedKeyMap.emplace(newEntry.primaryKey, &newEntry);
        }
        else
        {
            if (newEntry.secondaryKey.Length() == 0)
                LogWarning("|{}| Item #{} overwrites entry [{}].",
                           ModuleName, i, newEntry.primaryKey);
            else
                LogWarning("|{}| Item #{} overwrites entry [{}] aka [{}].",
                           ModuleName, i, newEntry.primaryKey, newEntry.secondaryKey.c_str());

            *existingIt.value() = newEntry;
        }
    }

    return success;
}

App::OnScreenEntry* App::LocalizationModule::FindEntry(OnScreenEntry& aEntry, OnScreenEntryList& aList, uint32_t aCount)
{
    auto end = aList.Begin() + aCount;
    auto it = std::lower_bound(aList.Begin(), end, aEntry,
        [](const OnScreenEntry& a, const OnScreenEntry& b) -> bool {
            return a.primaryKey < b.primaryKey;
        });

    if (it == end || it->primaryKey != aEntry.primaryKey)
        return nullptr;

    return it;
}
