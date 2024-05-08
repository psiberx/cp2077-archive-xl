#include "Module.hpp"
#include "App/Utils/Registers.hpp"
#include "Red/JobHandle.hpp"
#include "Red/Localization.hpp"

namespace
{
constexpr auto ModuleName = "Journal";

constexpr auto PathSeparator = '/';
constexpr auto EditMarker = '*';
constexpr auto IdentityProp = Red::CName("id");

const Red::ClassLocator<Red::gameJournalQuestMapPin> s_questMappinType;
const Red::ClassLocator<Red::gameJournalQuestMapPinBase> s_questMappinBaseType;
const Red::ClassLocator<Red::gameJournalPointOfInterestMappin> s_pointOfInterestType;
const Red::TypeLocator<"LocalizationString"> s_localizationStringType;
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

    if (!HookAfter<Raw::MappinSystem::OnStreamingWorldLoaded>(&JournalModule::OnMappinDataLoaded))
        throw std::runtime_error("Failed to hook [MappinSystem::OnStreamingWorldLoaded].");

    if (!Hook<Raw::MappinSystem::GetMappinData>(&JournalModule::OnGetMappinData))
        throw std::runtime_error("Failed to hook [MappinSystem::GetMappinData].");

    if (!Hook<Raw::MappinSystem::GetPoiData>(&JournalModule::OnGetPoiData))
        throw std::runtime_error("Failed to hook [MappinSystem::GetPoiData].");

    return true;
}

bool App::JournalModule::Unload()
{
    Unhook<Raw::JournalTree::ProcessJournalIndex>();
    Unhook<Raw::JournalRootFolderEntry::Initialize>();
    Unhook<Raw::MappinSystem::OnStreamingWorldLoaded>();
    Unhook<Raw::MappinSystem::GetMappinData>();
    Unhook<Raw::MappinSystem::GetPoiData>();

    return true;
}

void App::JournalModule::Reload()
{
    if (!m_configs.empty())
    {
        ResetResourceData();
        ResetRuntimeData();
        ReloadJournal();
    }
}

void App::JournalModule::OnLoadJournal(uintptr_t a1, Red::JobGroup& aJobGroup)
{
    LogInfo("|{}| Journal tree is initializing...", ModuleName);

    if (m_configs.empty())
    {
        LogInfo("|{}| No entries to merge.", ModuleName);
        return;
    }

    auto queue = Red::JobQueue(aJobGroup);
    auto loader = Red::ResourceLoader::Get();

    for (const auto& unit : m_configs)
    {
        LogInfo("|{}| Processing \"{}\"...", ModuleName, unit.name);

        for (const auto& path : unit.journals)
        {
            auto token = loader->LoadAsync<Red::game::JournalResource>(path.c_str());

            if (!token->IsLoaded())
            {
                queue.Wait(token->job);
            }

            s_resources.emplace_back(token);
            s_paths.emplace(token->path, path);
        }
    }
}

void App::JournalModule::OnInitializeRoot(Red::game::JournalRootFolderEntry* aJournalRoot, uintptr_t, uintptr_t,
                                          Red::JobQueue& aJobQueue)
{
    static const auto s_rootEntryType = Red::GetClass<Red::game::JournalRootFolderEntry>();

    if (m_configs.empty())
        return;

    auto successAll = true;
    auto mergedAny = false;

    for (const auto& resource : s_resources)
    {
        if (resource->IsFailed())
        {
            LogError("|{}| Resource \"{}\" failed to load.", ModuleName, s_paths[resource->path]);
            successAll = false;
            continue;
        }

        auto root = resource->Get()->entry.GetPtr<Red::game::JournalContainerEntry>();

        if (!root->GetType()->IsA(s_rootEntryType))
        {
            LogError("|{}| Resource \"{}\" root entry must be {}.",
                     ModuleName, s_paths[resource->path], s_rootEntryType->GetName().ToString());
            successAll = false;
            continue;
        }

        if (root->id != aJournalRoot->id)
            continue;

        LogInfo("|{}| Merging entries from \"{}\"...", ModuleName, s_paths[resource->path]);

        successAll &= MergeEntries(aJournalRoot, root);
        mergedAny = true;
    }

    if (mergedAny)
    {
        if (successAll)
            LogInfo("|{}| All journal entries merged.", ModuleName);
        else
            LogWarning("|{}| Journal entries merged with issues.", ModuleName);
    }
}

void App::JournalModule::OnMappinDataLoaded(void* aMappinSystem, Red::worldRuntimeScene*)
{
    auto cookedMappinResource = Raw::MappinSystem::CookedMappinResource::Ptr(aMappinSystem)->instance;
    if (cookedMappinResource->cookedData.size && cookedMappinResource->cookedData.size == cookedMappinResource->cookedData.capacity)
    {
        {
            const auto reserve = std::max(s_mappins.size() << 1, cookedMappinResource->cookedData.size / 2ull);
            cookedMappinResource->cookedData.Reserve(cookedMappinResource->cookedData.size + reserve);
        }
        {
            const auto reserve = std::max(s_mappins.size() << 1, cookedMappinResource->cookedMultiData.size / 2ull);
            cookedMappinResource->cookedMultiData.Reserve(cookedMappinResource->cookedMultiData.size + reserve);
        }
    }

    auto cookedPoiResource = Raw::MappinSystem::CookedPoiResource::Ptr(aMappinSystem)->instance;
    if (cookedPoiResource->cookedData.size && cookedPoiResource->cookedData.size == cookedPoiResource->cookedData.capacity)
    {
        const auto reserve = std::max(s_mappins.size() << 1, cookedPoiResource->cookedData.size / 2ull);
        cookedPoiResource->cookedData.Reserve(cookedPoiResource->cookedData.size + reserve);
    }
}

