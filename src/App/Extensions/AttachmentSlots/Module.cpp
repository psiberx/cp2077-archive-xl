#include "Module.hpp"
#include "App/Extensions/GarmentOverride/Module.hpp"
#include "Red/Entity.hpp"
#include "Red/ItemObject.hpp"
#include "Red/TweakDB.hpp"

namespace
{
constexpr auto ModuleName = "AttachmentSlots";

constexpr auto HeadSlot = Red::TweakDBID("AttachmentSlots.Head");
constexpr auto FaceSlot = Red::TweakDBID("AttachmentSlots.Eyes");
constexpr auto TorsoSlot = Red::TweakDBID("AttachmentSlots.Torso");
constexpr auto ChestSlot = Red::TweakDBID("AttachmentSlots.Chest");
constexpr auto LegsSlot = Red::TweakDBID("AttachmentSlots.Legs");
constexpr auto FeetSlot = Red::TweakDBID("AttachmentSlots.Feet");
constexpr auto TPPAffectedSlots = {HeadSlot, FaceSlot};

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
    HookAfter<Raw::AttachmentSlots::InitializeSlots>(&OnInitializeSlots).OrThrow();
    Hook<Raw::AttachmentSlots::IsSlotSpawning>(&OnSlotSpawningCheck).OrThrow();
    HookAfter<Raw::TPPRepresentationComponent::OnAttach>(&OnAttachTPP).OrThrow();
    HookAfter<Raw::TPPRepresentationComponent::IsAffectedSlot>(&OnSlotCheckTPP).OrThrow();
    HookAfter<Raw::CharacterCustomizationHairstyleController::CheckState>(&OnCheckHairState).OrThrow();
    //HookAfter<Raw::CharacterCustomizationGenitalsController::CheckState>(&OnCheckBodyState).OrThrow();
    HookAfter<Raw::CharacterCustomizationFeetController::CheckState>(&OnCheckFeetState).OrThrow();
    HookWrap<Raw::AppearanceChanger::GetSuffixValue>(&OnGetSuffixValue).OrThrow();

    {
        std::unique_lock _(s_slotsMutex);
        s_dependentSlots[TorsoSlot].insert(ChestSlot);
        s_dependentSlots[FeetSlot].insert(LegsSlot);
    }

    return true;
}

bool App::AttachmentSlotsModule::Unload()
{
    Unhook<Raw::AttachmentSlots::InitializeSlots>();
    Unhook<Raw::AttachmentSlots::IsSlotSpawning>();
    Unhook<Raw::TPPRepresentationComponent::OnAttach>();
    Unhook<Raw::TPPRepresentationComponent::IsAffectedSlot>();
    Unhook<Raw::CharacterCustomizationHairstyleController::CheckState>();
    // Unhook<Raw::CharacterCustomizationGenitalsController::CheckState>();
    Unhook<Raw::CharacterCustomizationFeetController::CheckState>();
    Unhook<Raw::AppearanceChanger::GetSuffixValue>();

    return true;
}

void App::AttachmentSlotsModule::OnInitializeSlots(Red::game::AttachmentSlots* aComponent,
                                                   Red::DynArray<Red::TweakDBID>& aSlotIDs)
{
    if (aSlotIDs.size > 0)
    {
// #ifndef NDEBUG
//         const auto entity = Raw::IComponent::Owner::Ptr(aComponent);
//         const auto entityID = Raw::Entity::EntityID::Ptr(entity);
//         LogDebug("|{}| [event=InitializeSlots ent={}]", ModuleName, entityID->hash);
// #endif

        std::unique_lock _(s_slotsMutex);
        auto tweakDB = Red::TweakDB::Get();

        for (auto& slotID : aSlotIDs)
        {
            auto parentSlotID = Red::GetFlatValue<Red::TweakDBID>({slotID, ".parentSlot"});
            if (parentSlotID)
            {
#ifndef NDEBUG
                auto debugSlotName = Red::ToStringDebug(slotID);
                auto debugParentName = Red::ToStringDebug(parentSlotID);
#endif
                s_extraSlots[parentSlotID].insert(slotID);
                s_baseSlots[slotID] = parentSlotID;
            }

            auto dependencySlotIDs = Red::GetFlatPtr<Red::DynArray<Red::TweakDBID>>({slotID, ".dependencySlots"});
            if (dependencySlotIDs)
            {
                for (const auto dependencySlotID : *dependencySlotIDs)
                {
#ifndef NDEBUG
                    auto debugSlotName = Red::ToStringDebug(slotID);
                    auto debugDependencyName = Red::ToStringDebug(dependencySlotID);
#endif
                    s_dependentSlots[dependencySlotID].insert(slotID);
                }
            }
        }
    }
}

