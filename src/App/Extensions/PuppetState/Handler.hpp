#pragma once

namespace App
{
enum class PuppetFeetState
{
    None,
    Flat,
    Lifted,
    HighHeels,
    FlatShoes,
};

class PuppetStateHandler : public Red::AttachmentSlotsListener
{
public:
    PuppetStateHandler() = default;
    explicit PuppetStateHandler(Red::Entity* aPuppet);

    PuppetFeetState GetFeetState();

private:
    void OnItemEquipped(Red::ItemID& aItemID, Red::TweakDBID aSlotID) override;
    void OnItemEquippedVisual(Red::ItemID& aItemID, Red::TweakDBID aSlotID) override;
    void OnItemUnequipped(Red::ItemID& aItemID, Red::TweakDBID aSlotID) override;
    void OnItemUnequippedComplete(Red::ItemID& aItemID, Red::TweakDBID aSlotID) override;

    void HandleAppearanceChange(Red::ItemID& aItemID, Red::TweakDBID aSlotID);
    void RefreshChestAppearances(const Red::Handle<Red::Entity>& aPuppet);
    void RefreshLegsAppearances(const Red::Handle<Red::Entity>& aPuppet);
    PuppetFeetState ResolveFeetState(const Red::Handle<Red::Entity>& aPuppet);
    bool UpdateFeetState(const Red::Handle<Red::Entity>& aPuppet);

    bool IsMale();
    bool IsTorsoSlot(Red::TweakDBID aSlotID);
    bool IsFeetSlot(Red::TweakDBID aSlotID);
    bool HidesFootwear(const Red::Handle<Red::Entity>& aPuppet, Red::ItemID& aItemID);
    bool RollsUpSleeves(const Red::Handle<Red::Entity>& aPuppet, Red::ItemID& aItemID);
    bool HasHighHeels(const Red::Handle<Red::ItemObject>& aItemObject);
    bool HasFlatSole(const Red::Handle<Red::ItemObject>& aItemObject);
    Red::Handle<Red::ItemObject> GetItemInSlot(const Red::Handle<Red::Entity>& aPuppet, Red::TweakDBID aSlotID);
    static bool HasVisibleAppearance(const Red::Handle<Red::ItemObject>& aItemObject);
    static bool ReactsToSleeves(const Red::Handle<Red::ItemObject>& aItemObject);
    void RefreshItemAppearance(const Red::Handle<Red::Entity>& aPuppet,
                               const Red::Handle<Red::ItemObject>& aItemObject);

    Red::WeakHandle<Red::Entity> m_puppetWeak;
    Red::ITransactionSystem* m_transactionSystem;
    Core::Set<Red::TweakDBID> m_torsoSlots;
    Core::Set<Red::TweakDBID> m_chestSlots;
    Core::Set<Red::TweakDBID> m_legsSlots;
    Core::Set<Red::TweakDBID> m_feetSlots;
    PuppetFeetState m_feetState;
    Red::CName m_gender;

    RTTI_IMPL_TYPEINFO(App::PuppetStateHandler);
    RTTI_IMPL_ALLOCATOR();
};
}

RTTI_DEFINE_ENUM(App::PuppetFeetState);
RTTI_DEFINE_CLASS(App::PuppetStateHandler);
