#include "System.hpp"
#include "Module.hpp"
#include "Red/Entity.hpp"

Red::CString App::PuppetStateSystem::GetBodyTypeSuffix(Red::ItemID aItemID,
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

    return "";
}

Red::CString App::PuppetStateSystem::GetLegsStateSuffix(Red::ItemID aItemID,
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
