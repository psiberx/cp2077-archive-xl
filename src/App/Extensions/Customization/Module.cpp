#include "Module.hpp"
#include "Red/Rtti/Locator.hpp"

namespace
{
constexpr auto ModuleName = "Customization";

constexpr auto MaleResource = Red::ResourcePath(R"(base\gameplay\gui\fullscreen\main_menu\male_cco.inkcharcustomization)");
constexpr auto FemaleResource = Red::ResourcePath(R"(base\gameplay\gui\fullscreen\main_menu\female_cco.inkcharcustomization)");

Red::Rtti::ClassLocator<Red::game::ui::AppearanceInfo> s_AppInfoType;
Red::Rtti::ClassLocator<Red::game::ui::MorphInfo> s_MorphInfoType;
Red::Rtti::ClassLocator<Red::game::ui::SwitcherInfo> s_SwitcherInfoType;
}

std::string_view App::CustomizationModule::GetName()
{
    return ModuleName;
}

bool App::CustomizationModule::Load()
{
    if (!HookBefore<Raw::CharacterCustomizationSystem::Initialize>(&CustomizationModule::OnActivateSystem))
        throw std::runtime_error("Failed to hook [CharacterCustomizationSystem::Initialize].");

    if (!HookBefore<Raw::CharacterCustomizationSystem::Uninitialize>(&CustomizationModule::OnDeactivateSystem))
        throw std::runtime_error("Failed to hook [CharacterCustomizationSystem::Uninitialize].");

    if (!HookBefore<Raw::CharacterCustomizationSystem::EnsureState>(&CustomizationModule::OnEnsureState))
        throw std::runtime_error("Failed to hook [CharacterCustomizationSystem::EnsureState].");

    if (!HookBefore<Raw::CharacterCustomizationSystem::InitializeAppOption>(&CustomizationModule::OnInitAppOption))
        throw std::runtime_error("Failed to hook [CharacterCustomizationSystem::InitializeAppOption].");

    if (!HookBefore<Raw::CharacterCustomizationSystem::InitializeMorphOption>(&CustomizationModule::OnInitMorphOption))
        throw std::runtime_error("Failed to hook [CharacterCustomizationSystem::InitializeMorphOption].");

    if (!HookAfter<Raw::CharacterCustomizationSystem::InitializeSwitcherOption>(&CustomizationModule::OnInitSwitcherOption))
        throw std::runtime_error("Failed to hook [CharacterCustomizationSystem::InitializeSwitcherOption].");

    if (!HookBefore<Raw::RuntimeSystemEntityAppearanceChanger::ChangeAppearance>(&CustomizationModule::OnChangeAppearance))
        throw std::runtime_error("Failed to hook [RuntimeSystemEntityAppearanceChanger::ChangeAppearance].");

    return true;
}

void App::CustomizationModule::PostLoad()
{
    PrefetchCustomResources();
}

bool App::CustomizationModule::Unload()
{
    Unhook<Raw::CharacterCustomizationSystem::Initialize>();
    Unhook<Raw::CharacterCustomizationSystem::EnsureState>();
    Unhook<Raw::CharacterCustomizationSystem::InitializeAppOption>();
    Unhook<Raw::CharacterCustomizationSystem::InitializeMorphOption>();
    Unhook<Raw::CharacterCustomizationSystem::InitializeSwitcherOption>();
    Unhook<Raw::RuntimeSystemEntityAppearanceChanger::ChangeAppearance>();

    return true;
}

void App::CustomizationModule::Reload()
{
    RemoveCustomEntries();
    ResetCustomResources();
    PrefetchCustomResources();
}

void App::CustomizationModule::RegisterCustomEntryName(Red::CName aName)
{
    m_customEntryNames.insert(aName);
}

bool App::CustomizationModule::IsCustomEntryName(Red::CName aName)
{
    return m_customEntryNames.contains(aName);
}

void App::CustomizationModule::RegisterCustomEntryName(Red::CName aName, const Red::CString& aSubName)
{
    RegisterCustomEntryName(Red::FNV1a64(reinterpret_cast<const uint8_t*>(aSubName.c_str()), aSubName.Length(), aName));
}

bool App::CustomizationModule::IsCustomEntryName(Red::CName aName, const Red::CString& aSubName)
{
    return IsCustomEntryName(Red::FNV1a64(reinterpret_cast<const uint8_t*>(aSubName.c_str()), aSubName.Length(), aName));
}

