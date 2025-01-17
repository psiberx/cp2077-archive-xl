#include "Extension.hpp"
#include "Red/Entity.hpp"
#include "Red/TweakDB.hpp"

namespace
{
constexpr auto ExtensionName = "PuppetState";

constexpr auto BodyTagPrefix = Red::CName("Body:");
constexpr auto SuffixListID = Red::TweakDBID("itemsFactoryAppearanceSuffix.ItemsFactoryAppearanceSuffixOrderDefault");
}

std::string_view App::PuppetStateExtension::GetName()
{
    return ExtensionName;
}

bool App::PuppetStateExtension::Load()
{
    HookBefore<Raw::CharacterCustomizationGenitalsController::OnAttach>(&OnAttachPuppet).OrThrow();
    HookBefore<Raw::CharacterCustomizationHairstyleController::OnDetach>(&OnDetachPuppet).OrThrow();

    s_baseBodyType = Red::CNamePool::Add(BaseBodyName);

    return true;
}

bool App::PuppetStateExtension::Unload()
{
    Unhook<Raw::CharacterCustomizationGenitalsController::OnAttach>();
    Unhook<Raw::CharacterCustomizationHairstyleController::OnDetach>();

    return true;
}

void App::PuppetStateExtension::Configure()
{
    s_bodyTypes.clear();
    s_bodyTags.clear();

    for (const auto& unit : m_configs)
    {
        for (const auto& bodyTypeName : unit.bodyTypes)
        {
            auto bodyType = Red::CNamePool::Add(bodyTypeName.c_str());
            s_bodyTypes.emplace(bodyType);
            s_bodyTags.emplace(Red::FNV1a64(bodyTypeName.c_str(), BodyTagPrefix), bodyType);
        }
    }
}

void App::PuppetStateExtension::OnTweakDBReady()
{
    CreateSuffixRecord(BodyTypeSuffixID,
                       RTTI_TYPE_NAME(PuppetStateSystem),
                       RTTI_FUNC_NAME(PuppetStateSystem::GetBodyTypeSuffix));

    CreateSuffixRecord(ArmsStateSuffixID,
                       RTTI_TYPE_NAME(PuppetStateSystem),
                       RTTI_FUNC_NAME(PuppetStateSystem::GetArmsStateSuffix));

    CreateSuffixRecord(LegsStateSuffixID,
                       RTTI_TYPE_NAME(PuppetStateSystem),
                       RTTI_FUNC_NAME(PuppetStateSystem::GetFeetStateSuffix));

    CreateSuffixRecord(FeetStateSuffixID,
                       RTTI_TYPE_NAME(PuppetStateSystem),
                       RTTI_FUNC_NAME(PuppetStateSystem::GetFeetStateSuffix));

    ActivateSuffixRecords({
        BodyTypeSuffixID,
        ArmsStateSuffixID,
        LegsStateSuffixID,
        FeetStateSuffixID,
    });
}

void App::PuppetStateExtension::OnAttachPuppet(Red::gameuiCharacterCustomizationGenitalsController* aComponent)
{
#ifndef NDEBUG
    LogDebug("[{}] [event=AttachPuppet ent={}]", ExtensionName, aComponent->owner->entityID.hash);
#endif

    std::unique_lock _(s_mutex);

    auto owner = aComponent->owner;

    auto it = s_handlers.find(owner);
    if (it != s_handlers.end())
        return;

    auto handler = Red::MakeHandle<PuppetStateHandler>(owner);
    s_handlers.insert({owner, handler});

    auto transactionSystem = Red::GetGameSystem<Red::ITransactionSystem>();
    transactionSystem->RegisterSlotListener(owner, handler);
}

