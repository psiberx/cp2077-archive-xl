#include "Extension.hpp"
#include "App/Extensions/ResourceLink/Extension.hpp"
#include "App/Extensions/ResourceMeta/Extension.hpp"
#include "Red/AppearanceResource.hpp"
#include "Red/Buffer.hpp"
#include "Red/TweakDB.hpp"

namespace
{
constexpr auto ExtensionName = "CharacterCustomization";

constexpr auto HairColorSlotName = Red::CName("hair_color");

Red::ClassLocator<Red::game::ui::AppearanceInfo> s_AppInfoType;
Red::ClassLocator<Red::game::ui::MorphInfo> s_MorphInfoType;
Red::ClassLocator<Red::game::ui::SwitcherInfo> s_SwitcherInfoType;
}

std::string_view App::CustomizationExtension::GetName()
{
    return ExtensionName;
}

bool App::CustomizationExtension::Load()
{
    HookBefore<Raw::CharacterCustomizationSystem::Initialize>(&CustomizationExtension::OnActivateSystem).OrThrow();
    HookBefore<Raw::CharacterCustomizationSystem::Uninitialize>(&CustomizationExtension::OnDeactivateSystem).OrThrow();
    HookBefore<Raw::CharacterCustomizationSystem::GetResource>(&CustomizationExtension::OnPrepareResource).OrThrow();
    HookBefore<Raw::CharacterCustomizationSystem::InitializeAppOption>(&CustomizationExtension::OnInitAppOption).OrThrow();
    HookBefore<Raw::CharacterCustomizationSystem::InitializeMorphOption>(&CustomizationExtension::OnInitMorphOption).OrThrow();
    HookAfter<Raw::CharacterCustomizationSystem::InitializeSwitcherOption>(&CustomizationExtension::OnInitSwitcherOption).OrThrow();
    HookAfter<Raw::CharacterCustomizationState::GetHeadAppearances1>(&CustomizationExtension::OnGetAppearances);
    HookAfter<Raw::CharacterCustomizationState::GetHeadAppearances2>(&CustomizationExtension::OnGetAppearances);
    HookAfter<Raw::CharacterCustomizationState::GetBodyAppearances1>(&CustomizationExtension::OnGetAppearances);
    HookAfter<Raw::CharacterCustomizationState::GetBodyAppearances2>(&CustomizationExtension::OnGetAppearances);
    HookAfter<Raw::CharacterCustomizationState::GetArmsAppearances1>(&CustomizationExtension::OnGetAppearances);
    HookAfter<Raw::CharacterCustomizationState::GetArmsAppearances2>(&CustomizationExtension::OnGetAppearances);
    HookBefore<Raw::RuntimeSystemEntityAppearanceChanger::ChangeAppearance>(&CustomizationExtension::OnChangeAppearance).OrThrow();
    HookBefore<Raw::RuntimeSystemEntityAppearanceChanger::ChangeAppearances>(&CustomizationExtension::OnChangeAppearances).OrThrow();

    return true;
}

void App::CustomizationExtension::OnDepotReady()
{
    PrefetchCustomResources();
}

bool App::CustomizationExtension::Unload()
{
    Unhook<Raw::CharacterCustomizationSystem::Initialize>();
    Unhook<Raw::CharacterCustomizationSystem::Uninitialize>();
    Unhook<Raw::CharacterCustomizationSystem::GetResource>();
    Unhook<Raw::CharacterCustomizationSystem::InitializeAppOption>();
    Unhook<Raw::CharacterCustomizationSystem::InitializeMorphOption>();
    Unhook<Raw::CharacterCustomizationSystem::InitializeSwitcherOption>();
    Unhook<Raw::CharacterCustomizationState::GetHeadAppearances1>();
    Unhook<Raw::CharacterCustomizationState::GetHeadAppearances2>();
    Unhook<Raw::CharacterCustomizationState::GetBodyAppearances1>();
    Unhook<Raw::CharacterCustomizationState::GetBodyAppearances2>();
    Unhook<Raw::CharacterCustomizationState::GetArmsAppearances1>();
    Unhook<Raw::CharacterCustomizationState::GetArmsAppearances2>();
    Unhook<Raw::RuntimeSystemEntityAppearanceChanger::ChangeAppearance>();
    Unhook<Raw::RuntimeSystemEntityAppearanceChanger::ChangeAppearances>();

    return true;
}

