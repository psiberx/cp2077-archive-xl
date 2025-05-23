#include "Extension.hpp"
#include "App/Extensions/Localization/Language.hpp"
#include "Red/Localization.hpp"

namespace
{
constexpr auto ExtensionName = "Localization";
}

std::string_view App::LocalizationExtension::GetName()
{
    return ExtensionName;
}

bool App::LocalizationExtension::Load()
{
    HookAfter<Raw::Localization::LoadTexts>(&LocalizationExtension::OnLoadTexts).OrThrow();
    HookAfter<Raw::Localization::LoadSubtitles>(&LocalizationExtension::OnLoadSubtitles).OrThrow();
    HookBefore<Raw::Localization::LoadVoiceOvers>(&LocalizationExtension::OnLoadVoiceOvers).OrThrow();
    HookAfter<Raw::Localization::LoadLipsyncs>(&LocalizationExtension::OnLoadLipsyncs).OrThrow();

    return true;
}

bool App::LocalizationExtension::Unload()
{
    Unhook<Raw::Localization::LoadTexts>();
    Unhook<Raw::Localization::LoadSubtitles>();
    Unhook<Raw::Localization::LoadVoiceOvers>();
    Unhook<Raw::Localization::LoadLipsyncs>();

    return true;
}

void App::LocalizationExtension::Configure()
{
    for (auto config = m_configs.begin(); config != m_configs.end();)
    {
        if (config->extend.empty())
        {
            ++config;
            continue;
        }

        const auto& target = std::ranges::find_if(m_configs, [&](auto& aConfig) { return aConfig.name == config->extend; });
        if (target != m_configs.end())
        {
            for (const auto& [language, resources] : config->onscreens)
                for (const auto& resource : resources)
                    target->onscreens[language].push_back(resource);

            for (const auto& [language, resources] : config->subtitles)
                for (const auto& resource : resources)
                    target->subtitles[language].push_back(resource);

            for (const auto& [language, resources] : config->lipmaps)
                for (const auto& resource : resources)
                    target->lipmaps[language].push_back(resource);

            for (const auto& [language, resources] : config->vomaps)
                for (const auto& resource : resources)
                    target->vomaps[language].push_back(resource);
        }

        config = m_configs.erase(config);
    }
}

void App::LocalizationExtension::OnLoadTexts(Red::Handle<TextResource>& aOnScreens, Red::ResourcePath aPath)
{
    const auto language = Language::ResolveFromTextResource(aPath);

    if (language.IsNone())
        return;

    LogInfo("[{}] Initializing texts for \"{}\" language...", ExtensionName, language.ToString());

    auto mergedAny = false;
    auto successAll = true;

    if (!m_configs.empty())
    {
        auto& entryList = aOnScreens->entries;
        auto originalCount = entryList.size;
        auto originalMaxKey = (entryList.End() - 1)->primaryKey;
        auto usedKeyMap = TextEntryMap();

        for (const auto& unit : m_configs)
        {
            if (unit.onscreens.empty())
                continue;

            auto fallback = false;
            auto paths = unit.onscreens.find(language);

            if (paths != unit.onscreens.end())
            {
                // LogInfo(R"([{}] Processing "{}"...)", ExtensionName, unit.name);
            }
            else
            {
                fallback = true;
                paths = unit.onscreens.find(unit.fallback);
                // LogInfo(R"([{}] Processing "{}" using fallback language "{}"...)",
                //         ExtensionName, unit.name, unit.fallback.ToString());
            }

            for (const auto& path : paths->second)
            {
                successAll &= MergeTextResource(path, entryList, usedKeyMap, originalCount, originalMaxKey, fallback);
            }

            if (!fallback && language != unit.fallback)
            {
                fallback = true;
                paths = unit.onscreens.find(unit.fallback);

                for (const auto& path : paths->second)
                {
                    successAll &=
                        MergeTextResource(path, entryList, usedKeyMap, originalCount, originalMaxKey, fallback);
                }
            }

            mergedAny = true;
        }
    }

    if (!mergedAny)
        LogInfo("[{}] No translations to merge.", ExtensionName);
    else if (successAll)
        LogInfo("[{}] All translations merged.", ExtensionName);
    else
        LogWarning("[{}] Translations merged with issues.", ExtensionName);
}

