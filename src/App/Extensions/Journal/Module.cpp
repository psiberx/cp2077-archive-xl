#include "Module.hpp"
#include "Red/JobHandle.hpp"
#include "Red/Localization.hpp"
#include "Red/Rtti/Locator.hpp"

namespace
{
constexpr auto ModuleName = "Journal";

constexpr auto PathSeparator = '/';
constexpr auto EditMarker = '*';
constexpr auto IdentityProp = Red::CName("id");
}

std::string_view App::JournalModule::GetName()
{
    return ModuleName;
}

bool App::JournalModule::Load()
{
    if (!HookBefore<Raw::JournalTree::ProcessJournalIndex>(&JournalModule::OnLoadJournal))
        throw std::runtime_error("Failed to hook [JournalTree::ProcessJournalIndex].");

    if (!HookBefore<Raw::JournalRootFolderEntry::Initialize>(&JournalModule::OnInitializeRoot))
        throw std::runtime_error("Failed to hook [JournalRootFolderEntry::Initialize].");

    return true;
}

bool App::JournalModule::Unload()
{
    Unhook<Raw::JournalTree::ProcessJournalIndex>();
    Unhook<Raw::JournalRootFolderEntry::Initialize>();

    return true;
}

void App::JournalModule::Reload()
{
    if (!m_units.empty() && m_resources.empty())
    {
        ReloadJournal();
    }
}

void App::JournalModule::OnLoadJournal(uintptr_t a1, Red::JobGroup& aJobGroup)
{
    LogInfo("|{}| Journal tree is initializing...", ModuleName);

    if (m_units.empty())
    {
        LogInfo("|{}| No entries to merge.", ModuleName);
        return;
    }

    auto queue = Red::JobQueue(aJobGroup);
    auto loader = Red::ResourceLoader::Get();

    for (const auto& unit : m_units)
    {
        LogInfo("|{}| Processing \"{}\"...", ModuleName, unit.name);

        for (const auto& path : unit.journals)
        {
            auto token = loader->LoadAsync<Red::game::JournalResource>(path.c_str());

            if (!token->IsLoaded())
            {
                queue.Wait(token->job);
            }

            m_resources.emplace_back(token);
            m_paths.emplace(token->path, path);
        }
    }
}

void App::JournalModule::OnInitializeRoot(Red::game::JournalRootFolderEntry* aJournalRoot, uintptr_t, uintptr_t,
                                          Red::JobQueue& aJobQueue)
{
    static const auto s_rootEntryType = Red::Rtti::GetClass<Red::game::JournalRootFolderEntry>();

    if (m_units.empty())
        return;

    auto successAll = true;

    for (const auto& resource : m_resources)
    {
        if (resource->IsFailed())
        {
            LogError("|{}| Resource \"{}\" failed to load.", ModuleName, m_paths[resource->path]);
            successAll = false;
            continue;
        }

        auto root = resource->Get()->entry.GetPtr<Red::game::JournalContainerEntry>();

        if (!root->GetType()->IsA(s_rootEntryType))
        {
            LogError("|{}| Resource \"{}\" root entry must be {}.",
                     ModuleName, m_paths[resource->path], s_rootEntryType->GetName().ToString());
            successAll = false;
            continue;
        }

        LogInfo("|{}| Merging entries from \"{}\"...", ModuleName, m_paths[resource->path]);

        successAll &= MergeEntries(aJournalRoot, root);
    }

    ResetResources();

    if (successAll)
        LogInfo("|{}| All journal entries merged.", ModuleName);
    else
        LogWarning("|{}| Journal entries merged with issues.", ModuleName);

}

App::JournalModule::EntrySearchResult App::JournalModule::FindEntry(Red::game::JournalEntry* aParent,
                                                                    Red::CString& aPath)
{
    static const auto s_containerEntryType = Red::Rtti::GetClass<Red::game::JournalContainerEntry>();

    if (!aParent)
        return {};

    Red::game::JournalContainerEntry* parentEntry = nullptr;
    Red::game::JournalEntry* finalEntry = aParent;
    bool markedForEdit = false;

    std::stringstream ss(aPath.c_str());
    std::string id;

    while (std::getline(ss, id, PathSeparator))
    {
        if (!finalEntry->GetType()->IsA(s_containerEntryType))
            return {};

        parentEntry = reinterpret_cast<Red::game::JournalContainerEntry*>(finalEntry);

        if (ss.eof() && id.ends_with(EditMarker))
        {
            id.erase(id.end() - 1);
            markedForEdit = true;
        }

        auto it = std::find_if(parentEntry->entries.Begin(), parentEntry->entries.End(),
            [&id](const Red::Handle<Red::game::JournalEntry>& aEntry) -> bool
            {
               return strcmp(aEntry->id.c_str(), id.c_str()) == 0;
            });

        if (it == parentEntry->entries.End())
        {
            return {nullptr, ss.eof() ? parentEntry : nullptr, markedForEdit};
        }

        finalEntry = it->GetPtr();
    }

    return {finalEntry, parentEntry, markedForEdit};
}

