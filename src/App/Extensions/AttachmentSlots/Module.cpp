#include "Module.hpp"
#include "Red/Entity.hpp"
#include "Red/ItemObject.hpp"
#include "Red/TweakDB.hpp"

namespace
{
constexpr auto ModuleName = "AttachmentSlots";

constexpr auto ParentSlotFlat = ".parentSlot";

constexpr auto HeadSlot = Red::TweakDBID("AttachmentSlots.Head");
constexpr auto FaceSlot = Red::TweakDBID("AttachmentSlots.Eyes");
constexpr auto TorsoSlot = Red::TweakDBID("AttachmentSlots.Torso");
constexpr auto FeetSlot = Red::TweakDBID("AttachmentSlots.Feet");
constexpr auto TPPAffectedSlots = {HeadSlot, FaceSlot};

constexpr auto ForceHairTag = Red::CName("force_Hair");
constexpr auto ForceFlatFeetTag = Red::CName("force_FlatFeet");

constexpr auto InnerSleevesSuffix = Red::TweakDBID("itemsFactoryAppearanceSuffix.Partial");
constexpr auto HideInnerSleevesTag = Red::CName("hide_T1part");
constexpr auto HideInnerSleevesSuffixValue = "Part";
constexpr auto ShowInnerSleevesSuffixValue = "Full";

constexpr auto EmptyAppearanceName = Red::CName("empty_appearance_default");
}

std::string_view App::AttachmentSlotsModule::GetName()
{
    return ModuleName;
}

bool App::AttachmentSlotsModule::Load()
{
    if (!HookAfter<Raw::AttachmentSlots::InitializeSlots>(&OnInitializeSlots))
        throw std::runtime_error("Failed to hook [AttachmentSlots::InitializeSlots].");

    if (!Hook<Raw::AttachmentSlots::IsSlotSpawning>(&OnSlotSpawningCheck))
        throw std::runtime_error("Failed to hook [AttachmentSlots::IsSlotSpawning].");

    if (!HookBefore<Raw::TPPRepresentationComponent::OnAttach>(&OnAttachTPP))
        throw std::runtime_error("Failed to hook [TPPRepresentationComponent::OnInitialize].");

    if (!HookWrap<Raw::TPPRepresentationComponent::OnItemEquipped>(&OnItemChangeTPP))
        throw std::runtime_error("Failed to hook [TPPRepresentationComponent::OnItemEquipped].");

    if (!HookWrap<Raw::TPPRepresentationComponent::OnItemUnequipped>(&OnItemChangeTPP))
        throw std::runtime_error("Failed to hook [TPPRepresentationComponent::OnItemUnequipped].");

    if (!HookAfter<Raw::CharacterCustomizationHairstyleController::CheckState>(&OnCheckHairState))
        throw std::runtime_error("Failed to hook [CharacterCustomizationHairstyleController::CheckState].");

    if (!HookAfter<Raw::CharacterCustomizationFeetController::CheckState>(&OnCheckFeetState))
        throw std::runtime_error("Failed to hook [CharacterCustomizationFeetController::CheckState].");

    if (!HookWrap<Raw::AppearanceChanger::GetSuffixValue>(&OnGetSuffixValue))
        throw std::runtime_error("Failed to hook [AppearanceChanger::GetSuffixValue].");

    return true;
}

bool App::AttachmentSlotsModule::Unload()
{
    Unhook<Raw::AttachmentSlots::InitializeSlots>();
    Unhook<Raw::AttachmentSlots::IsSlotSpawning>();
    Unhook<Raw::TPPRepresentationComponent::OnAttach>();
    Unhook<Raw::TPPRepresentationComponent::OnItemEquipped>();
    Unhook<Raw::TPPRepresentationComponent::OnItemUnequipped>();
    Unhook<Raw::CharacterCustomizationHairstyleController::CheckState>();
    Unhook<Raw::CharacterCustomizationFeetController::CheckState>();
    Unhook<Raw::AppearanceChanger::GetSuffixValue>();

    return true;
}

