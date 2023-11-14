#pragma once

#include "App/Extensions/ModuleBase.hpp"
#include "App/Extensions/Customization/Unit.hpp"
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

class CustomizationModule : public ConfigurableUnitModule<CustomizationUnit>
{
public:
    std::string_view GetName() override;
    bool Load() override;
    void PostLoad() override;
    void Reload() override;
    bool Unload() override;

private:
    void RegisterCustomEntryName(Red::CName aName);
    void RegisterCustomEntryName(Red::CName aName, const Red::CString& aSubName);
    bool IsCustomEntryName(Red::CName aName);
    bool IsCustomEntryName(Red::CName aName, const Red::CString& aSubName);

    void RegisterAppOverride(Red::ResourcePath aOriginal, Red::ResourcePath aOverride, Red::CName aName);
    void ApplyAppOverride(Red::AppearanceDescriptor& aAppearance);

    void PrefetchCustomResources();
    void PrefetchCustomResources(Core::Vector<CustomizationResourceToken>& aResources,
                                 const Core::Vector<std::string>& aPaths);

    void MergeCustomEntries();
    void MergeCustomEntries(CustomizationResourceToken& aTargetResource,
                            Core::Vector<CustomizationResourceToken>& aSourceResources);
    void MergeCustomGroups(Red::DynArray<CustomizationGroup>& aTargetGroups,
                           Red::DynArray<CustomizationGroup>& aSourceGroups);
    void MergeCustomOptions(Red::DynArray<CustomizationOption>& aTargetOptions,
                            Red::DynArray<CustomizationOption>& aSourceOptions);

    void RemoveCustomEntries();
    void RemoveCustomEntries(CustomizationResourceToken& aTargetResource);
    void RemoveCustomGroups(Red::DynArray<CustomizationGroup>& aTargetGroups);
    void RemoveCustomOptions(Red::DynArray<CustomizationOption>& aTargetOptions);

    void ResetCustomResources();

    void OnActivateSystem(CustomizationSystem& aSystem, CustomizationPuppet& aPuppet, bool aIsMale, uintptr_t a4);
    void OnDeactivateSystem(CustomizationSystem& aSystem);
    void OnEnsureState(CustomizationSystem& aSystem, CustomizationState& aState);
    void OnInitAppOption(CustomizationSystem& aSystem,
                         CustomizationPart aPartType,
                         CustomizationStateOption& aOption,
                         Red::SortedUniqueArray<Red::CName>& aStateOptions,
                         Red::Map<Red::CName, CustomizationStateOption>& aUiSlots);
    void OnInitMorphOption(CustomizationSystem& aSystem,
                           CustomizationStateOption& aOption,
                           Red::SortedUniqueArray<Red::CName>& aStateOptions,
                           Red::Map<Red::CName, CustomizationStateOption>& aUiSlots);
    void OnInitSwitcherOption(CustomizationSystem& aSystem,
                              CustomizationPart aPartType,
                              CustomizationStateOption& aOption,
                              int32_t aCurrentIndex,
                              uint64_t a5,
                              Red::Map<Red::CName, CustomizationStateOption>& aUiSlots);
    void OnChangeAppearance(AppearanceChangerSystem& aSystem,
                            CustomizationPuppet& aPuppet,
                            Red::Range<Red::AppearanceDescriptor>& aOldApp,
                            Red::Range<Red::AppearanceDescriptor>& aNewApp,
                            uintptr_t a5,
                            uint8_t a6);

    Core::Vector<CustomizationResourceToken> m_customMaleResources;
    Core::Vector<CustomizationResourceToken> m_customFemaleResources;
    Core::Map<Red::AppearanceDescriptor::Hash, Red::AppearanceDescriptor> m_customAppOverrides;
    Core::Set<Red::CName> m_customEntryNames;
    std::mutex m_customEntriesMergeLock;
    bool m_customEntriesMerged;
    bool m_customizationActive;
};
}
