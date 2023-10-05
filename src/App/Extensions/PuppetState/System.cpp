#include "System.hpp"
#include "Module.hpp"
#include "Red/Entity.hpp"

Red::CString App::PuppetStateSystem::GetBodyTypeSuffix(Red::ItemID& aItemID,
                                                       const Red::WeakHandle<Red::GameObject>& aOwner,
                                                       const Red::Handle<Red::ItemsFactoryAppearanceSuffixBase_Record>&)
{
    auto entityTags = Raw::Entity::Tags(aOwner.instance);
    auto visualTags = Raw::Entity::VisualTags(aOwner.instance);

    for (const auto& bodyType : PuppetStateModule::GetBodyTypes())
    {
        if (entityTags->Contains(bodyType))
        {
            return bodyType.ToString();
        }

        if (visualTags->Contains(bodyType))
        {
            return bodyType.ToString();
        }
    }

    return BaseBodyName;
}

Red::CString App::PuppetStateSystem::GetArmsStateSuffix(Red::ItemID& aItemID,
                                                        const Red::WeakHandle<Red::GameObject>& aOwner,
                                                        const Red::Handle<Red::ItemsFactoryAppearanceSuffixBase_Record>&)
{
    switch (PuppetStateModule::GetArmsState(aOwner))
    {
    case PuppetArmsState::BaseArms:
        return RTTI_ENUM_NAME_STR(PuppetArmsState::BaseArms);
    case PuppetArmsState::MantisBlades:
        return RTTI_ENUM_NAME_STR(PuppetArmsState::MantisBlades);
    case PuppetArmsState::Monowire:
        return RTTI_ENUM_NAME_STR(PuppetArmsState::Monowire);
    case PuppetArmsState::ProjectileLauncher:
        return RTTI_ENUM_NAME_STR(PuppetArmsState::ProjectileLauncher);
    default:
        return "";
    }
}

Red::CString App::PuppetStateSystem::GetFeetStateSuffix(Red::ItemID& aItemID,
                                                        const Red::WeakHandle<Red::GameObject>& aOwner,
                                                        const Red::Handle<Red::ItemsFactoryAppearanceSuffixBase_Record>&)
{
    switch (PuppetStateModule::GetFeetState(aOwner))
    {
    case PuppetFeetState::Flat:
        return RTTI_ENUM_NAME_STR(PuppetFeetState::Flat);
    case PuppetFeetState::Lifted:
        return RTTI_ENUM_NAME_STR(PuppetFeetState::Lifted);
    case PuppetFeetState::HighHeels:
        return RTTI_ENUM_NAME_STR(PuppetFeetState::HighHeels);
    case PuppetFeetState::FlatShoes:
        return RTTI_ENUM_NAME_STR(PuppetFeetState::FlatShoes);
    case PuppetFeetState::None:
    default:
        return "";
    }
}
