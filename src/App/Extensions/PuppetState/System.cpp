#include "System.hpp"
#include "Module.hpp"
#include "Red/Entity.hpp"

Red::CString App::PuppetStateSystem::GetBodyTypeSuffix(Red::ItemID& aItemID,
                                                       const Red::WeakHandle<Red::GameObject>& aOwner,
                                                       const Red::Handle<Red::ItemsFactoryAppearanceSuffixBase_Record>&)
{
    if (!aOwner)
    {
        return BaseBodyName;
    }

    const auto& bodyTags = PuppetStateModule::GetBodyTags();

    if (bodyTags.empty())
    {
        return BaseBodyName;
    }

    const auto& entityTags = Raw::Entity::EntityTags::Ref(aOwner.instance);
    const auto& visualTags = Raw::Entity::VisualTags::Ref(aOwner.instance);

    if (!entityTags.IsEmpty() || !visualTags.IsEmpty())
    {
        for (const auto& [bodyTag, bodyType] : bodyTags)
        {
            if (entityTags.Contains(bodyTag))
            {
                return bodyType.ToString();
            }

            if (visualTags.Contains(bodyTag))
            {
                return bodyType.ToString();
            }
        }
    }

    const auto& components = Raw::Entity::ComponentsStorage::Ref(aOwner.instance).components;

    for (const auto& component : components | std::views::reverse)
    {
        const auto& morphTarget = Red::Cast<Red::entMorphTargetSkinnedMeshComponent>(component);

        for (const auto& [bodyTag, bodyType] : bodyTags)
        {
            if (component->name == bodyTag)
            {
                return bodyType.ToString();
            }

            if (morphTarget)
            {
                if (morphTarget->tags.Contains(bodyTag))
                {
                    return bodyType.ToString();
                }
            }
        }
    }

    return BaseBodyName;
}

Red::CString App::PuppetStateSystem::GetArmsStateSuffix(Red::ItemID& aItemID,
                                                        const Red::WeakHandle<Red::GameObject>& aOwner,
                                                        const Red::Handle<Red::ItemsFactoryAppearanceSuffixBase_Record>&)
{
    constexpr auto BaseArmsStr = RTTI_ENUM_NAME_STR(PuppetArmsState::BaseArms);
    constexpr auto MantisBladesStr = RTTI_ENUM_NAME_STR(PuppetArmsState::MantisBlades);
    constexpr auto MonowireStr = RTTI_ENUM_NAME_STR(PuppetArmsState::Monowire);
    constexpr auto ProjectileLauncherStr = RTTI_ENUM_NAME_STR(PuppetArmsState::ProjectileLauncher);

    switch (PuppetStateModule::GetArmsState(aOwner))
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

    switch (PuppetStateModule::GetFeetState(aOwner))
    {
    case PuppetFeetState::Flat: return FlatStr;
    case PuppetFeetState::Lifted: return LiftedStr;
    case PuppetFeetState::HighHeels: return HighHeelsStr;
    case PuppetFeetState::FlatShoes: return FlatShoesStr;
    default: return "";
    }
}