bool App::AttachmentSlotsModule::OnSlotSpawningCheck(Red::game::AttachmentSlots* aComponent, Red::TweakDBID aSlotID)
{
#ifndef NDEBUG
    auto debugSlotName = Red::ToStringDebug(aSlotID);
#endif

    bool result = Raw::AttachmentSlots::IsSlotSpawning(aComponent, aSlotID);

    if (!result)
    {
// #ifndef NDEBUG
//         LogDebug("|{}| [event=SlotSpawningCheck]", ModuleName);
// #endif

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
#ifndef NDEBUG
    LogDebug("|{}| [event=AttachTPP]", ModuleName);
#endif

    auto transactionSystem = Red::GetGameSystem<Red::ITransactionSystem>();

    std::shared_lock _(s_slotsMutex);

    for (const auto slotID : TPPAffectedSlots)
    {
        const auto& subSlots = s_extraSlots.find(slotID);
        if (subSlots != s_extraSlots.end())
        {
            for (const auto& subSlotID : subSlots->second)
            {
                if (!aComponent->affectedAppearanceSlots.Contains(subSlotID))
                {
                    aComponent->affectedAppearanceSlots.PushBack(subSlotID);
                }

                if (aComponent->owner)
                {
#ifndef NDEBUG
                    auto debugSlotName = Red::ToStringDebug(subSlotID);
#endif

                    auto slotData = transactionSystem->FindSlotData(aComponent->owner,
                                                                   [subSlotID](const Red::AttachmentSlotData& aSlotData)
                                                                   {
                                                                       return aSlotData.slotID == subSlotID;
                                                                   });
                    if (slotData && slotData->itemObject)
                    {
                        const auto& itemID = Raw::ItemObject::ItemID::Ref(slotData->itemObject).tdbid;
#ifndef NDEBUG
                        auto debugItemName = Red::ToStringDebug(itemID);
#endif
                        Raw::TPPRepresentationComponent::RegisterAffectedItem(aComponent, itemID, slotData->itemObject);
                    }
                }
            }
        }
    }
}

void App::AttachmentSlotsModule::OnSlotCheckTPP(bool& aAffected, Red::TweakDBID aSlotID)
{
    if (!aAffected)
    {
#ifndef NDEBUG
        auto debugSlotName = Red::ToStringDebug(aSlotID);
#endif

        std::shared_lock _(s_slotsMutex);
        const auto& baseSlot = s_baseSlots.find(aSlotID);
        if (baseSlot != s_baseSlots.end())
        {
            aAffected = (baseSlot.value() == HeadSlot || baseSlot.value() == FaceSlot);
        }
    }
}

void App::AttachmentSlotsModule::OnCheckHairState(Red::game::ui::CharacterCustomizationHairstyleController* aComponent,
                                                  Red::CharacterBodyPartState& aHairState)
{
    if (aHairState == Red::CharacterBodyPartState::Hidden)
    {
        auto owner = Red::ToHandle(aComponent->owner);
        if (IsVisualTagActive(owner, HeadSlot, GarmentOverrideModule::ForceHairTag))
        {
            aHairState = Red::CharacterBodyPartState::Visible;
        }
    }
}

void App::AttachmentSlotsModule::OnCheckBodyState(Red::game::ui::CharacterCustomizationGenitalsController* aComponent,
                                                  Red::CharacterBodyPartState& aUpperState,
                                                  Red::CharacterBodyPartState& aBottomState)
{
}

void App::AttachmentSlotsModule::OnCheckFeetState(Red::game::ui::CharacterCustomizationFeetController* aComponent,
                                                  Red::CharacterBodyPartState& aLiftedState,
                                                  Red::CharacterBodyPartState& aFlatState)
{
    if (aLiftedState != Red::CharacterBodyPartState::Visible)
    {
        auto transactionSystem = Red::GetGameSystem<Red::ITransactionSystem>();
        if (transactionSystem->IsSlotSpawning(aComponent->owner, FeetSlot))
        {
            aLiftedState = Red::CharacterBodyPartState::Visible;
            aFlatState = Red::CharacterBodyPartState::Hidden;
        }
    }

    if (aLiftedState == Red::CharacterBodyPartState::Visible)
    {
        auto owner = Red::ToHandle(aComponent->owner);
        if (IsVisualTagActive(owner, FeetSlot, GarmentOverrideModule::ForceFlatFeetTag))
        {
            aLiftedState = Red::CharacterBodyPartState::Hidden;
            aFlatState = Red::CharacterBodyPartState::Visible;
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

        if (slotData->itemObject)
        {
            Red::CName itemAppearance;
            Raw::ItemObject::GetAppearanceName(slotData->itemObject, itemAppearance);

            return itemAppearance && itemAppearance != EmptyAppearanceName &&
                   aTransactionSystem->MatchVisualTag(slotData->itemObject, aVisualTag, false);
        }

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
    }

    return false;
}

Core::Set<Red::TweakDBID> App::AttachmentSlotsModule::GetExtraSlots(Red::TweakDBID aBaseSlotID)
{
    std::shared_lock _(s_slotsMutex);
    const auto& extraSlotsIt = s_extraSlots.find(aBaseSlotID);

    if (extraSlotsIt == s_extraSlots.end())
        return {};

    return extraSlotsIt.value();
}

Core::Set<Red::TweakDBID> App::AttachmentSlotsModule::GetRelatedSlots(Red::TweakDBID aBaseSlotID)
{
    Core::Set<Red::TweakDBID> result;
    result.insert(aBaseSlotID);

    {
        std::shared_lock _(s_slotsMutex);
        const auto& extraSlotsIt = s_extraSlots.find(aBaseSlotID);
        if (extraSlotsIt != s_extraSlots.end())
        {
            const auto& subSlots = extraSlotsIt.value();
            result.insert(subSlots.begin(), subSlots.end());
        }
    }

    return result;
}

Core::Set<Red::TweakDBID> App::AttachmentSlotsModule::GetDependentSlots(Red::TweakDBID aBaseSlotID)
{
    std::shared_lock _(s_slotsMutex);
    const auto& dependentSlotsIt = s_dependentSlots.find(aBaseSlotID);

    if (dependentSlotsIt == s_dependentSlots.end())
        return {};

    return dependentSlotsIt.value();
}