void App::AttachmentSlotsModule::OnInitializeSlots(Red::game::AttachmentSlots*, Red::DynArray<Red::TweakDBID>& aSlotIDs)
{
    if (aSlotIDs.size > 0)
    {
        std::unique_lock _(s_slotsMutex);
        auto tweakDB = Red::TweakDB::Get();

        for (auto& slotID : aSlotIDs)
        {
            const auto parentSlotFlat = tweakDB->GetFlatValue({slotID, ParentSlotFlat});
            if (parentSlotFlat)
            {
                const auto parentSlotID = *parentSlotFlat->GetValue<Red::TweakDBID>();
                if (parentSlotID)
                {
                    s_extraSlots[parentSlotID].insert(slotID);
                    s_baseSlots[slotID] = parentSlotID;
                }
            }
        }
    }
}

bool App::AttachmentSlotsModule::OnSlotSpawningCheck(Red::game::AttachmentSlots* aComponent, Red::TweakDBID aSlotID)
{
    bool result = Raw::AttachmentSlots::IsSlotSpawning(aComponent, aSlotID);

    if (!result)
    {
        std::shared_lock _(s_slotsMutex);
        const auto& subSlots = s_extraSlots.find(aSlotID);
        if (subSlots != s_extraSlots.end())
        {
            for (const auto& subSlotID : subSlots->second)
            {
                result = !Raw::AttachmentSlots::IsSlotEmpty(aComponent, subSlotID)
                         || Raw::AttachmentSlots::IsSlotSpawning(aComponent, subSlotID);

                if (result)
                    break;
            }
        }
    }

    return result;
}

void App::AttachmentSlotsModule::OnAttachTPP(Red::game::TPPRepresentationComponent* aComponent, uintptr_t)
{
    std::shared_lock _(s_slotsMutex);

    for (const auto slotID : TPPAffectedSlots)
    {
        const auto& subSlots = s_extraSlots.find(slotID);
        if (subSlots != s_extraSlots.end())
        {
            for (const auto& subSlotID : subSlots->second)
            {
                aComponent->affectedAppearanceSlots.PushBack(subSlotID);
            }
        }
    }
}

void App::AttachmentSlotsModule::OnItemChangeTPP(Raw::TPPRepresentationComponent::SlotListenerCallback aCallback,
                                                 Red::game::TPPRepresentationComponent* aComponent,
                                                 Red::TweakDBID aItemID, Red::TweakDBID aSlotID)
{
    {
        std::shared_lock _(s_slotsMutex);
        const auto& baseSlot = s_baseSlots.find(aSlotID);
        if (baseSlot != s_baseSlots.end())
        {
            aSlotID = baseSlot.value();
        }
    }

    aCallback(aComponent, aItemID, aSlotID);
}

void App::AttachmentSlotsModule::OnCheckHairState(Red::game::ui::CharacterCustomizationHairstyleController* aComponent,
                                                  Red::CharacterHairState& aHairState)
{
    if (aHairState == Red::CharacterHairState::Hidden)
    {
        auto owner = Red::ToHandle(Raw::IComponent::Owner::Get(aComponent));
        if (IsVisualTagActive(owner, HeadSlot, ForceHairTag))
        {
            aHairState = Red::CharacterHairState::Visible;
        }
    }
}

void App::AttachmentSlotsModule::OnCheckFeetState(Red::game::ui::CharacterCustomizationFeetController* aComponent,
                                                  Red::CharacterFeetState& aLiftedState,
                                                  Red::CharacterFeetState& aFlatState)
{
    if (aLiftedState != Red::CharacterFeetState::Lifted)
    {
        auto transactionSystem = Red::GetGameSystem<Red::ITransactionSystem>();
        auto owner = Raw::IComponent::Owner(aComponent);

        if (transactionSystem->IsSlotSpawning(owner, FeetSlot))
        {
            aLiftedState = Red::CharacterFeetState::Lifted;
            aFlatState = Red::CharacterFeetState::Flat;
        }
    }

    if (aLiftedState == Red::CharacterFeetState::Lifted)
    {
        auto owner = Red::ToHandle(Raw::IComponent::Owner::Get(aComponent));
        if (IsVisualTagActive(owner, FeetSlot, ForceFlatFeetTag))
        {
            aLiftedState = Red::CharacterFeetState::Flat;
            aFlatState = Red::CharacterFeetState::Lifted;
        }
    }
}

