#pragma once

#include "App/Extensions/Journal/Unit.hpp"
#include "App/Extensions/ModuleBase.hpp"
#include "Red/JournalManager.hpp"
#include "Red/JournalTree.hpp"
#include "Red/MappinSystem.hpp"

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

    struct JournalMappin
    {
        Red::NodeRef reference;
        Red::Vector3 offset;
        bool isPointOfInterest;
    };

    void OnLoadJournal(uintptr_t a1, Red::JobGroup& aJobGroup);
    void OnInitializeRoot(Red::game::JournalRootFolderEntry* aRoot, uintptr_t, uintptr_t, Red::JobQueue& aJobQueue);
    void OnMappinDataLoaded(void* aMappinSystem, Red::worldRuntimeScene*);
    // void* OnGetMappinData(void* aMappinSystem, uint32_t aHash);
    // void* OnGetPoiData(void* aMappinSystem, uint32_t aHash);

    EntrySearchResult FindEntry(Red::game::JournalEntry* aParent, Red::CString& aPath);
    bool MergeEntries(Red::game::JournalContainerEntry* aTarget, Red::game::JournalContainerEntry* aSource,
                      const std::string& aPath = "");
    bool MergeEntry(Red::game::JournalEntry* aTarget, Red::game::JournalEntry* aSource,
                    const std::string& aPath, bool aEditProps = false);
    void ProcessNewEntries(Red::game::JournalEntry* aEntry, const std::string& aPath, bool aRecursive);
    void ConvertLocKeys(Red::game::JournalEntry* aEntry);
    void CollectMappin(Red::game::JournalEntry* aEntry, const std::string& aPath);
    static bool ResolveMappinPosition(uint32_t aHash, const JournalMappin& aMappin, Red::Vector3& aResult);
    void ResetResourceData();
    void ResetRuntimeData();
    void ReloadJournal();

    static std::string MakePath(const std::string& aPath, const std::string& aStep);
    static uint32_t CalculateJournalHash(const std::string& aPath);

    Core::Vector<Red::SharedPtr<Red::ResourceToken<Red::game::JournalResource>>> m_resources;
    Core::Map<Red::ResourcePath, std::string> m_paths;
    Core::Map<uint32_t, JournalMappin> m_mappins;
    std::shared_mutex m_mappinsLock;
};
}