void App::CustomizationExtension::Reload()
{
    if (!m_customMaleResources.empty() || !m_customFemaleResources.empty())
    {
        RemoveCustomEntries();
        ResetCustomResources();
    }

    PrefetchCustomResources();
}

void App::CustomizationExtension::OnActivateSystem(App::CustomizationSystem* aSystem, App::CustomizationPuppet& aPuppet,
                                                   bool aIsMale, uintptr_t a4)
{
    m_customizationActive = true;
}

void App::CustomizationExtension::OnDeactivateSystem(App::CustomizationSystem* aSystem)
{
    m_customizationActive = false;
}

void App::CustomizationExtension::OnPrepareResource(App::CustomizationSystem* aSystem,
    Red::SharedPtr<Red::ResourceToken<Red::gameuiCharacterCustomizationInfoResource>>& aOut, bool aIsMale)
{
    MergeCustomEntries(aSystem);
}

void App::CustomizationExtension::OnInitAppOption(App::CustomizationSystem* aSystem,
                                                  App::CustomizationPart aPartType,
                                                  App::CustomizationStateOption& aOption,
                                                  Red::SortedUniqueArray<Red::CName>& aStateOptions,
                                                  Red::Map<Red::CName, App::CustomizationStateOption>& aUiSlots)
{
    bool found = aStateOptions.Find(aOption->info->name) != aStateOptions.end();

    if (!found && !aOption->info->hidden && aOption->info->enabled)
    {
        if (IsCustomEntryName(aOption->info->name))
        {
            aStateOptions.Emplace(aOption->info->name);
        }
    }
}

void App::CustomizationExtension::OnInitMorphOption(App::CustomizationSystem* aSystem,
                                                    App::CustomizationStateOption& aOption,
                                                    Red::SortedUniqueArray<Red::CName>& aStateOptions,
                                                    Red::Map<Red::CName, App::CustomizationStateOption>& aUiSlots)
{
    bool found = aStateOptions.Find(aOption->info->name) != aStateOptions.end();

    if (!found && !aOption->info->hidden && aOption->info->enabled)
    {
        if (IsCustomEntryName(aOption->info->name))
        {
            aStateOptions.Emplace(aOption->info->name);
        }
    }
}

void App::CustomizationExtension::OnInitSwitcherOption(App::CustomizationSystem* aSystem,
                                                       App::CustomizationPart aPartType,
                                                       App::CustomizationStateOption& aOption,
                                                       int32_t aCurrentIndex, uint64_t a5,
                                                       Red::Map<Red::CName, App::CustomizationStateOption>& aUiSlots)
{
    if (!aOption->isActive && !aOption->info->hidden && aOption->info->enabled)
    {
        if (IsCustomEntryName(aOption->info->name))
        {
            aOption->isActive = true;
        }
    }
}

void App::CustomizationExtension::OnGetAppearances(Red::gameuiICharacterCustomizationState* aState,
                                                   Red::CName aGroupName, bool aIsFPP,
                                                   Red::DynArray<Red::AppearanceDescriptor>& aAppearances)
{
    for (auto& app : aAppearances)
    {
        ApplyAppOverride(app);
    }
}

void App::CustomizationExtension::OnChangeAppearance(App::AppearanceChangerSystem& aSystem,
                                                     Red::AppearanceChangeRequest* aRequest, uintptr_t a3)
{
    if (/*m_customizationActive &&*/ !m_customAppOverrides.empty())
    {
        if (aRequest->oldAppearance)
        {
            ApplyAppOverride(aRequest->oldAppearance);
        }
        if (aRequest->newAppearance)
        {
            ApplyAppOverride(aRequest->newAppearance);
        }
    }
}