void App::CustomizationModule::RegisterAppOverride(Red::ResourcePath aOriginal, Red::ResourcePath aOverride,
                                                   Red::CName aName)
{
    m_customAppOverrides.insert({Red::AppearanceDescriptor{aOriginal, aName},
                                 Red::AppearanceDescriptor{aOverride, aName}});
}

void App::CustomizationModule::ApplyAppOverride(Red::AppearanceDescriptor& aAppearance)
{
    auto it = m_customAppOverrides.find(aAppearance);
    if (it != m_customAppOverrides.end())
    {
        aAppearance = it.value();
    }
}

void App::CustomizationModule::PrefetchCustomResources()
{
    for (const auto& unit : m_units)
    {
        PrefetchCustomResources(m_customMaleResources, unit.maleOptions);
        PrefetchCustomResources(m_customFemaleResources, unit.femaleOptions);
    }
}

void App::CustomizationModule::PrefetchCustomResources(Core::Vector<CustomizationResourceToken>& aResources,
                                                       const Core::Vector<std::string>& aPaths)
{
    auto loader = Red::ResourceLoader::Get();

    for (const auto& path : aPaths)
    {
        auto token = loader->LoadAsync<CustomizationResource>(path.c_str());
        // token->OnLoaded([path](CustomizationResource& aResource) {
        //     LogInfo("OnLoaded {}", path);
        // });

        aResources.emplace_back(token);
    }
}

void App::CustomizationModule::MergeCustomEntries()
{
    std::unique_lock _(m_customEntriesMergeLock);

    if (m_customEntriesMerged)
        return;

    auto loader = Red::ResourceLoader::Get();

    auto maleResource = loader->LoadAsync<CustomizationResource>(MaleResource);
    auto femaleResource = loader->LoadAsync<CustomizationResource>(FemaleResource);

    MergeCustomEntries(maleResource, m_customMaleResources);
    MergeCustomEntries(femaleResource, m_customFemaleResources);

    m_customEntriesMerged = true;
}

void App::CustomizationModule::MergeCustomEntries(CustomizationResourceToken& aTargetResource,
                                                  Core::Vector<CustomizationResourceToken>& aSourceResources)
{
    if (!aTargetResource.instance->finished)
    {
        LogWarning("Game customization resource is not ready.");
        return;
    }

    auto& gameData = aTargetResource->Get();

    for (const auto& customResource : aSourceResources)
    {
        if (!customResource->finished)
        {
            LogWarning("Custom customization resource is not ready.");
            continue;
        }

        auto& customData = customResource->Get();

        MergeCustomGroups(gameData->armsGroups, customData->armsGroups);
        MergeCustomGroups(gameData->bodyGroups, customData->bodyGroups);
        MergeCustomGroups(gameData->headGroups, customData->headGroups);

        MergeCustomOptions(gameData->armsCustomizationOptions, customData->armsCustomizationOptions);
        MergeCustomOptions(gameData->bodyCustomizationOptions, customData->bodyCustomizationOptions);
        MergeCustomOptions(gameData->headCustomizationOptions, customData->headCustomizationOptions);
    }
}

