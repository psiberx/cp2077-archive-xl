#include "Module.hpp"
#include "Red/JobHandle.hpp"
#include "Red/Localization.hpp"

namespace
{
constexpr auto ModuleName = "Journal";

constexpr auto MappinsResource = Red::ResourcePath(R"(base\worlds\03_night_city\_compiled\default\03_night_city.mappins)");

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

    if (!Hook<Raw::MappinResource::GetMappinData>(&JournalModule::OnGetMappinData))
        throw std::runtime_error("Failed to hook [MappinResource::GetMappinData].");

    return true;
}

bool App::JournalModule::Unload()
{
    Unhook<Raw::JournalTree::ProcessJournalIndex>();
    Unhook<Raw::JournalRootFolderEntry::Initialize>();
    Unhook<Raw::MappinResource::GetMappinData>();

    return true;
}

void App::JournalModule::Reload()
{
    if (!m_units.empty() && m_resources.empty())
    {
        ResetRuntimeData();
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
    static const auto s_rootEntryType = Red::GetClass<Red::game::JournalRootFolderEntry>();

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

    ResetResourceData();

    if (successAll)
        LogInfo("|{}| All journal entries merged.", ModuleName);
    else
        LogWarning("|{}| Journal entries merged with issues.", ModuleName);

}

Red::game::CookedMappinData* App::JournalModule::OnGetMappinData(Red::game::MappinResource* aResource, uint32_t aHash)
{
    if (aResource->cookedData.size == aResource->cookedData.capacity && aResource->path == MappinsResource)
    {
        const auto reserve = std::max(m_mappins.size() << 1, aResource->cookedData.size / 2ull);
        aResource->cookedData.Reserve(aResource->cookedData.size + reserve);
    }

    auto result = Raw::MappinResource::GetMappinData(aResource, aHash);

    if (!result && !m_mappins.empty() && aResource->path == MappinsResource)
    {
        const auto it = m_mappins.find(aHash);
        if (it != m_mappins.end())
        {
            // LogInfo("|{}| Uncooked mappin #{} requested...", ModuleName, aHash);

            Red::game::CookedMappinData cookedMappin{};
            cookedMappin.journalPathHash = aHash;

            const auto& mappin = it.value();

            if (mappin->reference.reference.unk00)
            {
                Red::world::GlobalNodeRef context{};
                Red::ExecuteFunction("worldGlobalNodeID", "GetRoot", &context);

                if (!context.hash)
                {
                    LogError("|{}| Can't resolve mappin #{} context.", ModuleName, aHash);
                    return nullptr;
                }

                Red::NodeRef reference = mappin->reference.reference;
                Red::world::GlobalNodeRef resolved{};
                Red::ExecuteGlobalFunction("ResolveNodeRef", &resolved, reference, context);

                if (!resolved.hash)
                {
                    LogError("|{}| Can't resolve mappin #{} reference.", ModuleName, aHash);
                    return nullptr;
                }

                bool success{};
                Red::Transform transform{};
                Red::ScriptGameInstance game{};
                Red::ExecuteFunction("ScriptGameInstance", "GetNodeTransform", &success, game, resolved, transform);

                if (!success)
                {
                    LogError("|{}| Can't resolve mappin #{} position.", ModuleName, aHash);
                    return nullptr;
                }

                cookedMappin.position.X = transform.position.X;
                cookedMappin.position.Y = transform.position.Y;
                cookedMappin.position.Z = transform.position.Z;
            }
            else
            {
                cookedMappin.position = mappin->offset;
                mappin->offset = {};
            }

            {
                std::unique_lock _(m_mappinsLock);
                aResource->cookedData.PushBack(std::move(cookedMappin));
                return aResource->cookedData.End() - 1;
            }
        }
    }

    return result;
}

App::JournalModule::EntrySearchResult App::JournalModule::FindEntry(Red::game::JournalEntry* aParent,
                                                                    Red::CString& aPath)
{
    static const auto s_containerEntryType = Red::GetClass<Red::game::JournalContainerEntry>();

    if (!aParent)
        return {};

    Red::game::JournalContainerEntry* parentEntry = nullptr;
    Red::game::JournalEntry* finalEntry = aParent;
    std::string finalPath;
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

        if (!finalPath.empty())
            finalPath += PathSeparator;

        finalPath += id;

        auto it = std::find_if(parentEntry->entries.Begin(), parentEntry->entries.End(),
            [&id](const Red::Handle<Red::game::JournalEntry>& aEntry) -> bool
            {
               return strcmp(aEntry->id.c_str(), id.c_str()) == 0;
            });

        if (it == parentEntry->entries.End())
        {
            return {nullptr, finalPath, ss.eof() ? parentEntry : nullptr, markedForEdit};
        }

        finalEntry = it->GetPtr();
    }

    return {finalEntry, finalPath, parentEntry, markedForEdit};
}

