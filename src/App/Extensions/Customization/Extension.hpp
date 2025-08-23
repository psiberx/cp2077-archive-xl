#pragma once

#include "App/Extensions/ExtensionBase.hpp"
#include "App/Extensions/Customization/Config.hpp"
#include "Red/AppearanceChanger.hpp"
#include "Red/CharacterCustomization.hpp"

namespace App
{
using CustomizationResource = Red::game::ui::CharacterCustomizationInfoResource;
using CustomizationResourceToken = Red::SharedPtr<Red::ResourceToken<CustomizationResource>>;
using CustomizationGroup = Red::game::ui::OptionsGroup;
using CustomizationOption = Red::Handle<Red::game::ui::CharacterCustomizationInfo>;
using CustomizationAppearance = Red::Handle<Red::game::ui::AppearanceInfo>;
using CustomizationMorph = Red::Handle<Red::game::ui::MorphInfo>;
using CustomizationSwitcher = Red::Handle<Red::game::ui::SwitcherInfo>;
using CustomizationSystem = Red::game::ui::ICharacterCustomizationSystem;
using CustomizationState = Red::Handle<Red::game::ui::CharacterCustomizationState>;
using CustomizationStateOption = Red::Handle<Red::game::ui::CharacterCustomizationOption>;
using CustomizationPuppet = Red::Handle<Red::game::Puppet>;
using CustomizationPuppetWeak = Red::WeakHandle<Red::game::Puppet>;
using CustomizationPart = Red::game::ui::CharacterCustomizationPart;
using AppearanceChangerSystem = Red::world::RuntimeSystemEntityAppearanceChanger;

class CustomizationExtension : public ConfigurableExtensionImpl<CustomizationConfig>
{
public:
    std::string_view GetName() override;
    bool Load() override;
    void OnDepotReady() override;
    void Reload() override;
    bool Unload() override;

    static void FixCustomizationAppearance(Red::AppearanceResource* aResource,
                                           Red::Handle<Red::AppearanceDefinition>* aDefinition,
                                           Red::CName aAppearanceName);
    static void FixCustomizationComponents(const Red::Handle<Red::AppearanceResource>& aResource,
                                           const Red::Handle<Red::AppearanceDefinition>& aDefinition,
                                           Red::DynArray<Red::Handle<Red::ISerializable>>& aComponents);
    static bool IsFixedCustomizationAppearance(const Red::Handle<Red::AppearanceDefinition>& aDefinition);

private:
    void OnActivateSystem(CustomizationSystem* aSystem, CustomizationPuppet& aPuppet, bool aIsMale, uintptr_t a4);
    void OnDeactivateSystem(CustomizationSystem* aSystem);
    void OnPrepareResource(CustomizationSystem* aSystem,
                           Red::SharedPtr<Red::ResourceToken<Red::gameuiCharacterCustomizationInfoResource>>& aOut,
                           bool aIsMale);
    void OnInitAppOption(CustomizationSystem* aSystem,
                         CustomizationPart aPartType,
                         CustomizationStateOption& aOption,
                         Red::SortedUniqueArray<Red::CName>& aStateOptions,
                         Red::Map<Red::CName, CustomizationStateOption>& aUiSlots);
    void OnInitMorphOption(CustomizationSystem* aSystem,
                           CustomizationStateOption& aOption,
                           Red::SortedUniqueArray<Red::CName>& aStateOptions,
                           Red::Map<Red::CName, CustomizationStateOption>& aUiSlots);
    void OnInitSwitcherOption(CustomizationSystem* aSystem,
                              CustomizationPart aPartType,
                              CustomizationStateOption& aOption,
                              int32_t aCurrentIndex,
                              uint64_t a5,
                              Red::Map<Red::CName, CustomizationStateOption>& aUiSlots);
    void OnGetAppearances(Red::gameuiICharacterCustomizationState* aState, Red::CName aGroupName, bool aIsFPP,
                          Red::DynArray<Red::AppearanceDescriptor>& aAppearances);
    void OnChangeAppearance(AppearanceChangerSystem& aSystem,
                            Red::AppearanceChangeRequest* aRequest,
                            uintptr_t a3);
    void OnChangeAppearances(AppearanceChangerSystem& aSystem,
                            CustomizationPuppet& aPuppet,
                            Red::Range<Red::AppearanceDescriptor>& aOldApp,
                            Red::Range<Red::AppearanceDescriptor>& aNewApp,
                            uintptr_t a5,
                            uint8_t a6);

    void RegisterCustomEntryName(Red::CName aName);
    void RegisterCustomEntryName(Red::CName aName, const Red::CString& aSubName);
    bool IsCustomEntryName(Red::CName aName);
    bool IsCustomEntryName(Red::CName aName, const Red::CString& aSubName);

    void RegisterAppOverride(Red::ResourcePath aResource, Red::ResourcePath aOverride, Red::CName aAppearance);
    void ApplyAppOverride(Red::AppearanceDescriptor& aAppearance);

    void PrefetchCustomResources();
    void PrefetchCustomResources(Core::Vector<CustomizationResourceToken>& aResources,
                                 const Core::Vector<std::string>& aPaths);

    void MergeCustomEntries(CustomizationSystem* aSystem);
    void MergeCustomEntries(CustomizationResourceToken& aTargetResource,
                            Core::Vector<CustomizationResourceToken>& aSourceResources);
    void MergeCustomGroups(Red::DynArray<CustomizationGroup>& aTargetGroups,
                           Red::DynArray<CustomizationGroup>& aSourceGroups);
    void MergeCustomOptions(Red::DynArray<CustomizationOption>& aTargetOptions,
                            Red::DynArray<CustomizationOption>& aSourceOptions,
                            bool aSlotsAndLinks);

    void ExpandCustomizationOptions(Red::DynArray<CustomizationOption>& aSourceOptions);
    void RegenerateIndexes(Red::DynArray<CustomizationOption>& aTargetOptions);

    void RemoveCustomEntries();
    void RemoveCustomEntries(CustomizationResourceToken& aTargetResource);
    void RemoveCustomGroups(Red::DynArray<CustomizationGroup>& aTargetGroups);
    void RemoveCustomOptions(Red::DynArray<CustomizationOption>& aTargetOptions);

    void ResetCustomResources();

    void FixCustomizationOptions(Red::ResourcePath aTargetPath, Red::DynArray<CustomizationOption>& aTargetOptions);

    CustomizationResourceToken m_baseMaleResource;
    CustomizationResourceToken m_baseFemaleResource;
    Core::Vector<CustomizationResourceToken> m_customMaleResources;
    Core::Vector<CustomizationResourceToken> m_customFemaleResources;
    Core::Map<Red::AppearanceDescriptor::Hash, Red::AppearanceDescriptor> m_customAppOverrides;
    Core::Set<Red::CName> m_customEntryNames;
    Red::SharedSpinLock m_customEntriesMergeLock;
    Core::Set<Red::CName> m_hairColorNames;
    bool m_customEntriesMerged;
    bool m_customizationActive;
};
}
