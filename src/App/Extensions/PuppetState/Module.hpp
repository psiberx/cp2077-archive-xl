#pragma once

#include "App/Extensions/ModuleBase.hpp"
#include "App/Extensions/PuppetState/Handler.hpp"
#include "App/Extensions/PuppetState/System.hpp"
#include "App/Extensions/PuppetState/Unit.hpp"
#include "Red/CharacterCustomization.hpp"

namespace App
{
class PuppetStateModule : public ConfigurableUnitModule<PuppetStateUnit>
{
public:
    static constexpr auto BodyTypeSuffixID = Red::TweakDBID("itemsFactoryAppearanceSuffix.BodyType");
    static constexpr auto ArmsStateSuffixID = Red::TweakDBID("itemsFactoryAppearanceSuffix.ArmsState");
    static constexpr auto FeetStateSuffixID = Red::TweakDBID("itemsFactoryAppearanceSuffix.FeetState");
    static constexpr auto LegsStateSuffixID = Red::TweakDBID("itemsFactoryAppearanceSuffix.LegsState");

    bool Load() override;
    void Reload() override;
    bool Unload() override;

    std::string_view GetName() override;

    static const Core::Set<Red::CName>& GetBodyTypes();

    static PuppetArmsState GetArmsState(const Red::WeakHandle<Red::GameObject>& aPuppet);
    static PuppetFeetState GetFeetState(const Red::WeakHandle<Red::GameObject>& aPuppet);

private:
    void FillBodyTypes();

    static void OnLoadTweakDB();
    static void OnAttachPuppet(Red::gameuiCharacterCustomizationGenitalsController* aComponent);
    static void OnDetachPuppet(Red::gameuiCharacterCustomizationHairstyleController* aComponent, uintptr_t);

    static void CreateSuffixRecord(Red::TweakDBID aSuffixID, Red::CName aSystemName, Red::CName aFunctionName);
    static void CreateSuffixAlias(Red::TweakDBID aSuffixID, Red::TweakDBID aAliasID);
    static void ActivateSuffixRecords(const Core::Vector<Red::TweakDBID>& aSuffixIDs);

    inline static Core::Set<Red::CName> s_bodyTypes;
    inline static Core::Map<Red::Entity*, Red::Handle<PuppetStateHandler>> s_handlers;
    static inline std::shared_mutex s_mutex;
};
}