bool App::JournalModule::MergeEntries(Red::game::JournalContainerEntry* aTarget,
                                      Red::game::JournalContainerEntry* aSource,
                                      const std::string& aPath)
{
    auto success = true;

    for (const auto& sourceEntry : aSource->entries)
    {
        auto [targetEntry, targetId, parentEntry, markedForEdit] = FindEntry(aTarget, sourceEntry->id);
        auto targetPath = MakePath(aPath, targetId);

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

            ProcessNewEntries(sourceEntry, targetPath, true);

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
                                    const std::string& aPath, bool aEditProps)
{
    static const auto s_containerEntryType = Red::GetClass<Red::game::JournalContainerEntry>();

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
            LogWarning("|{}| {}: Cannot modify entry, type mismatch.", ModuleName, aPath);
            success = false;
        }
    }

    if (aTarget->GetType()->IsA(s_containerEntryType) && aSource->GetType()->IsA(s_containerEntryType))
    {
        success &= MergeEntries(reinterpret_cast<Red::game::JournalContainerEntry*>(aTarget),
                                reinterpret_cast<Red::game::JournalContainerEntry*>(aSource),
                                aPath);
    }

    return success;
}

void App::JournalModule::ProcessNewEntries(Red::game::JournalEntry* aEntry, const std::string& aPath, bool aRecursive)
{
    static const auto s_containerEntryType = Red::GetClass<Red::game::JournalContainerEntry>();

    ConvertLocKeys(aEntry);
    CollectMappin(aEntry, aPath);

    if (aRecursive && aEntry->GetType()->IsA(s_containerEntryType))
    {
        for (const auto& entry : reinterpret_cast<Red::game::JournalContainerEntry*>(aEntry)->entries)
        {
            ProcessNewEntries(entry, MakePath(aPath, entry->id.c_str()), true);
        }
    }
}

void App::JournalModule::ConvertLocKeys(Red::game::JournalEntry* aEntry)
{
    static const auto s_localizationStringType = Red::GetType<"LocalizationString">();

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
}

void App::JournalModule::CollectMappin(Red::game::JournalEntry* aEntry, const std::string& aPath)
{
    static const auto s_mappinEntryType = Red::GetClass<Red::game::JournalQuestMapPin>();

    if (aEntry->GetType()->IsA(s_mappinEntryType))
    {
        const auto hash = CalculateJournalHash(aPath);

        m_mappins.emplace(hash, reinterpret_cast<Red::game::JournalQuestMapPin*>(aEntry));
    }
}

void App::JournalModule::ResetResourceData()
{
    m_resources.clear();
    m_paths.clear();
}

void App::JournalModule::ResetRuntimeData()
{
    m_mappins.clear();
}

void App::JournalModule::ReloadJournal()
{
    auto manager = Red::GetGameSystem<Red::game::IJournalManager>();

    if (!manager)
        return;

    Red::JournalEntryHash questHash{};
    Red::JournalEntryHash poiHash{};

    {
        Red::Handle<Red::game::JournalEntry> quest;
        Red::Handle<Red::game::JournalEntry> poi;

        Raw::JournalManager::GetTrackedQuest(manager, quest);
        Raw::JournalManager::GetTrackedPointOfInterest(manager, poi);

        if (quest)
            questHash = Raw::JournalManager::GetEntryHash(manager, quest);

        if (poi)
            poiHash = Raw::JournalManager::GetEntryHash(manager, poi);
    }

    {
        Red::Handle<Red::game::JournalEntry> null;
        Raw::JournalManager::TrackQuest(manager, null);
        Raw::JournalManager::TrackPointOfInterest(manager, null);
    }

    Red::JobHandle job{};
    Raw::JournalManager::LoadJournal(manager, job);
    Raw::JobHandle::Wait(job);

    if (questHash)
    {
        Red::Handle<Red::game::JournalEntry> quest;
        Raw::JournalManager::GetEntryByHash(manager, quest, questHash);

        if (quest)
            Raw::JournalManager::TrackQuest(manager, quest);
    }

    if (poiHash)
    {
        Red::Handle<Red::game::JournalEntry> poi;
        Raw::JournalManager::GetEntryByHash(manager, poi, poiHash);

        if (poiHash)
            Raw::JournalManager::TrackPointOfInterest(manager, poi);
    }
}

std::string App::JournalModule::MakePath(const std::string& aPath, const std::string& aStep)
{
    return !aPath.empty() ? aPath + PathSeparator + aStep : aStep;
}

uint32_t App::JournalModule::CalculateJournalHash(const std::string& aPath)
{
    return Red::Murmur3_32(reinterpret_cast<const uint8_t*>(aPath.data()), aPath.length(), 0X5EEDBA5E);
}