void App::PuppetStateExtension::OnDetachPuppet(Red::gameuiCharacterCustomizationHairstyleController* aComponent, uintptr_t)
{
#ifndef NDEBUG
    LogDebug("[{}] [event=DetachPuppet ent={}]", ExtensionName, aComponent->owner->entityID.hash);
#endif

    std::unique_lock _(s_mutex);

    auto owner = aComponent->owner;

    auto it = s_handlers.find(owner);
    if (it == s_handlers.end())
        return;

    auto& handler = it.value();
    s_handlers.erase(owner);

    auto transactionSystem = Red::GetGameSystem<Red::ITransactionSystem>();
    transactionSystem->UnregisterSlotListener(owner, handler);
}

App::PuppetArmsState App::PuppetStateExtension::GetArmsState(const Red::WeakHandle<Red::GameObject>& aPuppet)
{
    std::shared_lock _(s_mutex);

    auto it = s_handlers.find(aPuppet.instance);
    if (it == s_handlers.end())
        return PuppetArmsState::BaseArms;

    return it.value()->GetArmsState();
}

App::PuppetFeetState App::PuppetStateExtension::GetFeetState(const Red::WeakHandle<Red::GameObject>& aPuppet)
{
    std::shared_lock _(s_mutex);

    auto it = s_handlers.find(aPuppet.instance);
    if (it == s_handlers.end())
        return PuppetFeetState::None;

    return it.value()->GetFeetState();
}

Red::CName App::PuppetStateExtension::GetBodyType(const Red::WeakHandle<Red::GameObject>& aPuppet)
{
    if (!aPuppet || s_bodyTags.empty())
    {
        return s_baseBodyType;
    }

    const auto& entityTags = Raw::Entity::EntityTags::Ref(aPuppet.instance);
    const auto& visualTags = Raw::Entity::VisualTags::Ref(aPuppet.instance);

    if (!entityTags.IsEmpty() || !visualTags.IsEmpty())
    {
        for (const auto& [bodyTag, bodyType] : s_bodyTags)
        {
            if (entityTags.Contains(bodyTag))
            {
                return bodyType;
            }

            if (visualTags.Contains(bodyTag))
            {
                return bodyType;
            }
        }
    }

    for (const auto& component : aPuppet.instance->components | std::views::reverse)
    {
        const auto& morphTarget = Red::Cast<Red::entMorphTargetSkinnedMeshComponent>(component);

        for (const auto& [bodyTag, bodyType] : s_bodyTags)
        {
            if (component->name == bodyTag)
            {
                return bodyType;
            }

            if (morphTarget)
            {
                if (morphTarget->tags.Contains(bodyTag))
                {
                    return bodyType;
                }
            }
        }
    }

    return s_baseBodyType;
}

const Core::Set<Red::CName>& App::PuppetStateExtension::GetBodyTypes()
{
    return s_bodyTypes;
}

const Core::Map<Red::CName, Red::CName>& App::PuppetStateExtension::GetBodyTags()
{
    return s_bodyTags;
}

void App::PuppetStateExtension::CreateSuffixRecord(Red::TweakDBID aSuffixID, Red::CName aSystemName,
                                                Red::CName aFunctionName)
{
    if (!Red::RecordExists(aSuffixID))
    {
        Red::CreateFlat(aSuffixID, ".instantSwitch", false);
        Red::CreateFlat(aSuffixID, ".scriptedSystem", aSystemName);
        Red::CreateFlat(aSuffixID, ".scriptedFunction", aFunctionName);
        Red::CreateRecord(aSuffixID, "ItemsFactoryAppearanceSuffixBase");
    }
}

void App::PuppetStateExtension::CreateSuffixAlias(Red::TweakDBID aSuffixID, Red::TweakDBID aAliasID)
{
    if (!Red::RecordExists(aAliasID))
    {
        Red::CreateRecordAlias(aSuffixID, aAliasID);
    }
}

void App::PuppetStateExtension::ActivateSuffixRecords(const Core::Vector<Red::TweakDBID>& aSuffixIDs)
{
    Red::AppendToFlat(SuffixListID, ".appearanceSuffixes", aSuffixIDs);
    Red::UpdateRecord(SuffixListID);
}
