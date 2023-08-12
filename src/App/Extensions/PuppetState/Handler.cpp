#include "Handler.hpp"
#include "App/Extensions/AttachmentSlots/Module.hpp"
#include "App/Extensions/GarmentOverride/Module.hpp"
#include "App/Extensions/PuppetState/Module.hpp"
#include "Red/ItemObject.hpp"
#include "Red/TweakDB.hpp"

namespace
{
constexpr auto TorsoSlot = Red::TweakDBID("AttachmentSlots.Torso");
constexpr auto HandsSlot = Red::TweakDBID("AttachmentSlots.Hands");
constexpr auto FeetSlot = Red::TweakDBID("AttachmentSlots.Feet");

constexpr auto WeaponLeftSlot = Red::TweakDBID("AttachmentSlots.WeaponLeft");
constexpr auto WeaponRightSlot = Red::TweakDBID("AttachmentSlots.WeaponRight");

constexpr auto HideInnerSleevesTag = Red::CName("hide_T1part");
constexpr auto HideFootwearTag = Red::CName("hide_S1");

constexpr auto HighHeelsTag = Red::CName("HighHeels");
constexpr auto FlatShoesTag = Red::CName("FlatShoes");
constexpr auto ForceFlatFeetTag = App::GarmentOverrideModule::ForceFlatFeetTag;

constexpr auto InnerSleevesSuffix = Red::TweakDBID("itemsFactoryAppearanceSuffix.Partial");
constexpr auto ArmsStateSuffix = App::PuppetStateModule::ArmsStateSuffixID;
constexpr auto FeetStateSuffix = App::PuppetStateModule::FeetStateSuffixID;
constexpr auto LegsStateSuffix = App::PuppetStateModule::LegsStateSuffixID;

constexpr auto EmptyAppearanceName = Red::CName("empty_appearance_default");
constexpr auto MaleGenderName = Red::CName("Male");

constexpr auto MantisBladesName = "mantis_blade";
constexpr auto MonowireName = "mono_wires";
constexpr auto ProjectileLauncher = "projectile_launcher";
}

App::PuppetStateHandler::PuppetStateHandler(Red::Entity* aPuppet)
    : m_puppetWeak(Red::ToWeakHandle(aPuppet))
    , m_transactionSystem(Red::GetGameSystem<Red::ITransactionSystem>())
    , m_torsoSlots(AttachmentSlotsModule::GetRelatedSlots(TorsoSlot))
    , m_feetSlots(AttachmentSlotsModule::GetRelatedSlots(FeetSlot))
    , m_torsoDependentSlots(AttachmentSlotsModule::GetDependentSlots(TorsoSlot))
    , m_handsDependentSlots(AttachmentSlotsModule::GetDependentSlots(HandsSlot))
    , m_feetDependentSlots(AttachmentSlotsModule::GetDependentSlots(FeetSlot))
    , m_armsState(PuppetArmsState::BaseArms)
    , m_feetState(PuppetFeetState::None)
{
    UpdateFeetState(m_puppetWeak.Lock());
}

void App::PuppetStateHandler::OnItemEquipped(Red::ItemID& aItemID, Red::TweakDBID aSlotID)
{
    HandleAppearanceChange(aItemID, aSlotID, true);
}

void App::PuppetStateHandler::OnItemEquippedVisual(Red::ItemID& aItemID, Red::TweakDBID aSlotID)
{
    HandleAppearanceChange(aItemID, aSlotID, true);
}

void App::PuppetStateHandler::OnItemUnequipped(Red::ItemID& aItemID, Red::TweakDBID aSlotID)
{
    HandleAppearanceChange(aItemID, aSlotID, false);
}

void App::PuppetStateHandler::OnItemUnequippedComplete(Red::ItemID& aItemID, Red::TweakDBID aSlotID)
{
    HandleAppearanceChange(aItemID, aSlotID, false);
}

void App::PuppetStateHandler::HandleAppearanceChange(Red::ItemID& aItemID, Red::TweakDBID aSlotID, bool aEquipped)
{
    auto puppet = m_puppetWeak.Lock();
    if (puppet)
    {
#ifndef NDEBUG
        auto debugSlotName = Red::ToStringDebug(aSlotID);
        auto debugItemName = Red::ToStringDebug(aItemID.tdbid);
#endif

        if (IsWeaponSlot(aSlotID))
        {
            if (UpdateArmsState(puppet, aItemID, aEquipped))
            {
                RefreshHandsAppearances(puppet);
            }
        }

        if (IsFeetSlot(aSlotID) || HidesFootwear(puppet, aItemID))
        {
            if (UpdateFeetState(puppet))
            {
                RefreshLegsAppearances(puppet);
            }
        }

        if (IsTorsoSlot(aSlotID) && RollsUpSleeves(puppet, aItemID))
        {
            RefreshChestAppearances(puppet);
        }
    }
}