void App::CustomizationExtension::OnChangeAppearances(App::AppearanceChangerSystem& aSystem,
                                                      App::CustomizationPuppet& aPuppet,
                                                      Red::Range<Red::AppearanceDescriptor>& aOldApp,
                                                      Red::Range<Red::AppearanceDescriptor>& aNewApp,
                                                      uintptr_t a5, uint8_t a6)
{
    if (/*m_customizationActive &&*/ !m_customAppOverrides.empty())
    {
        if (aOldApp)
        {
            for (auto& app : aOldApp)
            {
                ApplyAppOverride(app);
            }
        }

        if (aNewApp)
        {
            for (auto& app : aNewApp)
            {
                ApplyAppOverride(app);
            }
        }
    }
}

void App::CustomizationExtension::RegisterCustomEntryName(Red::CName aName)
{
    m_customEntryNames.insert(aName);
}

void App::CustomizationExtension::RegisterCustomEntryName(Red::CName aName, const Red::CString& aSubName)
{
    RegisterCustomEntryName(Red::FNV1a64(reinterpret_cast<const uint8_t*>(aSubName.c_str()), aSubName.Length(), aName));
}

bool App::CustomizationExtension::IsCustomEntryName(Red::CName aName)
{
    return m_customEntryNames.contains(aName);
}

bool App::CustomizationExtension::IsCustomEntryName(Red::CName aName, const Red::CString& aSubName)
{
    return IsCustomEntryName(Red::FNV1a64(reinterpret_cast<const uint8_t*>(aSubName.c_str()), aSubName.Length(), aName));
}

void App::CustomizationExtension::RegisterAppOverride(Red::ResourcePath aResource, Red::ResourcePath aOverride,
                                                      Red::CName aAppearance)
{
    m_customAppOverrides.insert({Red::AppearanceDescriptor{aResource, aAppearance},
                                 Red::AppearanceDescriptor{aOverride, aAppearance}});
}

void App::CustomizationExtension::ApplyAppOverride(Red::AppearanceDescriptor& aAppearance)
{
    auto it = m_customAppOverrides.find(aAppearance);
    if (it != m_customAppOverrides.end())
    {
        aAppearance = it.value();
    }
}

void App::CustomizationExtension::PrefetchCustomResources()
{
    for (const auto& unit : m_configs)
    {
        PrefetchCustomResources(m_customMaleResources, unit.maleOptions);
        PrefetchCustomResources(m_customFemaleResources, unit.femaleOptions);
    }
}

void App::CustomizationExtension::PrefetchCustomResources(Core::Vector<CustomizationResourceToken>& aResources,
                                                          const Core::Vector<std::string>& aPaths)
{
    auto loader = Red::ResourceLoader::Get();

    for (const auto& path : aPaths)
    {
        auto token = loader->LoadAsync<CustomizationResource>(path.c_str());
        aResources.emplace_back(token);
    }
}

void App::CustomizationExtension::MergeCustomEntries(App::CustomizationSystem* aSystem)
{
    std::unique_lock _(m_customEntriesMergeLock);

    if (m_customEntriesMerged)
        return;

    m_baseMaleResource = Raw::CharacterCustomizationSystem::MaleResource::Ref(aSystem);
    m_baseFemaleResource = Raw::CharacterCustomizationSystem::FemaleResource::Ref(aSystem);

    MergeCustomEntries(m_baseMaleResource, m_customMaleResources);
    MergeCustomEntries(m_baseFemaleResource, m_customFemaleResources);

    m_customEntriesMerged = true;
}