bool App::LocalizationExtension::MergeTextResource(const std::string& aPath, TextEntryList& aFinalList,
                                                TextEntryMap& aUsedKeyMap, uint32_t aOriginalCount,
                                                uint64_t aOriginalMaxKey, bool aFallback)
{
    Red::Handle<TextResource> resource;
    Raw::Localization::LoadTexts(resource, aPath.c_str());

    if (!resource.instance)
    {
        LogError("[{}] Resource \"{}\" failed to load.", ExtensionName, aPath);
        return false;
    }

    LogInfo("[{}] Merging entries from \"{}\"...", ExtensionName, aPath);

    bool success = true;
    auto& newEntries = resource.GetPtr()->entries;

    for (uint32_t i = 0; i < newEntries.size; ++i)
    {
        auto& newEntry = newEntries[i];

        if (IsCommentEntry(newEntry))
            continue;

        if (newEntry.primaryKey == 0)
        {
            if (newEntry.secondaryKey.Length() == 0)
            {
                LogWarning("[{}] Item #{} has no primary or secondary key. Skipped.", ExtensionName, i);
                success = false;
                continue;
            }

            newEntry.primaryKey = Red::FNV1a32(newEntry.secondaryKey.c_str());
            MergeTextEntry(aFinalList, newEntry, i, aUsedKeyMap, aOriginalCount, aOriginalMaxKey, aFallback);

            newEntry.primaryKey = Red::FNV1a64(newEntry.secondaryKey.c_str());
            newEntry.secondaryKey = "";
            MergeTextEntry(aFinalList, newEntry, i, aUsedKeyMap, aOriginalCount, aOriginalMaxKey, aFallback, true);
        }
        else
        {
            MergeTextEntry(aFinalList, newEntry, i, aUsedKeyMap, aOriginalCount, aOriginalMaxKey, aFallback);
        }
    }

    return success;
}

void App::LocalizationExtension::MergeTextEntry(TextEntryList& aFinalList, TextEntry& aNewEntry, uint32_t aIndex,
                                             TextEntryMap& aUsedKeyMap, uint32_t aOriginalCount,
                                             uint64_t aOriginalMaxKey, bool aFallback, bool aExtraEntry)
{
    const auto& existingIt = aUsedKeyMap.find(aNewEntry.primaryKey);

    if (existingIt == aUsedKeyMap.end())
    {
        if (aNewEntry.primaryKey <= aOriginalMaxKey)
        {
            auto* originalEntry = FindSameTextEntry(aNewEntry, aFinalList, aOriginalCount);
            if (originalEntry)
            {
                if (!aFallback)
                {
                    *originalEntry = aNewEntry;
                    //aUsedKeyMap.emplace(originalEntry->primaryKey, originalEntry);
                }
                return;
            }
        }

        aUsedKeyMap.emplace(aNewEntry.primaryKey, aFinalList.size);
        aFinalList.EmplaceBack(aNewEntry);
    }
    else if (!aFallback)
    {
        auto* originalEntry = aFinalList.Begin() + existingIt.value();

        if (!aExtraEntry)
        {
            if (originalEntry->secondaryKey.Length() == 0)
                LogWarning("[{}] Item #{} overwrites entry {}.",
                           ExtensionName, aIndex, originalEntry->primaryKey);
            else
                LogWarning("[{}] Item #{} overwrites entry {} (\"{}\").",
                           ExtensionName, aIndex, originalEntry->primaryKey, originalEntry->secondaryKey.c_str());
        }

        *originalEntry = aNewEntry;
    }
}

App::TextEntry* App::LocalizationExtension::FindSameTextEntry(TextEntry& aEntry, TextEntryList& aList, uint32_t aCount)
{
    auto end = aList.Begin() + aCount;
    auto it = std::lower_bound(aList.Begin(), end, aEntry,
        [](const TextEntry& a, const TextEntry& b) -> bool {
            return a.primaryKey < b.primaryKey;
        });

    if (it == end || it->primaryKey != aEntry.primaryKey)
        return nullptr;

    return it;
}

bool App::LocalizationExtension::IsCommentEntry(App::TextEntry& aEntry)
{
    if (aEntry.secondaryKey.Length() == 0)
        return false;

    const auto* keyStr = aEntry.secondaryKey.c_str();
    return keyStr && (*keyStr == '-' || *keyStr == '=');
}