bool App::PuppetStateHandler::UpdateArmsState(const Red::Handle<Red::Entity>& aPuppet, Red::ItemID& aItemID,
                                              bool aEquipped)
{
    auto state = ResolveArmsState(aPuppet, aItemID, aEquipped);
    auto updated = (m_armsState != state);

    m_armsState = state;

    return updated;
}

App::PuppetArmsState App::PuppetStateHandler::ResolveArmsState(const Red::Handle<Red::Entity>& aPuppet,
                                                               Red::ItemID& aItemID, bool aEquipped)
{
    if (aEquipped)
    {
        auto friendlyName = Red::GetFlatValue<Red::CString>({aItemID.tdbid, ".friendlyName"});

        if (friendlyName == MantisBladesName)
            return PuppetArmsState::MantisBlades;

        if (friendlyName == MonowireName)
            return PuppetArmsState::Monowire;

        if (friendlyName == ProjectileLauncher)
            return PuppetArmsState::ProjectileLauncher;
    }

    return PuppetArmsState::BaseArms;
}

bool App::PuppetStateHandler::UpdateFeetState(const Red::Handle<Red::Entity>& aPuppet)
{
    auto state = ResolveFeetState(aPuppet);
    auto updated = (m_feetState != state);

    m_feetState = state;

    return updated;
}

App::PuppetFeetState App::PuppetStateHandler::ResolveFeetState(const Red::Handle<Red::Entity>& aPuppet)
{
    if (IsMale())
        return PuppetFeetState::None;

    auto state = PuppetFeetState::Flat;

    for (const auto& slotID : m_feetSlots)
    {
        auto itemObject = GetItemInSlot(aPuppet, slotID);
        if (itemObject && IsVisible(itemObject))
        {
            if (IsHighHeels(itemObject))
            {
                state = PuppetFeetState::HighHeels;
                break;
            }

            if (IsFlatSole(itemObject))
            {
                state = PuppetFeetState::FlatShoes;
                break;
            }

            state = PuppetFeetState::Lifted;
            break;
        }
    }

    return state;
}

void App::PuppetStateHandler::RefreshChestAppearances(const Red::Handle<Red::Entity>& aPuppet)
{
    for (const auto& slotID : m_torsoDependentSlots)
    {
        auto itemObject = GetItemInSlot(aPuppet, slotID);
        if (itemObject && IsVisible(itemObject) && (IsDynamicAppearance(itemObject) || ReactsToSleeves(itemObject)))
        {
            RefreshItemAppearance(aPuppet, itemObject);
        }
    }
}

void App::PuppetStateHandler::RefreshHandsAppearances(const Red::Handle<Red::Entity>& aPuppet)
{
    for (const auto& slotID : m_handsDependentSlots)
    {
        auto itemObject = GetItemInSlot(aPuppet, slotID);
        if (itemObject && IsVisible(itemObject) && (IsDynamicAppearance(itemObject) || ReactsToArms(itemObject)))
        {
            RefreshItemAppearance(aPuppet, itemObject);
        }
    }
}

void App::PuppetStateHandler::RefreshLegsAppearances(const Red::Handle<Red::Entity>& aPuppet)
{
    for (const auto& slotID : m_feetDependentSlots)
    {
        auto itemObject = GetItemInSlot(aPuppet, slotID);
        if (itemObject && IsVisible(itemObject) && (IsDynamicAppearance(itemObject) || ReactsToFeet(itemObject)))
        {
            RefreshItemAppearance(aPuppet, itemObject);
        }
    }
}

bool App::PuppetStateHandler::IsMale()
{
    if (!m_gender)
    {
        Red::CallVirtual(m_puppetWeak.instance, "GetResolvedGenderName", m_gender);
    }

    return m_gender == MaleGenderName;
}

bool App::PuppetStateHandler::IsTorsoSlot(Red::TweakDBID aSlotID)
{
    return m_torsoSlots.contains(aSlotID);
}

bool App::PuppetStateHandler::IsFeetSlot(Red::TweakDBID aSlotID)
{
    return m_feetSlots.contains(aSlotID);
}