void App::CustomizationExtension::MergeCustomEntries(CustomizationResourceToken& aTargetResource,
                                                     Core::Vector<CustomizationResourceToken>& aSourceResources)
{
    if (!aTargetResource.instance->finished)
    {
        Red::WaitForResource(aTargetResource, std::chrono::milliseconds(250));
        if (!aTargetResource.instance->finished)
        {
            LogError("[{}] Game customization resource is not ready.", ExtensionName);
            return;
        }
    }

    auto& gameData = aTargetResource->Get();

    FixCustomizationOptions(aTargetResource->path, gameData->armsCustomizationOptions);
    FixCustomizationOptions(aTargetResource->path, gameData->bodyCustomizationOptions);
    FixCustomizationOptions(aTargetResource->path, gameData->headCustomizationOptions);

    for (const auto& customResource : aSourceResources)
    {
        if (!customResource->finished)
        {
            LogError("[{}] Mod customization resource is not ready.", ExtensionName);
            continue;
        }

        auto& customData = customResource->Get();

        MergeCustomGroups(gameData->armsGroups, customData->armsGroups);
        MergeCustomGroups(gameData->bodyGroups, customData->bodyGroups);
        MergeCustomGroups(gameData->headGroups, customData->headGroups);

        MergeCustomOptions(gameData->armsCustomizationOptions, customData->armsCustomizationOptions, false);
        MergeCustomOptions(gameData->bodyCustomizationOptions, customData->bodyCustomizationOptions, false);
        MergeCustomOptions(gameData->headCustomizationOptions, customData->headCustomizationOptions, false);
    }

    for (const auto& customResource : aSourceResources)
    {
        if (!customResource->finished)
            continue;

        auto& customData = customResource->Get();

        MergeCustomOptions(gameData->armsCustomizationOptions, customData->armsCustomizationOptions, true);
        MergeCustomOptions(gameData->bodyCustomizationOptions, customData->bodyCustomizationOptions, true);
        MergeCustomOptions(gameData->headCustomizationOptions, customData->headCustomizationOptions, true);
    }

    RegenerateIndexes(gameData->armsCustomizationOptions);
    RegenerateIndexes(gameData->bodyCustomizationOptions);
    RegenerateIndexes(gameData->headCustomizationOptions);

    if (!m_hairColorNames.empty())
    {
        Red::AppendToFlat("ItemFactory.HairColors.hairColors", m_hairColorNames);
    }
}

void App::CustomizationExtension::MergeCustomGroups(Red::DynArray<CustomizationGroup>& aTargetGroups,
                                                    Red::DynArray<CustomizationGroup>& aSourceGroups)
{
    for (const auto& sourceGroup : aSourceGroups)
    {
        for (auto& targetGroup : aTargetGroups)
        {
            if (targetGroup.name == sourceGroup.name)
            {
                for (const auto& sourceGroupEntry : sourceGroup.options)
                {
                    targetGroup.options.EmplaceBack(sourceGroupEntry);

                    RegisterCustomEntryName(sourceGroupEntry);
                }
            }
        }
    }
}

