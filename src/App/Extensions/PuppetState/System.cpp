#include "System.hpp"
#include "Module.hpp"
#include "Red/Entity.hpp"

Red::CString App::PuppetStateSystem::GetBodyTypeSuffix(Red::ItemID aItemID,
                                                       const Red::WeakHandle<Red::GameObject>& aOwner,
                                                       const Red::Handle<Red::ItemsFactoryAppearanceSuffixBase_Record>&)
{
    Raw::Entity::Tags entityTags(aOwner.instance);
    Raw::Entity::VisualTags visualTags(aOwner.instance);

    for (const auto& bodyType : PuppetStateModule::s_bodyTypes)
    {
        for (const auto& tag : *entityTags)
        {
            if (tag == bodyType)
            {
                return bodyType.ToString();
            }
        }

        for (const auto& tag : *visualTags)
        {
            if (tag == bodyType)
            {
                return bodyType.ToString();
            }
        }
    }

    return "";
}