bool App::PuppetStateHandler::IsWeaponSlot(Red::TweakDBID aSlotID)
{
    return aSlotID == WeaponRightSlot || aSlotID == WeaponLeftSlot;
}

bool App::PuppetStateHandler::HidesFootwear(const Red::Handle<Red::Entity>& aPuppet, Red::ItemID& aItemID)
{
    return m_transactionSystem->MatchVisualTagByItemID(aItemID, aPuppet, HideFootwearTag);
}

bool App::PuppetStateHandler::RollsUpSleeves(const Red::Handle<Red::Entity>& aPuppet, Red::ItemID& aItemID)
{
    return m_transactionSystem->MatchVisualTagByItemID(aItemID, aPuppet, HideInnerSleevesTag);
}

bool App::PuppetStateHandler::IsHighHeels(const Red::Handle<Red::ItemObject>& aItemObject)
{
    return m_transactionSystem->MatchVisualTag(aItemObject, HighHeelsTag, false);
}

bool App::PuppetStateHandler::IsFlatSole(const Red::Handle<Red::ItemObject>& aItemObject)
{
    return m_transactionSystem->MatchVisualTag(aItemObject, FlatShoesTag, false)
        || m_transactionSystem->MatchVisualTag(aItemObject, ForceFlatFeetTag, false);
}

Red::Handle<Red::ItemObject> App::PuppetStateHandler::GetItemInSlot(const Red::Handle<Red::Entity>& aPuppet,
                                                                    Red::TweakDBID aSlotID)
{
    auto slotData = m_transactionSystem->FindSlotData(aPuppet,
                                                      [aSlotID](const Red::AttachmentSlotData& aSlotData)
                                                      {
                                                          return aSlotData.slotID == aSlotID;
                                                      });

    if (!slotData)
        return {};

    return slotData->itemObject;
}

bool App::PuppetStateHandler::IsVisible(const Red::Handle<Red::ItemObject>& aItemObject)
{
    Red::CName itemAppearance;
    Raw::ItemObject::GetAppearanceName(aItemObject, itemAppearance);

    return itemAppearance && itemAppearance != EmptyAppearanceName;
}

bool App::PuppetStateHandler::IsDynamicAppearance(const Red::Handle<Red::ItemObject>& aItemObject)
{
    Red::CName itemAppearance;
    Raw::ItemObject::GetAppearanceName(aItemObject, itemAppearance);

    return DynamicAppearanceName::CheckMark(itemAppearance);
}

bool App::PuppetStateHandler::ReactsToSleeves(const Red::Handle<Red::ItemObject>& aItemObject)
{
    auto itemID = Raw::ItemObject::ItemID(aItemObject);
    auto appearanceSuffixes = Red::GetFlatPtr<Red::DynArray<Red::TweakDBID>>({itemID->tdbid, ".appearanceSuffixes"});

    return appearanceSuffixes->Contains(InnerSleevesSuffix);
}

bool App::PuppetStateHandler::ReactsToArms(const Red::Handle<Red::ItemObject>& aItemObject)
{
    auto itemID = Raw::ItemObject::ItemID(aItemObject);
    auto appearanceSuffixes = Red::GetFlatPtr<Red::DynArray<Red::TweakDBID>>({itemID->tdbid, ".appearanceSuffixes"});

    return appearanceSuffixes->Contains(ArmsStateSuffix);
}

bool App::PuppetStateHandler::ReactsToFeet(const Red::Handle<Red::ItemObject>& aItemObject)
{
    auto itemID = Raw::ItemObject::ItemID(aItemObject);
    auto appearanceSuffixes = Red::GetFlatPtr<Red::DynArray<Red::TweakDBID>>({itemID->tdbid, ".appearanceSuffixes"});

    return appearanceSuffixes->Contains(FeetStateSuffix) || appearanceSuffixes->Contains(LegsStateSuffix);
}

void App::PuppetStateHandler::RefreshItemAppearance(const Red::Handle<Red::Entity>& aPuppet,
                                                    const Red::Handle<Red::ItemObject>& aItemObject)
{
    m_transactionSystem->ResetItemAppearance(aPuppet, Raw::ItemObject::ItemID(aItemObject));
}

App::PuppetArmsState App::PuppetStateHandler::GetArmsState()
{
    return m_armsState;
}

App::PuppetFeetState App::PuppetStateHandler::GetFeetState()
{
    return m_feetState;
}
