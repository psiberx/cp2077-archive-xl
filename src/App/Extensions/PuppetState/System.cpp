#include "System.hpp"
#include "Extension.hpp"

Red::CString App::PuppetStateSystem::GetBodyTypeSuffix(Red::ItemID& aItemID,
                                                       const Red::WeakHandle<Red::GameObject>& aOwner,
                                                       const Red::Handle<Red::ItemsFactoryAppearanceSuffixBase_Record>&)
{
    return PuppetStateExtension::GetBodyType(aOwner).ToString();
}

Red::CString App::PuppetStateSystem::GetArmsStateSuffix(Red::ItemID& aItemID,
                                                        const Red::WeakHandle<Red::GameObject>& aOwner,
                                                        const Red::Handle<Red::ItemsFactoryAppearanceSuffixBase_Record>&)
{
    constexpr auto BaseArmsStr = RTTI_ENUM_NAME_STR(PuppetArmsState::BaseArms);
    constexpr auto MantisBladesStr = RTTI_ENUM_NAME_STR(PuppetArmsState::MantisBlades);
    constexpr auto MonowireStr = RTTI_ENUM_NAME_STR(PuppetArmsState::Monowire);
    constexpr auto ProjectileLauncherStr = RTTI_ENUM_NAME_STR(PuppetArmsState::ProjectileLauncher);

    switch (PuppetStateExtension::GetArmsState(aOwner))
    {
    case PuppetArmsState::BaseArms: return BaseArmsStr;
    case PuppetArmsState::MantisBlades: return MantisBladesStr;
    case PuppetArmsState::Monowire: return MonowireStr;
    case PuppetArmsState::ProjectileLauncher: return ProjectileLauncherStr;
    default: return "";
    }
}

Red::CString App::PuppetStateSystem::GetFeetStateSuffix(Red::ItemID& aItemID,
                                                        const Red::WeakHandle<Red::GameObject>& aOwner,
                                                        const Red::Handle<Red::ItemsFactoryAppearanceSuffixBase_Record>&)
{
    constexpr auto FlatStr = RTTI_ENUM_NAME_STR(PuppetFeetState::Flat);
    constexpr auto LiftedStr = RTTI_ENUM_NAME_STR(PuppetFeetState::Lifted);
    constexpr auto HighHeelsStr = RTTI_ENUM_NAME_STR(PuppetFeetState::HighHeels);
    constexpr auto FlatShoesStr = RTTI_ENUM_NAME_STR(PuppetFeetState::FlatShoes);

    switch (PuppetStateExtension::GetFeetState(aOwner))
    {
    case PuppetFeetState::Flat: return FlatStr;
    case PuppetFeetState::Lifted: return LiftedStr;
    case PuppetFeetState::HighHeels: return HighHeelsStr;
    case PuppetFeetState::FlatShoes: return FlatShoesStr;
    default: return "";
    }
}
