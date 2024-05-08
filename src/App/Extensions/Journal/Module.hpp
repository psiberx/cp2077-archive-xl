#pragma once

#include "App/Extensions/Journal/Config.hpp"
#include "App/Extensions/ModuleBase.hpp"
#include "Red/JournalManager.hpp"
#include "Red/JournalTree.hpp"
#include "Red/MappinSystem.hpp"

namespace App
{
class JournalModule : public ConfigurableModuleImpl<JournalConfig>
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
    static void OnMappinDataLoaded(void* aMappinSystem, Red::worldRuntimeScene*);
    static void* OnGetMappinData(void* aMappinSystem, uint32_t aHash);
    static void* OnGetPoiData(void* aMappinSystem, uint32_t aHash);

    static EntrySearchResult FindEntry(Red::game::JournalEntry* aParent, Red::CString& aPath);
    static bool MergeEntries(Red::game::JournalContainerEntry* aTarget, Red::game::JournalContainerEntry* aSource,
                      const std::string& aPath = "");
    static bool MergeEntry(Red::game::JournalEntry* aTarget, Red::game::JournalEntry* aSource,
                    const std::string& aPath, bool aEditProps = false);
    static void ProcessNewEntries(Red::game::JournalEntry* aEntry, const std::string& aPath, bool aRecursive);
    static void ConvertLocKeys(Red::game::JournalEntry* aEntry);
    static void CollectMappin(Red::game::JournalEntry* aEntry, const std::string& aPath);
    static void ResolveCookedMappin(void* aMappinSystem, uint32_t aHash, const JournalMappin& aJournalMappin,
                                    void*& aCookedMappin);
    static bool ResolveMappinPosition(uint32_t aJournalHash, const JournalMappin& aMappin, Red::Vector3& aResult);
    static void ResetResourceData();
    static void ResetRuntimeData();
    static void ReloadJournal();

    static std::string MakePath(const std::string& aPath, const std::string& aStep);
    static uint32_t CalculateJournalHash(const std::string& aPath);

    inline static Core::Vector<Red::SharedPtr<Red::ResourceToken<Red::game::JournalResource>>> s_resources;
    inline static Core::Map<Red::ResourcePath, std::string> s_paths;
    inline static Core::Map<uint32_t, JournalMappin> s_mappins;
    inline static std::shared_mutex s_mappinsLock;
};
}