void App::LocalizationExtension::OnLoadSubtitles(Red::Handle<SubtitleResource>& aSubtitles, Red::ResourcePath aPath)
{
    const auto language = Language::ResolveFromSubtitleResource(aPath);

    if (language.IsNone())
        return;

    LogInfo("[{}] Initializing subtitles for \"{}\" language...", ExtensionName, language.ToString());

    auto mergedAny = false;
    auto successAll = true;

    if (!m_configs.empty())
    {
        auto& entryList = aSubtitles->entries;

        for (const auto& unit : m_configs)
        {
            if (unit.subtitles.empty())
                continue;

            auto paths = unit.subtitles.find(language);
            auto fallback = false;

            if (paths != unit.subtitles.end())
            {
                // LogInfo("[{}] Processing \"{}\"...", ExtensionName, unit.name);
            }
            else
            {
                fallback = true;
                paths = unit.subtitles.find(unit.fallback);
                // LogInfo(R"([{}] Processing "{}" using fallback language "{}"...)",
                //         ExtensionName, unit.name, unit.fallback.ToString());
            }

            for (const auto& path : paths->second)
            {
                successAll &= MergeSubtitleResource(path, entryList);
            }

            mergedAny = true;
        }
    }

    if (!mergedAny)
        LogInfo("[{}] No subtitles to merge.", ExtensionName);
    else if (successAll)
        LogInfo("[{}] All subtitles merged.", ExtensionName);
    else
        LogWarning("[{}] Subtitles merged with issues.", ExtensionName);
}

bool App::LocalizationExtension::MergeSubtitleResource(const std::string& aPath, App::SubtitleEntryList& aFinalList)
{
    Red::Handle<SubtitleResource> resource;
    Raw::Localization::LoadSubtitles(resource, aPath.c_str());

    if (!resource.instance)
    {
        LogError("[{}] Resource \"{}\" failed to load.", ExtensionName, aPath);
        return false;
    }

    LogInfo("[{}] Merging entries from \"{}\"...", ExtensionName, aPath);

    for (const auto& newEntry : resource->entries)
    {
        aFinalList.PushBack(newEntry);
    }

    return true;
}

void App::LocalizationExtension::OnLoadVoiceOvers(void* aContext, uint64_t a2)
{
    LogInfo("[{}] Initializing voiceover index...", ExtensionName);

    auto mergedAny = false;
    auto successAll = true;

    if (!m_configs.empty())
    {
        auto depot = Red::ResourceDepot::Get();
        auto& tokens = Raw::Localization::VoiceOverTokens::Ref(aContext);
        auto map = tokens.Get(0)->entries[0]->resource->root.GetPtr<Red::locVoLanguageDataMap>();

        for (auto& entry : map->entries)
        {
            for (const auto& unit : m_configs)
            {
                auto paths = unit.vomaps.find(entry.languageCode);
                if (paths != unit.vomaps.end())
                {
                    // LogInfo("[{}] Processing \"{}\"...", ExtensionName, unit.name);

                    for (const auto& path : paths->second)
                    {
                        Red::RaRef<Red::JsonResource> ref(path.c_str());

                        if (depot->ResourceExists(ref.path))
                        {
                            LogInfo("[{}] Merging entries from \"{}\"...", ExtensionName, path);
                            entry.voMapChunks.PushBack(ref);
                        }
                        else
                        {
                            LogError("[{}] Resource \"{}\" not found.", ExtensionName, path);
                            successAll = false;
                        }
                    }

                    mergedAny = true;
                }
            }
        }
    }

    if (!mergedAny)
        LogInfo("[{}] No voiceover maps to merge.", ExtensionName);
    else if (successAll)
        LogInfo("[{}] All voiceover maps merged.", ExtensionName);
    else
        LogWarning("[{}] Voiceover maps merged with issues.", ExtensionName);
}

