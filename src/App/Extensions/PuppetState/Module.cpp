#include "Module.hpp"
#include "Red/Entity.hpp"
#include "Red/TweakDB.hpp"

namespace
{
constexpr auto ModuleName = "PuppetState";

constexpr auto LegsStateSuffixID = Red::TweakDBID("itemsFactoryAppearanceSuffix.LegsState");
constexpr auto BodyTypeSuffixID = Red::TweakDBID("itemsFactoryAppearanceSuffix.BodyType");
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

    if (!HookAfter<Raw::CharacterCustomizationGenitalsController::OnAttach>(&OnAttachPuppet))
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

    for (const auto& unit : m_units)
    {
        for (const auto& bodyType : unit.bodyTypes)
        {
            s_bodyTypes.emplace(bodyType.c_str());
        }
    }
}

void App::PuppetStateModule::OnLoadTweakDB()
{
    CreateSuffixRecord(LegsStateSuffixID, Red::GetTypeName<PuppetStateSystem>(), "GetLegsStateSuffix");
    CreateSuffixRecord(BodyTypeSuffixID, Red::GetTypeName<PuppetStateSystem>(), "GetBodyTypeSuffix");

    ActivateSuffixRecords({LegsStateSuffixID, BodyTypeSuffixID});
}

void App::PuppetStateModule::OnAttachPuppet(Red::gameuiCharacterCustomizationGenitalsController * aComponent)
{
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
    auto owner = Raw::IComponent::Owner::Ptr(aComponent);

    auto it = s_handlers.find(owner);
    if (it == s_handlers.end())
        return;

    auto handler = it.value();
    s_handlers.erase(owner);

    auto transactionSystem = Red::GetGameSystem<Red::ITransactionSystem>();
    transactionSystem->RegisterSlotListener(owner, handler);
}

App::PuppetFeetState App::PuppetStateModule::GetFeetState(const Red::WeakHandle<Red::GameObject>& aPuppet)
{
    auto it = s_handlers.find(aPuppet.instance);
    if (it == s_handlers.end())
        return PuppetFeetState::None;

    return it.value()->GetFeetState();
}

const Core::Set<Red::CName>& App::PuppetStateModule::GetBodyTypes()
{
    return s_bodyTypes;
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

void App::PuppetStateModule::ActivateSuffixRecords(const Core::Vector<Red::TweakDBID>& aSuffixIDs)
{
    Red::AppendToFlat(SuffixListID, ".appearanceSuffixes", aSuffixIDs);
    Red::UpdateRecord(SuffixListID);
}