void App::CustomizationExtension::MergeCustomOptions(Red::DynArray<CustomizationOption>& aTargetOptions,
                                                     Red::DynArray<CustomizationOption>& aSourceOptions,
                                                     bool aSlotsAndLinks)
{
    for (auto sourceOption : aSourceOptions)
    {
        if (aSlotsAndLinks)
        {
            if (sourceOption->name || (!sourceOption->uiSlot && !sourceOption->link))
                continue;
        }
        else
        {
            if (!sourceOption->name)
                continue;
        }

        std::string_view sourceSlotStr = sourceOption->uiSlot.ToString();
        const bool isWildcardSlot = aSlotsAndLinks && sourceSlotStr.ends_with('*');
        if (isWildcardSlot)
        {
            sourceSlotStr.remove_suffix(1);
        }

        std::string_view sourceLinkStr = sourceOption->link.ToString();
        const bool isWildcardLink = aSlotsAndLinks && sourceSlotStr.ends_with('*');
        if (isWildcardLink)
        {
            sourceLinkStr.remove_suffix(1);
        }

        bool isExistingOption = false;

        for (auto& targetOption : aTargetOptions)
        {
            if (aSlotsAndLinks)
            {
                if (sourceOption->uiSlot)
                {
                    if (isWildcardSlot)
                    {
                        if (!std::string_view(targetOption->uiSlot.ToString()).starts_with(sourceSlotStr))
                            continue;
                    }
                    else
                    {
                        if (targetOption->uiSlot != sourceOption->uiSlot)
                            continue;
                    }
                }

                if (sourceOption->link)
                {
                    if (isWildcardLink)
                    {
                        if (!std::string_view(targetOption->link.ToString()).starts_with(sourceLinkStr))
                            continue;
                    }
                    else
                    {
                        if (targetOption->link != sourceOption->link)
                            continue;
                    }
                }
            }
            else
            {
                if (targetOption->name != sourceOption->name)
                    continue;

                if (sourceOption->link)
                {
                    auto isLinkFound = false;

                    for (const auto& linkedOption : aSourceOptions)
                    {
                        if (linkedOption->name == sourceOption->link)
                        {
                            sourceOption = linkedOption;
                            isLinkFound = true;
                            break;
                        }
                    }

                    if (!isLinkFound)
                        continue;
                }
            }

            isExistingOption = true;

            if (targetOption->GetNativeType() != sourceOption->GetNativeType())
            {
                LogWarning("[{}] Option \"{}\" can't be merged: expected {}, got {}.",
                           ExtensionName, targetOption->name.ToString(),
                           targetOption->GetNativeType()->GetName().ToString(),
                           sourceOption->GetNativeType()->GetName().ToString());
                continue;
            }

            if (targetOption->GetNativeType()->IsA(s_AppInfoType))
            {
                auto& targetAppOption = reinterpret_cast<CustomizationAppearance&>(targetOption);
                auto& sourceAppOption = reinterpret_cast<const CustomizationAppearance&>(sourceOption);

                if (isWildcardSlot || isWildcardLink)
                {
                    if (!targetAppOption->resource.path || !targetAppOption->definitions.size)
                        continue;
                }

                for (const auto& sourceChoice : sourceAppOption->definitions)
                {
                    bool isExistingChoice = false;

                    for (auto& targetChoice : targetAppOption->definitions)
                    {
                        if (targetChoice.name == sourceChoice.name)
                        {
                            targetChoice = sourceChoice;
                            isExistingChoice = true;
                            break;
                        }
                    }

                    if (!isExistingChoice)
                    {
                        targetAppOption->definitions.PushBack(sourceChoice);

                        if (sourceAppOption->resource.path)
                        {
                            if (targetAppOption->resource.path != sourceAppOption->resource.path)
                            {
                                RegisterAppOverride(targetAppOption->resource.path, sourceAppOption->resource.path,
                                                    sourceChoice.name);
                            }
                        }

                        RegisterCustomEntryName(sourceChoice.name);
                    }

                    if (targetOption->uiSlot == HairColorSlotName)
                    {
                        for (const auto& tag : sourceChoice.tags.tags)
                        {
                            m_hairColorNames.insert(tag);
                        }
                    }
                }
            }
            else if (targetOption->GetNativeType()->IsA(s_MorphInfoType))
            {
                auto& targetMorphOption = reinterpret_cast<CustomizationMorph&>(targetOption);
                auto& sourceMorphOption = reinterpret_cast<const CustomizationMorph&>(sourceOption);

                for (const auto& sourceChoice : sourceMorphOption->morphNames)
                {
                    bool isExistingChoice = false;

                    for (auto& targetChoice : targetMorphOption->morphNames)
                    {
                        if (targetChoice.localizedName == sourceChoice.localizedName)
                        {
                            targetChoice = sourceChoice;
                            isExistingChoice = true;
                            break;
                        }
                    }

                    if (!isExistingChoice)
                    {
                        targetMorphOption->morphNames.PushBack(sourceChoice);

                        RegisterCustomEntryName(sourceOption->name, sourceChoice.localizedName);
                    }
                }
            }
            else if (targetOption->GetNativeType()->IsA(s_SwitcherInfoType))
            {
                auto& targetSwitcherOption = reinterpret_cast<CustomizationSwitcher&>(targetOption);
                auto& sourceSwitcherOption = reinterpret_cast<const CustomizationSwitcher&>(sourceOption);

                for (const auto& sourceChoice : sourceSwitcherOption->options)
                {
                    bool isExistingChoice = false;

                    for (auto& targetChoice : targetSwitcherOption->options)
                    {
                        if (targetChoice.localizedName == sourceChoice.localizedName)
                        {
                            targetChoice = sourceChoice;
                            isExistingChoice = true;
                            break;
                        }
                    }

                    if (!isExistingChoice)
                    {
                        targetSwitcherOption->options.PushBack(sourceChoice);

                        RegisterCustomEntryName(sourceOption->name, sourceChoice.localizedName);
                    }
                }
            }
        }

        if (!isExistingOption && !aSlotsAndLinks && sourceOption->name)
        {
            aTargetOptions.EmplaceBack(sourceOption);

            RegisterCustomEntryName(sourceOption->name);
        }
    }
}

