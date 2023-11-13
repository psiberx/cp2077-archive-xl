#include "Module.hpp"
#include "Red/Entity.hpp"
#include "Red/TweakDB.hpp"

namespace
{
constexpr auto ModuleName = "PuppetState";

constexpr auto BodyTagPrefix = Red::CName("Body:");
constexpr auto SuffixListID = Red::TweakDBID("itemsFactoryAppearanceSuffix.ItemsFactoryAppearanceSuffixOrderDefault");
}

std::string_view App::PuppetStateModule::GetName()
{
    return ModuleName;
}

bool App::PuppetStateModule::Load()
{
    if (!HookAfter<Raw::LoadTweakDB>(&OnLoadTweakDB))
        throw std::runtime_error("Failed to hook [LoadTweakDB].");

    if (!HookBefore<Raw::CharacterCustomizationGenitalsController::OnAttach>(&OnAttachPuppet))
        throw std::runtime_error("Failed to hook [CharacterCustomizationGenitalsController::OnAttach].");

    if (!HookBefore<Raw::CharacterCustomizationHairstyleController::OnDetach>(&OnDetachPuppet))
        throw std::runtime_error("Failed to hook [CharacterCustomizationHairstyleController::OnDetach].");

    FillBodyTypes();

    return true;
}

bool App::PuppetStateModule::Unload()
{
    Unhook<Raw::LoadTweakDB>();
    Unhook<Raw::CharacterCustomizationGenitalsController::OnAttach>();
    Unhook<Raw::CharacterCustomizationHairstyleController::OnDetach>();

    return true;
}

void App::PuppetStateModule::Reload()
{
    FillBodyTypes();
}

void App::PuppetStateModule::FillBodyTypes()
{
    s_bodyTypes.clear();
    s_bodyTags.clear();

    for (const auto& unit : m_units)
    {
        for (const auto& bodyTypeName : unit.bodyTypes)
        {
            auto bodyType = Red::CNamePool::Add(bodyTypeName.c_str());
            s_bodyTypes.emplace(bodyType);
            s_bodyTags.emplace(Red::FNV1a64(bodyTypeName.c_str(), BodyTagPrefix), bodyType);
        }
    }
}

void App::PuppetStateModule::OnLoadTweakDB()
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

void App::PuppetStateModule::OnAttachPuppet(Red::gameuiCharacterCustomizationGenitalsController* aComponent)
{
#ifndef NDEBUG
    const auto entity = Raw::IComponent::Owner::Ptr(aComponent);
    const auto entityID = Raw::Entity::EntityID::Ptr(entity);
    LogDebug("|{}| [event=AttachPuppet ent={}]", ModuleName, entityID->hash);
#endif

    std::unique_lock _(s_mutex);

    auto owner = Raw::IComponent::Owner::Ptr(aComponent);

    auto it = s_handlers.find(owner);
    if (it != s_handlers.end())
        return;

    auto handler = Red::MakeHandle<PuppetStateHandler>(owner);
    s_handlers.insert({owner, handler});

    auto transactionSystem = Red::GetGameSystem<Red::ITransactionSystem>();
    transactionSystem->RegisterSlotListener(owner, handler);
}

void App::PuppetStateModule::OnDetachPuppet(Red::gameuiCharacterCustomizationHairstyleController* aComponent, uintptr_t)
{
#ifndef NDEBUG
    const auto entity = Raw::IComponent::Owner::Ptr(aComponent);
    const auto entityID = Raw::Entity::EntityID::Ptr(entity);
    LogDebug("|{}| [event=DetachPuppet ent={}]", ModuleName, entityID->hash);
#endif

    std::unique_lock _(s_mutex);

    auto owner = Raw::IComponent::Owner::Ptr(aComponent);

    auto it = s_handlers.find(owner);
    if (it == s_handlers.end())
        return;

    auto& handler = it.value();
    s_handlers.erase(owner);

    auto transactionSystem = Red::GetGameSystem<Red::ITransactionSystem>();
    transactionSystem->UnregisterSlotListener(owner, handler);
}

App::PuppetArmsState App::PuppetStateModule::GetArmsState(const Red::WeakHandle<Red::GameObject>& aPuppet)
{
    std::shared_lock _(s_mutex);

    auto it = s_handlers.find(aPuppet.instance);
    if (it == s_handlers.end())
        return PuppetArmsState::BaseArms;

    return it.value()->GetArmsState();
}

App::PuppetFeetState App::PuppetStateModule::GetFeetState(const Red::WeakHandle<Red::GameObject>& aPuppet)
{
    std::shared_lock _(s_mutex);

    auto it = s_handlers.find(aPuppet.instance);
    if (it == s_handlers.end())
        return PuppetFeetState::None;

    return it.value()->GetFeetState();
}

const Core::Set<Red::CName>& App::PuppetStateModule::GetBodyTypes()
{
    return s_bodyTypes;
}

const Core::Map<Red::CName, Red::CName>& App::PuppetStateModule::GetBodyTags()
{
    return s_bodyTags;
}

void App::PuppetStateModule::CreateSuffixRecord(Red::TweakDBID aSuffixID, Red::CName aSystemName,
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

void App::PuppetStateModule::CreateSuffixAlias(Red::TweakDBID aSuffixID, Red::TweakDBID aAliasID)
{
    if (!Red::RecordExists(aAliasID))
    {
        Red::CreateRecordAlias(aSuffixID, aAliasID);
    }
}

void App::PuppetStateModule::ActivateSuffixRecords(const Core::Vector<Red::TweakDBID>& aSuffixIDs)
{
    Red::AppendToFlat(SuffixListID, ".appearanceSuffixes", aSuffixIDs);
    Red::UpdateRecord(SuffixListID);
}
