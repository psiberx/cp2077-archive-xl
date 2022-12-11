#pragma once

#include "App/Extensions/ModuleBase.hpp"
#include "App/Extensions/Localization/Unit.hpp"

namespace App
{
using OnScreenEntry = Red::loc::alization::PersistenceOnScreenEntry;
using OnScreenEntries = Red::loc::alization::PersistenceOnScreenEntries;
using OnScreenEntryList = decltype(std::declval<OnScreenEntries>().entries);
using OnScreenEntryMap = Core::Map<decltype(std::declval<OnScreenEntry>().primaryKey), OnScreenEntry*>;

class LocalizationModule : public ConfigurableUnitModule<LocalizationUnit>
{
public:
    std::string_view GetName() override;
    bool Load() override;
    bool Unload() override;

private:
    void OnLoadOnScreens(Red::Handle<OnScreenEntries>& aOnScreens, Red::ResourcePath aPath);
    static bool AppendEntries(const std::string& aPath, OnScreenEntryList& aFinalList, OnScreenEntryMap& aUsedKeyMap,
                              uint32_t aOriginalCount, uint64_t aOriginalMaxKey, bool aFallback);
    static OnScreenEntry* FindSameEntry(OnScreenEntry& aEntry, OnScreenEntryList& aList, uint32_t aCount);
};
}
