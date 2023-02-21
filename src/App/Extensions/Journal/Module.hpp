#pragma once

#include "App/Extensions/ModuleBase.hpp"
#include "App/Extensions/Journal/Unit.hpp"
#include "Red/JournalManager.hpp"
#include "Red/JournalTree.hpp"

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
    using EntrySearchResult = std::tuple<Red::game::JournalEntry*, Red::game::JournalContainerEntry*, bool>;

    void OnLoadJournal(uintptr_t a1, Red::JobGroup& aJobGroup);
    void OnInitializeRoot(Red::game::JournalRootFolderEntry* aRoot, uintptr_t, uintptr_t, Red::JobQueue& aJobQueue);

    EntrySearchResult FindEntry(Red::game::JournalEntry* aParent, Red::CString& aPath);
    bool MergeEntries(Red::game::JournalContainerEntry* aTarget, Red::game::JournalContainerEntry* aSource,
                      const std::string& aFullPath = "");
    bool MergeEntry(Red::game::JournalEntry* aTarget, Red::game::JournalEntry* aSource,
                    const std::string& aFullPath, bool aEditProps = false);
    void ConvertLocKeys(Red::game::JournalEntry* aEntry, bool aRecursive = false);
    void ResetResources();
    void ReloadJournal();

    Core::Vector<Red::SharedPtr<Red::ResourceToken<Red::game::JournalResource>>> m_resources;
    Core::Map<Red::ResourcePath, std::string> m_paths;
};
}
