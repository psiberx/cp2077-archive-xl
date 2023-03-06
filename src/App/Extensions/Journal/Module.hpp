#pragma once

#include "App/Extensions/ModuleBase.hpp"
#include "App/Extensions/Journal/Unit.hpp"
#include "Red/JournalManager.hpp"
#include "Red/JournalTree.hpp"
#include "Red/MappinResource.hpp"

namespace App
{
class JournalModule : public ConfigurableUnitModule<JournalUnit>
{
public:
    std::string_view GetName() override;
    bool Load() override;
    void Reload() override;
    bool Unload() override;

private:
    using EntrySearchResult = std::tuple<Red::game::JournalEntry*, std::string, Red::game::JournalContainerEntry*, bool>;

    void OnLoadJournal(uintptr_t a1, Red::JobGroup& aJobGroup);
    void OnInitializeRoot(Red::game::JournalRootFolderEntry* aRoot, uintptr_t, uintptr_t, Red::JobQueue& aJobQueue);
    Red::game::CookedMappinData* OnGetMappinData(Red::game::MappinResource* aResource, uint32_t aHash);

    EntrySearchResult FindEntry(Red::game::JournalEntry* aParent, Red::CString& aPath);
    bool MergeEntries(Red::game::JournalContainerEntry* aTarget, Red::game::JournalContainerEntry* aSource,
                      const std::string& aPath = "");
    bool MergeEntry(Red::game::JournalEntry* aTarget, Red::game::JournalEntry* aSource,
                    const std::string& aPath, bool aEditProps = false);
    void ProcessNewEntries(Red::game::JournalEntry* aEntry, const std::string& aPath, bool aRecursive);
    void ConvertLocKeys(Red::game::JournalEntry* aEntry);
    void CollectMappin(Red::game::JournalEntry* aEntry, const std::string& aPath);
    void ResetResourceData();
    void ResetRuntimeData();
    void ReloadJournal();

    static std::string MakePath(const std::string& aPath, const std::string& aStep);
    static uint32_t CalculateJournalHash(const std::string& aPath);

    Core::Vector<Red::SharedPtr<Red::ResourceToken<Red::game::JournalResource>>> m_resources;
    Core::Map<Red::ResourcePath, std::string> m_paths;
    Core::Map<uint32_t, Red::game::JournalQuestMapPin*> m_mappins;
    std::shared_mutex m_mappinsLock;
};
}