void App::CustomizationExtension::RegenerateIndexes(Red::DynArray<CustomizationOption>& aTargetOptions)
{
    for (auto& targetOption : aTargetOptions)
    {
        if (targetOption->GetNativeType()->IsA(s_AppInfoType))
        {
            auto& targetAppOption = reinterpret_cast<CustomizationAppearance&>(targetOption);

            for (int32_t index = 0; index < targetAppOption->definitions.size; ++index)
            {
                targetAppOption->definitions[index].index = index;
            }
        }
        else if (targetOption->GetNativeType()->IsA(s_MorphInfoType))
        {
            auto& targetMorphOption = reinterpret_cast<CustomizationMorph&>(targetOption);

            for (int32_t index = 0; index < targetMorphOption->morphNames.size; ++index)
            {
                targetMorphOption->morphNames[index].index = index;
            }
        }
        else if (targetOption->GetNativeType()->IsA(s_SwitcherInfoType))
        {
            auto& targetSwitcherOption = reinterpret_cast<CustomizationSwitcher&>(targetOption);

            for (int32_t index = 0; index < targetSwitcherOption->options.size; ++index)
            {
                targetSwitcherOption->options[index].index = index;
            }
        }
    }
}

void App::CustomizationExtension::RemoveCustomEntries()
{
    std::unique_lock _(m_customEntriesMergeLock);

    if (!m_customEntriesMerged)
        return;

    RemoveCustomEntries(m_baseMaleResource);
    RemoveCustomEntries(m_baseFemaleResource);

    m_baseMaleResource = nullptr;
    m_baseFemaleResource = nullptr;

    m_customAppOverrides.clear();
    m_customEntryNames.clear();
    m_customEntriesMerged = false;
}

void App::CustomizationExtension::RemoveCustomEntries(App::CustomizationResourceToken& aTargetResource)
{
    if (!aTargetResource.instance->finished)
        return;

    auto& gameData = aTargetResource->Get();

    RemoveCustomGroups(gameData->armsGroups);
    RemoveCustomGroups(gameData->bodyGroups);
    RemoveCustomGroups(gameData->headGroups);

    RemoveCustomOptions(gameData->armsCustomizationOptions);
    RemoveCustomOptions(gameData->bodyCustomizationOptions);
    RemoveCustomOptions(gameData->headCustomizationOptions);
}

void App::CustomizationExtension::RemoveCustomGroups(Red::DynArray<CustomizationGroup>& aTargetGroups)
{
    for (auto& targetGroup : aTargetGroups)
    {
        for (auto i = static_cast<int32_t>(targetGroup.options.size) - 1; i >= 0; --i)
        {
            if (IsCustomEntryName(targetGroup.options[i]))
            {
                targetGroup.options.RemoveAt(i);
            }
        }
    }
}

