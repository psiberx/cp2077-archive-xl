#include "Handler.hpp"
#include "App/Extensions/AttachmentSlots/Module.hpp"
#include "App/Extensions/GarmentOverride/Module.hpp"
#include "Red/ItemObject.hpp"
#include "Red/TweakDB.hpp"

namespace
{
constexpr auto TorsoSlot = Red::TweakDBID("AttachmentSlots.Torso");
constexpr auto ChestSlot = Red::TweakDBID("AttachmentSlots.Chest");
constexpr auto LegsSlot = Red::TweakDBID("AttachmentSlots.Legs");
constexpr auto FeetSlot = Red::TweakDBID("AttachmentSlots.Feet");

constexpr auto HideInnerSleevesTag = Red::CName("hide_T1part");
constexpr auto HideFootwearTag = Red::CName("hide_S1");
constexpr auto HighHeelsTag = Red::CName("HighHeels");
constexpr auto FlatShoesTag = Red::CName("FlatShoes");

constexpr auto InnerSleevesSuffix = Red::TweakDBID("itemsFactoryAppearanceSuffix.Partial");

constexpr auto EmptyAppearanceName = Red::CName("empty_appearance_default");
constexpr auto MaleGenderName = Red::CName("Male");
}

App::PuppetStateHandler::PuppetStateHandler(Red::Entity* aPuppet)
    : m_puppetWeak(Red::ToWeakHandle(aPuppet))
    , m_transactionSystem(Red::GetGameSystem<Red::ITransactionSystem>())
    , m_torsoSlots(AttachmentSlotsModule::GetRelatedSlots(TorsoSlot))
    , m_chestSlots(AttachmentSlotsModule::GetRelatedSlots(ChestSlot))
    , m_legsSlots(AttachmentSlotsModule::GetRelatedSlots(LegsSlot))
    , m_feetSlots(AttachmentSlotsModule::GetRelatedSlots(FeetSlot))
    , m_feetState(PuppetFeetState::None)
{
    UpdateFeetState(m_puppetWeak.Lock());
}

void App::PuppetStateHandler::OnItemEquipped(Red::ItemID& aItemID, Red::TweakDBID aSlotID)
{
    HandleAppearanceChange(aItemID, aSlotID);
}

void App::PuppetStateHandler::OnItemEquippedVisual(Red::ItemID& aItemID, Red::TweakDBID aSlotID)
{
    HandleAppearanceChange(aItemID, aSlotID);
}

void App::PuppetStateHandler::OnItemUnequipped(Red::ItemID& aItemID, Red::TweakDBID aSlotID)
{
    HandleAppearanceChange(aItemID, aSlotID);
}

void App::PuppetStateHandler::OnItemUnequippedComplete(Red::ItemID& aItemID, Red::TweakDBID aSlotID)
{
    HandleAppearanceChange(aItemID, aSlotID);
}

void App::PuppetStateHandler::HandleAppearanceChange(Red::ItemID& aItemID, Red::TweakDBID aSlotID)
{
#ifndef NDEBUG
    auto debugSlotName = Red::ToStringDebug(aSlotID);
    auto debugItemName = Red::ToStringDebug(aItemID.tdbid);
#endif

    auto puppet = m_puppetWeak.Lock();
    if (puppet)
    {
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

void App::PuppetStateHandler::RefreshChestAppearances(const Red::Handle<Red::Entity>& aPuppet)
{
    for (const auto& slotID : m_chestSlots)
    {
        auto itemObject = GetItemInSlot(aPuppet, slotID);
        if (itemObject && HasVisibleAppearance(itemObject) && ReactsToSleeves(itemObject))
        {
            RefreshItemAppearance(aPuppet, itemObject);
        }
    }
}

void App::PuppetStateHandler::RefreshLegsAppearances(const Red::Handle<Red::Entity>& aPuppet)
{
    for (const auto& slotID : m_legsSlots)
    {
        auto itemObject = GetItemInSlot(aPuppet, slotID);
        if (itemObject && HasVisibleAppearance(itemObject))
        {
            RefreshItemAppearance(aPuppet, itemObject);
        }
    }
}

App::PuppetFeetState App::PuppetStateHandler::ResolveFeetState(const Red::Handle<Red::Entity>& aPuppet)
{
    if (IsMale())
        return PuppetFeetState::None;

    auto state = PuppetFeetState::Flat;

    for (const auto& slotID : m_feetSlots)
    {
        auto itemObject = GetItemInSlot(aPuppet, slotID);
        if (itemObject && HasVisibleAppearance(itemObject))
        {
            if (HasHighHeels(itemObject))
            {
                state = PuppetFeetState::HighHeels;
                break;
            }

            if (HasFlatSole(itemObject))
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

bool App::PuppetStateHandler::UpdateFeetState(const Red::Handle<Red::Entity>& aPuppet)
{
    auto state = ResolveFeetState(aPuppet);
    auto updated = (m_feetState != state);

    m_feetState = state;

    return updated;
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

bool App::PuppetStateHandler::HidesFootwear(const Red::Handle<Red::Entity>& aPuppet, Red::ItemID& aItemID)
{
    return m_transactionSystem->MatchVisualTagByItemID(aItemID, aPuppet, HideFootwearTag);
}

bool App::PuppetStateHandler::RollsUpSleeves(const Red::Handle<Red::Entity>& aPuppet, Red::ItemID& aItemID)
{
    return m_transactionSystem->MatchVisualTagByItemID(aItemID, aPuppet, HideInnerSleevesTag);
}

bool App::PuppetStateHandler::HasHighHeels(const Red::Handle<Red::ItemObject>& aItemObject)
{
    return m_transactionSystem->MatchVisualTag(aItemObject, HighHeelsTag, false);
}

bool App::PuppetStateHandler::HasFlatSole(const Red::Handle<Red::ItemObject>& aItemObject)
{
    return m_transactionSystem->MatchVisualTag(aItemObject, FlatShoesTag, false)
        || m_transactionSystem->MatchVisualTag(aItemObject, GarmentOverrideModule::ForceFlatFeetTag, false);
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

bool App::PuppetStateHandler::HasVisibleAppearance(const Red::Handle<Red::ItemObject>& aItemObject)
{
    Red::CName itemAppearance;
    Raw::ItemObject::GetAppearanceName(aItemObject, itemAppearance);

    return itemAppearance && itemAppearance != EmptyAppearanceName;
}

bool App::PuppetStateHandler::ReactsToSleeves(const Red::Handle<Red::ItemObject>& aItemObject)
{
    auto itemID = Raw::ItemObject::ItemID(aItemObject);
    auto appearanceSuffixes = Red::GetFlat<Red::DynArray<Red::TweakDBID>>({itemID->tdbid, ".appearanceSuffixes"});

    return appearanceSuffixes->Contains(InnerSleevesSuffix);
}

void App::PuppetStateHandler::RefreshItemAppearance(const Red::Handle<Red::Entity>& aPuppet,
                                                    const Red::Handle<Red::ItemObject>& aItemObject)
{
    m_transactionSystem->ResetItemAppearance(aPuppet, Raw::ItemObject::ItemID(aItemObject));
}

App::PuppetFeetState App::PuppetStateHandler::GetFeetState()
{
    return m_feetState;
}
