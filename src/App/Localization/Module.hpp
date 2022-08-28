#pragma once

#include "Raws.hpp"
#include "Unit.hpp"
#include "App/Common/ModuleBase.hpp"

namespace App
{
using OnScreenEntry = RED4ext::loc::alization::PersistenceOnScreenEntry;
using OnScreenEntries = RED4ext::loc::alization::PersistenceOnScreenEntries;
using OnScreenEntryList = decltype(std::declval<OnScreenEntries>().entries);
using OnScreenEntryMap = Core::Map<decltype(std::declval<OnScreenEntry>().primaryKey), OnScreenEntry*>;

class LocalizationModule : public ConfigurableUnitModule<LocalizationUnit>
{
public:
    std::string_view GetName() override;
    bool Load() override;
    bool Unload() override;

private:

    uint64_t OnLoadOnScreens(RED4ext::Handle<OnScreenEntries>* aOnScreens, RED4ext::ResourcePath aPath);
    static bool AppendEntries(const std::string& aPath, OnScreenEntryList& aFinalList, OnScreenEntryMap& aUsedKeyMap,
                              uint32_t aOriginalCount, uint64_t aOriginalMaxKey);
    static OnScreenEntry* FindEntry(OnScreenEntry& aEntry, OnScreenEntryList& aList, uint32_t aCount);

    inline static Raw::LoadOnScreens::type LoadOnScreens;
};
}