bool App::AttachmentSlotsModule::OnGetSuffixValue(Raw::AppearanceChanger::GetSuffixValuePtr aOriginalFunc,
                                                  Red::ItemID aItemID, uint64_t a2,
                                                  Red::Handle<Red::GameObject>& aOwner,
                                                  Red::TweakDBID aSuffixRecordID,
                                                  Red::CString& aResult)
{
    if (aOwner && a2 && aSuffixRecordID == InnerSleevesSuffix)
    {
        bool hideSleeves = false;

        if (!s_skipVisualTagCheck)
        {
            s_skipVisualTagCheck = true;
            hideSleeves = IsVisualTagActive(aOwner, TorsoSlot, HideInnerSleevesTag);
            s_skipVisualTagCheck = false;
        }

        aResult = hideSleeves ? HideInnerSleevesSuffixValue : ShowInnerSleevesSuffixValue;
        return true;
    }

    return aOriginalFunc(aItemID, a2, aOwner, aSuffixRecordID, aResult);
}

bool App::AttachmentSlotsModule::IsVisualTagActive(Red::Handle<Red::Entity>& aOwner,
                                                   Red::TweakDBID aBaseSlotID, Red::CName aVisualTag)
{
    auto transactionSystem = Red::GetGameSystem<Red::ITransactionSystem>();

    if (IsVisualTagActive(transactionSystem, aOwner, aBaseSlotID, aVisualTag))
    {
        return true;
    }
    else
    {
        std::shared_lock _(s_slotsMutex);
        const auto& subSlots = s_extraSlots.find(aBaseSlotID);
        if (subSlots != s_extraSlots.end())
        {
            for (const auto& subSlotID : subSlots->second)
            {
                if (IsVisualTagActive(transactionSystem, aOwner, subSlotID, aVisualTag))
                {
                    return true;
                }
            }
        }
    }

    return false;
}

bool App::AttachmentSlotsModule::IsVisualTagActive(Red::ITransactionSystem* aTransactionSystem,
                                                   Red::Handle<Red::Entity>& aOwner,
                                                   Red::TweakDBID aSlotID, Red::CName aVisualTag)
{
    auto slotData = aTransactionSystem->FindSlotData(aOwner,
                                                     [aSlotID](const Red::AttachmentSlotData& aSlotData)
                                                     {
                                                         return aSlotData.slotID == aSlotID;
                                                     });
    if (slotData)
    {
#ifndef NDEBUG
        auto debugSlotName = Red::ToStringDebug(aSlotID);
#endif

        if (slotData->spawningItemID)
        {
#ifndef NDEBUG
            auto debugItemName = Red::ToStringDebug(slotData->spawningItemID.tdbid);
#endif

            return aTransactionSystem->MatchVisualTagByItemID(slotData->spawningItemID, aOwner, aVisualTag);
        }

        if (slotData->appearanceItemID)
        {
#ifndef NDEBUG
            auto debugItemName = Red::ToStringDebug(slotData->appearanceItemID.tdbid);
#endif

            return aTransactionSystem->MatchVisualTagByItemID(slotData->appearanceItemID, aOwner, aVisualTag);
        }

        if (slotData->itemObject)
        {
            Red::CName itemAppearance;
            Raw::ItemObject::GetAppearanceName(slotData->itemObject, itemAppearance);

            return itemAppearance && itemAppearance != EmptyAppearanceName &&
                   aTransactionSystem->MatchVisualTag(slotData->itemObject, aVisualTag, false);
        }
    }

    return false;
}