void App::CustomizationModule::MergeCustomGroups(Red::DynArray<CustomizationGroup>& aTargetGroups,
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

void App::CustomizationModule::MergeCustomOptions(Red::DynArray<CustomizationOption>& aTargetOptions,
                                                  Red::DynArray<CustomizationOption>& aSourceOptions)
{
    for (auto& sourceOption : aSourceOptions)
    {
        bool isExistingOption = false;

        for (auto& targetOption : aTargetOptions)
        {
            if (targetOption->name != sourceOption->name)
                continue;

            isExistingOption = true;

            if (targetOption->GetNativeType() != sourceOption->GetNativeType())
                // TODO: Warning
                break;

            if (targetOption->GetNativeType()->IsA(s_AppInfoType))
            {
                auto& targetAppOption = reinterpret_cast<CustomizationAppearance&>(targetOption);
                auto& sourceAppOption = reinterpret_cast<CustomizationAppearance&>(sourceOption);

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
                        targetAppOption->definitions.EmplaceBack(sourceChoice);

                        if (targetAppOption->resource.path != sourceAppOption->resource.path)
                        {
                            RegisterAppOverride(targetAppOption->resource.path, sourceAppOption->resource.path,
                                                sourceChoice.name);
                        }

                        RegisterCustomEntryName(sourceChoice.name);
                    }
                }
            }
            else if (targetOption->GetNativeType()->IsA(s_MorphInfoType))
            {
                auto& targetMorphOption = reinterpret_cast<CustomizationMorph&>(targetOption);
                auto& sourceMorphOption = reinterpret_cast<CustomizationMorph&>(sourceOption);

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
                        targetMorphOption->morphNames.EmplaceBack(sourceChoice);

                        RegisterCustomEntryName(sourceOption->name, sourceChoice.localizedName);
                    }
                }
            }
            else if (targetOption->GetNativeType()->IsA(s_SwitcherInfoType))
            {
                auto& targetSwitcherOption = reinterpret_cast<CustomizationSwitcher&>(targetOption);
                auto& sourceSwitcherOption = reinterpret_cast<CustomizationSwitcher&>(sourceOption);

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
                        targetSwitcherOption->options.EmplaceBack(sourceChoice);

                        RegisterCustomEntryName(sourceOption->name, sourceChoice.localizedName);
                    }
                }
            }

            break;
        }

        if (!isExistingOption)
        {
            aTargetOptions.EmplaceBack(sourceOption);

            RegisterCustomEntryName(sourceOption->name);
        }
    }
}

void App::CustomizationModule::RemoveCustomEntries()
{
    std::unique_lock _(m_customEntriesMergeLock);

    if (!m_customEntriesMerged)
        return;

    auto loader = Red::ResourceLoader::Get();

    if (!loader)
        return;

    auto maleResource = loader->LoadAsync<CustomizationResource>(MaleResource);
    auto femaleResource = loader->LoadAsync<CustomizationResource>(FemaleResource);

    RemoveCustomEntries(maleResource);
    RemoveCustomEntries(femaleResource);

    m_customAppOverrides.clear();
    m_customEntryNames.clear();
    m_customEntriesMerged = false;
}

void App::CustomizationModule::RemoveCustomEntries(App::CustomizationResourceToken& aTargetResource)
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

void App::CustomizationModule::RemoveCustomGroups(Red::DynArray<CustomizationGroup>& aTargetGroups)
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

void App::CustomizationModule::RemoveCustomOptions(Red::DynArray<CustomizationOption>& aTargetOptions)
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

void App::CustomizationModule::ResetCustomResources()
{
    m_customMaleResources.clear();
    m_customFemaleResources.clear();
}

void App::CustomizationModule::OnActivateSystem(App::CustomizationSystem& aSystem, App::CustomizationPuppet& aPuppet,
                                                bool aIsMale, uintptr_t a4)
{
    MergeCustomEntries();

    m_customizationActive = true;
}

void App::CustomizationModule::OnDeactivateSystem(App::CustomizationSystem& aSystem)
{
    m_customizationActive = false;
}

void App::CustomizationModule::OnEnsureState(App::CustomizationSystem& aSystem, App::CustomizationState& aState)
{
    MergeCustomEntries();
}

void App::CustomizationModule::OnInitAppOption(App::CustomizationSystem& aSystem,
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

void App::CustomizationModule::OnInitMorphOption(App::CustomizationSystem& aSystem,
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

void App::CustomizationModule::OnInitSwitcherOption(App::CustomizationSystem& aSystem,
                                                    App::CustomizationPart aPartType,
                                                    App::CustomizationStateOption& aOption,
                                                    int32_t aCurrentIndex,
                                                    uint64_t a5,
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

void App::CustomizationModule::OnChangeAppearance(App::AppearanceChangerSystem& aSystem,
                                                  App::CustomizationPuppet& aPuppet,
                                                  Red::SomeIterator<Red::AppearanceDescriptor>& aOldApp,
                                                  Red::SomeIterator<Red::AppearanceDescriptor>& aNewApp,
                                                  uintptr_t a5,
                                                  uint8_t a6)
{
    if (m_customizationActive && !m_customAppOverrides.empty())
    {
        if (aOldApp)
        {
            ApplyAppOverride(aOldApp);
        }

        if (aNewApp)
        {
            ApplyAppOverride(aNewApp);
        }
    }
}