bool App::JournalModule::MergeEntries(Red::game::JournalContainerEntry* aTarget,
                                      Red::game::JournalContainerEntry* aSource,
                                      const std::string& aFullPath)
{
    auto success = true;

    for (const auto& sourceEntry : aSource->entries)
    {
        auto [targetEntry, parentEntry, markedForEdit] = FindEntry(aTarget, sourceEntry->id);

        auto targetPath = aFullPath;
        if (!aFullPath.empty())
        {
            targetPath.push_back(PathSeparator);
        }
        targetPath.append(sourceEntry->id.c_str());

        if (targetEntry)
        {
            success &= MergeEntry(targetEntry, sourceEntry, targetPath, markedForEdit);
        }
        else if (parentEntry)
        {
            if (markedForEdit)
            {
                LogWarning("|{}| {}: Cannot modify entry, path not fould.", ModuleName, targetPath);
                success = false;
                continue;
            }

            ConvertLocKeys(sourceEntry, true);

            parentEntry->entries.EmplaceBack(sourceEntry);
        }
        else
        {
            LogWarning("|{}| {}: Path not fould.", ModuleName, targetPath);
            success = false;
        }
    }

    return success;
}

bool App::JournalModule::MergeEntry(Red::game::JournalEntry* aTarget, Red::game::JournalEntry* aSource,
                                    const std::string& aFullPath, bool aEditProps)
{
    static const auto s_containerEntryType = Red::Rtti::GetClass<Red::game::JournalContainerEntry>();

    auto success = true;

    if (aEditProps)
    {
        if (aTarget->GetType() == aSource->GetType())
        {
            ConvertLocKeys(aSource);

            Red::DynArray<Red::CProperty*> props;
            aTarget->GetType()->GetProperties(props);

            for (const auto& prop : props)
            {
                if (prop->name == IdentityProp)
                    continue;

                prop->SetValue(aTarget, prop->GetValuePtr<void>(aSource));
            }
        }
        else
        {
            LogWarning("|{}| {}: Cannot modify entry, type mismatch.", ModuleName, aFullPath);
            success = false;
        }
    }

    if (aTarget->GetType()->IsA(s_containerEntryType) && aSource->GetType()->IsA(s_containerEntryType))
    {
        success &= MergeEntries(reinterpret_cast<Red::game::JournalContainerEntry*>(aTarget),
                                reinterpret_cast<Red::game::JournalContainerEntry*>(aSource),
                                aFullPath);
    }

    return success;
}

void App::JournalModule::ConvertLocKeys(Red::game::JournalEntry* aEntry, bool aRecursive)
{
    static const auto s_localizationStringType = Red::Rtti::GetType<"LocalizationString">();
    static const auto s_containerEntryType = Red::Rtti::GetClass<Red::game::JournalContainerEntry>();

    Red::DynArray<Red::CProperty*> props;
    aEntry->GetType()->GetProperties(props);

    for (const auto& prop : props)
    {
        if (prop->type == s_localizationStringType)
        {
            auto value = prop->GetValuePtr<Red::LocalizationString>(aEntry);
            auto lockey = std::string(value->unk08.c_str());

            if (!lockey.starts_with(Red::LocKeyPrefix))
            {
                auto hash = Red::FNV1a64(lockey.c_str());

                lockey.clear();
                lockey.append(Red::LocKeyPrefix);
                lockey.append(std::to_string(hash));

                value->unk08 = lockey.c_str();
            }
        }
    }

    if (aRecursive && aEntry->GetType()->IsA(s_containerEntryType))
    {
        for (const auto& entry : reinterpret_cast<Red::game::JournalContainerEntry*>(aEntry)->entries)
        {
            ConvertLocKeys(entry, true);
        }
    }
}

void App::JournalModule::ResetResources()
{
    m_resources.clear();
    m_paths.clear();
}

void App::JournalModule::ReloadJournal()
{
    auto* manager = Red::CGameEngine::Get()->framework->gameInstance->GetInstance(
        Red::Rtti::GetClass<Red::game::IJournalManager>()
    );

    if (!manager)
        return;

    Red::Handle<Red::game::JournalEntry> null;
    Raw::JournalManager::TrackQuest(manager, null);
    Raw::JournalManager::TrackPointOfInterest(manager, null);

    Red::JobHandle job{};
    Raw::JournalManager::LoadJournal(manager, job);
    Raw::JobHandle::Wait(job);
}