void App::CustomizationExtension::RemoveCustomOptions(Red::DynArray<CustomizationOption>& aTargetOptions)
{
    for (auto i = static_cast<int32_t>(aTargetOptions.size) - 1; i >= 0; --i)
    {
        auto& targetOption = aTargetOptions[i];

        if (IsCustomEntryName(targetOption->name))
        {
            aTargetOptions.RemoveAt(i);
            continue;
        }

        if (targetOption->GetNativeType()->IsA(s_AppInfoType))
        {
            auto& targetAppOption = reinterpret_cast<CustomizationAppearance&>(targetOption);

            for (auto j = static_cast<int32_t>(targetAppOption->definitions.size) - 1; j >= 0; --j)
            {
                if (IsCustomEntryName(targetAppOption->definitions[j].name))
                {
                    targetAppOption->definitions.RemoveAt(j);
                }
            }
        }
        else if (targetOption->GetNativeType()->IsA(s_MorphInfoType))
        {
            auto& targetMorphOption = reinterpret_cast<CustomizationMorph&>(targetOption);

            for (auto j = static_cast<int32_t>(targetMorphOption->morphNames.size) - 1; j >= 0; --j)
            {
                if (IsCustomEntryName(targetMorphOption->name, targetMorphOption->morphNames[j].localizedName))
                {
                    targetMorphOption->morphNames.RemoveAt(j);
                }
            }
        }
        else if (targetOption->GetNativeType()->IsA(s_SwitcherInfoType))
        {
            auto& targetSwitcherOption = reinterpret_cast<CustomizationSwitcher&>(targetOption);

            for (auto j = static_cast<int32_t>(targetSwitcherOption->options.size) - 1; j >= 0; --j)
            {
                if (IsCustomEntryName(targetSwitcherOption->name, targetSwitcherOption->options[j].localizedName))
                {
                    targetSwitcherOption->options.RemoveAt(j);
                }
            }
        }
    }
}

void App::CustomizationExtension::ResetCustomResources()
{
    m_customMaleResources.clear();
    m_customFemaleResources.clear();
}

void App::CustomizationExtension::FixCustomizationOptions(Red::ResourcePath aTargetPath,
                                                          Red::DynArray<CustomizationOption>& aTargetOptions)
{
    const auto& fix = ResourceMetaExtension::GetFix(aTargetPath);

    if (fix.DefinesPathMappings())
    {
        for (auto& targetOption : aTargetOptions)
        {
            if (auto& appInfoOption = Red::Cast<Red::game::ui::AppearanceInfo>(targetOption))
            {
                auto originalPath = appInfoOption->resource.path;
                auto mappedPath = fix.GetMappedPath(originalPath);

                if (mappedPath != originalPath)
                {
                    appInfoOption->resource.path = mappedPath;

                    const auto aliasPaths = ResourceLinkExtension::GetAliases(mappedPath);

                    for (const auto& definition : appInfoOption->definitions)
                    {
                        RegisterAppOverride(originalPath, mappedPath, definition.name);

                        for (const auto& aliasPath : aliasPaths)
                        {
                            RegisterAppOverride(aliasPath, mappedPath, definition.name);
                        }
                    }
                }
            }
        }
    }
}