void* App::JournalModule::OnGetMappinData(void* aMappinSystem, uint32_t aHash)
{
    auto result = Raw::MappinSystem::GetMappinData(aMappinSystem, aHash);

    if (!result && !s_mappins.empty())
    {
        auto r8 = get_r8();
        auto r9 = get_r9();

        const auto it = s_mappins.find(aHash);
        if (it != s_mappins.end())
        {
            ResolveCookedMappin(aMappinSystem, aHash, it.value(), result);
        }

        set_r8(r8);
        set_r9(r9);
    }

    return result;
}

void* App::JournalModule::OnGetPoiData(void* aMappinSystem, uint32_t aHash)
{
    auto result = Raw::MappinSystem::GetPoiData(aMappinSystem, aHash);

    if (!result && !s_mappins.empty())
    {
        auto r8 = get_r8();
        auto r9 = get_r9();

        const auto it = s_mappins.find(aHash);
        if (it != s_mappins.end())
        {
            ResolveCookedMappin(aMappinSystem, aHash, it.value(), result);
        }

        set_r8(r8);
        set_r9(r9);
    }

    return result;
}

void App::JournalModule::ResolveCookedMappin(void* aMappinSystem, uint32_t aHash, const JournalMappin& aJournalMappin,
                                             void*& aCookedMappin)
{
    if (aJournalMappin.isPointOfInterest)
    {
        Red::gameCookedPointOfInterestMappinData cookedMappin{};

        if (ResolveMappinPosition(aHash, aJournalMappin, cookedMappin.position))
        {
            cookedMappin.journalPathHash = aHash;
            cookedMappin.entityID.hash = aJournalMappin.reference.hash;

            std::unique_lock _(s_mappinsLock);
            auto resource = Raw::MappinSystem::CookedPoiResource::Ptr(aMappinSystem)->instance;
            resource->cookedData.PushBack(std::move(cookedMappin));

            aCookedMappin = resource->cookedData.End() - 1;
        }
    }
    else
    {
        Red::gameCookedMappinData cookedMappin{};

        if (ResolveMappinPosition(aHash, aJournalMappin, cookedMappin.position))
        {
            cookedMappin.journalPathHash = aHash;

            std::unique_lock _(s_mappinsLock);
            auto resource = Raw::MappinSystem::CookedMappinResource::Ptr(aMappinSystem)->instance;
            resource->cookedData.PushBack(std::move(cookedMappin));

            aCookedMappin = resource->cookedData.End() - 1;
        }
    }
}

bool App::JournalModule::ResolveMappinPosition(uint32_t aHash, const JournalMappin& aMappin, Red::Vector3& aResult)
{
    LogInfo("|{}| Cooked mappin #{} requested...", ModuleName, aHash);

    if (aMappin.reference.hash)
    {
        Red::world::GlobalNodeRef context{};
        Red::ExecuteFunction("worldGlobalNodeID", "GetRoot", &context);

        if (!context.hash)
        {
            LogError("|{}| Can't resolve mappin #{} context.", ModuleName, aHash);
            return false;
        }

        Red::NodeRef reference = aMappin.reference;
        Red::world::GlobalNodeRef resolved{};
        Red::ExecuteGlobalFunction("ResolveNodeRef", &resolved, reference, context);

        if (!resolved.hash)
        {
            LogError("|{}| Can't resolve mappin #{} reference.", ModuleName, aHash);
            return false;
        }

        bool success{};
        Red::Transform transform{};
        Red::ScriptGameInstance game{};
        Red::ExecuteFunction("ScriptGameInstance", "GetNodeTransform", &success, game, resolved, transform);

        if (!success)
        {
            LogError("|{}| Can't resolve mappin #{} position.", ModuleName, aHash);
            return false;
        }

        aResult.X = transform.position.X;
        aResult.Y = transform.position.Y;
        aResult.Z = transform.position.Z;

        LogInfo("|{}| Cooked mappin #{} resolved to NodeRef #{}.",  ModuleName, aHash, resolved.hash);
    }
    else
    {
        aResult = aMappin.offset;

        LogInfo("|{}| Cooked mappin #{} resolved to static offset.", ModuleName, aHash);
    }

    return true;
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
    auto entryType = aEntry->GetType();

    if (entryType->IsA(s_questMappinType))
    {
        auto hash = CalculateJournalHash(aPath);
        auto entry = reinterpret_cast<Red::gameJournalQuestMapPin*>(aEntry);

        if (!entry->reference.dynamicEntityUniqueName)
        {
            s_mappins.insert({hash, {entry->reference.reference, entry->offset, false}});

            if (!entry->reference.reference.hash)
            {
                entry->offset = {};
            }
        }
    }
    else if (entryType->IsA(s_questMappinBaseType))
    {
        auto hash = CalculateJournalHash(aPath);
        auto entry = reinterpret_cast<Red::gameJournalQuestMapPinBase*>(aEntry);

        s_mappins.insert({hash, {0ull, entry->offset, false}});
    }
    else if (entryType->IsA(s_pointOfInterestType))
    {
        auto hash = CalculateJournalHash(aPath);
        auto entry = reinterpret_cast<Red::gameJournalPointOfInterestMappin*>(aEntry);

        s_mappins.insert({hash, {entry->staticNodeRef, entry->offset, true}});

        if (!entry->staticNodeRef.hash)
        {
            entry->offset = {};
        }
    }
}

void App::JournalModule::ResetResourceData()
{
    s_resources.clear();
    s_paths.clear();
}

void App::JournalModule::ResetRuntimeData()
{
    s_mappins.clear();
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
    return Red::Murmur3_32(reinterpret_cast<const uint8_t*>(aPath.data()), aPath.length());
}
