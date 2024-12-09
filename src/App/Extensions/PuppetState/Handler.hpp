#pragma once

namespace App
{
enum class PuppetArmsState
{
    BaseArms,
    MantisBlades,
    Monowire,
    ProjectileLauncher,
};

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

    PuppetArmsState GetArmsState();
    PuppetFeetState GetFeetState();

private:
    using SlotState = std::pair<Red::Handle<Red::ItemObject>, bool>;

    // void OnItemEquipped(const Red::ItemID& aItemID, Red::TweakDBID aSlotID) override;
    // void OnItemEquippedVisual(const Red::ItemID& aItemID, Red::TweakDBID aSlotID) override;
    void OnItemEquippedComplete(const Red::ItemID& aItemID, Red::TweakDBID aSlotID) override;
    // void OnItemUnequipped(const Red::ItemID& aItemID, Red::TweakDBID aSlotID) override;
    void OnItemUnequippedComplete(const Red::ItemID& aItemID, Red::TweakDBID aSlotID) override;

    // void HandleAppearanceChange(const Red::ItemID& aItemID, Red::TweakDBID aSlotID, bool aEquipped);
    void FinalizeAppearanceChange(const Red::ItemID& aItemID, Red::TweakDBID aSlotID, bool aEquipped);

    bool UpdateArmsState(const Red::Handle<Red::Entity>& aPuppet, const Red::ItemID& aItemID, bool aEquipped);
    PuppetArmsState ResolveArmsState(const Red::Handle<Red::Entity>& aPuppet, const Red::ItemID& aItemID,
                                     bool aEquipped);

    bool UpdateFeetState(const Red::Handle<Red::Entity>& aPuppet);
    PuppetFeetState ResolveFeetState(const Red::Handle<Red::Entity>& aPuppet);

    void RefreshCameraDependentAppearances(const Red::Handle<Red::Entity>& aPuppet);
    void RefreshSleevesDependentAppearances(const Red::Handle<Red::Entity>& aPuppet);
    void RefreshArmsDependentAppearances(const Red::Handle<Red::Entity>& aPuppet);
    void RefreshFeetDependentAppearances(const Red::Handle<Red::Entity>& aPuppet);

    bool IsMale();

    bool IsTorsoSlot(Red::TweakDBID aSlotID);
    bool IsFeetSlot(Red::TweakDBID aSlotID);
    static bool IsWeaponSlot(Red::TweakDBID aSlotID);
    static bool IsTppHeadSlot(Red::TweakDBID aSlotID);

    SlotState GetItemInSlot(const Red::Handle<Red::Entity>& aPuppet, Red::TweakDBID aSlotID);

    bool HidesFootwear(const Red::Handle<Red::Entity>& aPuppet, const Red::ItemID& aItemID);
    bool RollsUpSleeves(const Red::Handle<Red::Entity>& aPuppet, const Red::ItemID& aItemID);

    bool IsHighHeels(const Red::Handle<Red::ItemObject>& aItemObject);
    bool IsFlatSole(const Red::Handle<Red::ItemObject>& aItemObject);

    static bool IsVisible(const Red::Handle<Red::ItemObject>& aItemObject);
    static bool IsDynamicAppearance(const Red::Handle<Red::ItemObject>& aItemObject);
    static bool ReactsToSuffix(const Red::Handle<Red::ItemObject>& aItemObject,
                               std::initializer_list<Red::TweakDBID> aSuffixIDs);

    void RefreshDependentAppearances(const Red::Handle<Red::Entity>& aPuppet,
                                     const Core::Set<Red::TweakDBID>& aSlotIDs,
                                     std::initializer_list<Red::TweakDBID> aSuffixIDs);
    void RefreshItemAppearance(const Red::Handle<Red::Entity>& aPuppet, const Red::Handle<Red::ItemObject>& aItemObject);
    void RefreshItemAppearance(const Red::Handle<Red::Entity>& aPuppet, const Red::ItemID& aItemID);

    void AddPendingRefresh(Red::TweakDBID aSlotID);
    bool PullPendingRefresh(Red::TweakDBID aSlotID);

    Red::WeakHandle<Red::Entity> m_puppetWeak;
    Red::ITransactionSystem* m_transactionSystem;
    Core::Set<Red::TweakDBID> m_torsoSlots;
    Core::Set<Red::TweakDBID> m_feetSlots;
    Core::Set<Red::TweakDBID> m_headDependentSlots;
    Core::Set<Red::TweakDBID> m_torsoDependentSlots;
    Core::Set<Red::TweakDBID> m_handsDependentSlots;
    Core::Set<Red::TweakDBID> m_feetDependentSlots;
    Core::Set<Red::TweakDBID> m_pendingRefreshes;
    PuppetArmsState m_armsState;
    PuppetFeetState m_feetState;
    Red::CName m_gender;

    RTTI_IMPL_TYPEINFO(App::PuppetStateHandler);
    RTTI_IMPL_ALLOCATOR(Red::Memory::DefaultAllocator);
};
}

RTTI_DEFINE_ENUM(App::PuppetArmsState);
RTTI_DEFINE_ENUM(App::PuppetFeetState);
RTTI_DEFINE_CLASS(App::PuppetStateHandler);