void App::LocalizationExtension::OnLoadLipsyncs(void* aContext, uint8_t a2)
{
    auto mainToken = Raw::Localization::LipMapToken(aContext)->instance;

    if (s_currentLipMap != mainToken)
    {
        s_currentLipMap = mainToken;

        const auto language = Language::ResolveFromLipsyncResource(mainToken->path);

        LogInfo("[{}] Initializing lipsync maps for \"{}\" language...", ExtensionName, language.ToString());

        auto mergedAny = false;
        bool successAll = true;

        auto loader = Red::ResourceLoader::Get();
        Red::DynArray<Red::SharedPtr<Red::ResourceToken<Red::animLipsyncMapping>>> tokens;

        for (const auto& unit : m_configs)
        {
            auto paths = unit.lipmaps.find(language);
            if (paths != unit.lipmaps.end())
            {
                // LogInfo("[{}] Processing \"{}\"...", ExtensionName, unit.name);

                for (const auto& path : paths->second)
                {
                    auto token = loader->LoadAsync<Red::animLipsyncMapping>(path.c_str());

                    if (!token->IsFailed())
                    {
                        s_paths[token->path] = path;
                        tokens.PushBack(std::move(token));
                    }
                    else
                    {
                        LogError("[{}] Resource \"{}\" failed to load.", ExtensionName, path);
                        successAll = false;
                    }
                }

                mergedAny = true;
            }
        }

        if (tokens.size)
        {
            Red::WaitForResources(tokens, std::chrono::milliseconds(5000));

            if (mainToken->IsFinished())
            {
                if (mainToken->IsFailed())
                {
                    LogWarning("[{}] Failed to initialize lipsync maps for \"{}\" language...",
                               ExtensionName, language.ToString());
                    return;
                }

                for (const auto& token : tokens)
                {
                    if (token->IsFailed())
                    {
                        LogError("[{}] Resource \"{}\" failed to load.", ExtensionName, s_paths[token->path]);
                        successAll = false;
                        continue;
                    }

                    successAll &= MergeLipsyncResource(token->resource, mainToken->resource);
                }
            }
            else
            {
                mainToken->OnLoaded([tokens = std::move(tokens)](Red::Handle<Red::animLipsyncMapping>& aResource) {
                    bool successAll = true;

                    for (const auto& token : tokens)
                    {
                        if (token->IsFailed())
                        {
                            LogError("[{}] Resource \"{}\" failed to load.", ExtensionName, s_paths[token->path]);
                            successAll = false;
                            continue;
                        }

                        successAll &= MergeLipsyncResource(token->resource, aResource);
                    }

                    if (successAll)
                        LogInfo("[{}] All lipsync maps merged.", ExtensionName);
                    else
                        LogWarning("[{}] Lipsync maps merged with issues.", ExtensionName);
                });

                return;
            }
        }

        if (!mergedAny)
            LogInfo("[{}] No lipsync maps to merge.", ExtensionName);
        else if (successAll)
            LogInfo("[{}] All lipsync maps merged.", ExtensionName);
        else
            LogWarning("[{}] Lipsync maps merged with issues.", ExtensionName);
    }
}

bool App::LocalizationExtension::MergeLipsyncResource(const Red::Handle<Red::animLipsyncMapping>& aSource,
                                                   Red::Handle<Red::animLipsyncMapping>& aTarget)
{
    if (aSource->scenePaths.size != aSource->sceneEntries.size)
        return false;

    LogInfo("[{}] Merging entries from \"{}\"...", ExtensionName, s_paths[aSource->path]);

    for (uint32_t i = 0; i < aSource->scenePaths.size; ++i)
    {
        auto& path = aSource->scenePaths[i];
        auto& entry = aSource->sceneEntries[i];

        LogInfo("[{}] Merging entry #{} with hash {}...", ExtensionName, i + 1, path);

        auto pathIt = std::lower_bound(aTarget->scenePaths.Begin(), aTarget->scenePaths.End(), path);
        auto targetIt = aTarget->sceneEntries.Begin() + (pathIt - aTarget->scenePaths.Begin());

        if (pathIt == aTarget->scenePaths.End() || *pathIt != path)
        {
            aTarget->scenePaths.Emplace(pathIt, path);
            aTarget->sceneEntries.Emplace(targetIt, entry);
        }
        else
        {
            for (auto& tag : entry.actorVoiceTags)
            {
                targetIt->actorVoiceTags.PushBack(tag);
            }

            for (auto& set : entry.animSets)
            {
                targetIt->animSets.PushBack(set);
            }
        }
    }

    return true;
}
