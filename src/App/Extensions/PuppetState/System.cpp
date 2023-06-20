#include "System.hpp"
#include "Module.hpp"
#include "Red/Entity.hpp"

Red::CString App::PuppetStateSystem::GetBodyTypeSuffix(Red::ItemID aItemID,
                                                       const Red::WeakHandle<Red::GameObject>& aOwner,
                                                       const Red::Handle<Red::ItemsFactoryAppearanceSuffixBase_Record>&)
{
    auto entityTags = Raw::Entity::Tags(aOwner.instance);
    auto visualTags = Raw::Entity::VisualTags(aOwner.instance);

    for (const auto& bodyType : PuppetStateModule::s_bodyTypes)
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