void App::CustomizationExtension::FixCustomizationAppearance(Red::AppearanceResource* aResource,
                                                             Red::Handle<Red::AppearanceDefinition>* aDefinition,
                                                             Red::CName aAppearanceName)
{
    if (aResource->appearances.size == 0)
        return;

    if (!ResourceMetaExtension::InScope(ResourceMetaExtension::CustomizationApp, aResource->path))
        return;

    auto meshAppearanceStr = std::string_view{aAppearanceName.ToString()};

    if (meshAppearanceStr.size() < 3)
        return;

    std::unique_lock _(*Raw::AppearanceResource::Mutex(aResource));
    Red::Handle<Red::AppearanceDefinition> sourceDefinition;

    if (meshAppearanceStr[2] == '_' && std::isdigit(meshAppearanceStr[0]))
    {
        meshAppearanceStr.remove_prefix(3);

        auto sourceIndex = aResource->appearances.size > 1 ? 1 : 0;
        sourceDefinition = aResource->appearances[sourceIndex];
    }
    else
    {
        auto delimiterPos = meshAppearanceStr.rfind("__");
        if (delimiterPos != std::string_view::npos)
        {
            if (aResource->appearances.size == 1)
            {
                sourceDefinition = aResource->appearances[0];
            }
            else
            {
                for (auto sourceIndex = 0; sourceIndex < aResource->appearances.size; ++sourceIndex)
                {
                    auto sourceNameStr = std::string_view{aResource->appearances[sourceIndex]->name.ToString()};
                    if (sourceNameStr.compare(0, delimiterPos, meshAppearanceStr, 0, delimiterPos) == 0)
                    {
                        sourceDefinition = aResource->appearances[sourceIndex];
                        break;
                    }
                }
            }

            meshAppearanceStr.remove_prefix(delimiterPos + 2);

            if (meshAppearanceStr.size() < 3)
                return;

            if (meshAppearanceStr[2] == '_' && std::isdigit(meshAppearanceStr[0]))
            {
                meshAppearanceStr.remove_prefix(3);
            }
        }

        if (!sourceDefinition)
        {
            auto sourceIndex = aResource->appearances.size > 1 ? 1 : 0;
            sourceDefinition = aResource->appearances[sourceIndex];
        }
    }

    if (sourceDefinition->partsOverrides.size > 0)
    {
        if (IsFixedCustomizationAppearance(sourceDefinition))
        {
            *aDefinition = sourceDefinition;
            return;
        }

        auto newDefinition = Red::MakeHandle<Red::AppearanceDefinition>();
        for (const auto prop : Red::GetClass<Red::AppearanceDefinition>()->props)
        {
            prop->SetValue(newDefinition.instance, prop->GetValuePtr<void>(sourceDefinition.instance));
        }

        Red::BufferReader::Clone(newDefinition->compiledData.unk30, sourceDefinition->compiledData.unk30);

        newDefinition->name = aAppearanceName;

        auto meshAppearance = Red::CNamePool::Add(meshAppearanceStr.data());
        for (auto& componentOverride : newDefinition->partsOverrides[0].componentsOverrides)
        {
            componentOverride.meshAppearance = meshAppearance;
        }

        aResource->appearances.PushBack(newDefinition);

        *aDefinition = std::move(newDefinition);
    }
    else
    {
        sourceDefinition->name = aAppearanceName;

        auto meshAppearance = Red::CNamePool::Add(meshAppearanceStr.data());
        sourceDefinition->partsOverrides.EmplaceBack();
        sourceDefinition->partsOverrides[0].componentsOverrides.EmplaceBack();
        sourceDefinition->partsOverrides[0].componentsOverrides[0].meshAppearance = meshAppearance;

        *aDefinition = std::move(sourceDefinition);
    }
}

void App::CustomizationExtension::FixCustomizationComponents(const Red::Handle<Red::AppearanceResource>& aResource,
                                                             const Red::Handle<Red::AppearanceDefinition>& aDefinition,
                                                             Red::DynArray<Red::Handle<Red::ISerializable>>& aComponents)
{
    if (!ResourceMetaExtension::InScope(ResourceMetaExtension::CustomizationApp, aResource->path))
        return;

    for (auto& override : aDefinition->partsOverrides[0].componentsOverrides)
    {
        for (const auto& component : aComponents)
        {
            if (auto meshComponent = Red::Cast<Red::entSkinnedMeshComponent>(component))
            {
                if (!override.componentName || meshComponent->name == override.componentName)
                {
                    if (meshComponent->meshAppearance && meshComponent->meshAppearance != "default")
                    {
                        meshComponent->meshAppearance = override.meshAppearance;
                    }
                }
            }
            else if (auto morphComponent = Red::Cast<Red::entMorphTargetSkinnedMeshComponent>(component))
            {
                if (!override.componentName || morphComponent->name == override.componentName)
                {
                    if (morphComponent->meshAppearance && morphComponent->meshAppearance != "default")
                    {
                        morphComponent->meshAppearance = override.meshAppearance;
                    }
                }
            }
        }
    }
}

bool App::CustomizationExtension::IsFixedCustomizationAppearance(const Red::Handle<Red::AppearanceDefinition>& aDefinition)
{
    return aDefinition->partsOverrides.size == 1 &&
           aDefinition->partsOverrides[0].componentsOverrides.size == 1 &&
           !aDefinition->partsOverrides[0].componentsOverrides[0].componentName;
}
