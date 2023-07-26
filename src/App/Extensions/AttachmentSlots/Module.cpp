#include "Module.hpp"
#include "Red/Entity.hpp"
#include "Red/TransactionSystem.hpp"

namespace
{
constexpr auto ModuleName = "AttachmentSlots";

constexpr auto ParentSlotFlat = ".parentSlot";
constexpr auto ForceHairTag = Red::CName("force_Hair");
constexpr auto ForceFlatFeetTag = Red::CName("force_FlatFeet");

const auto s_tppAffectedSlots = {
    Red::TweakDBID("AttachmentSlots.Head"),
    Red::TweakDBID("AttachmentSlots.Eyes"),
};

const auto s_headSlot = Red::TweakDBID("AttachmentSlots.Head");
const auto s_feetSlot = Red::TweakDBID("AttachmentSlots.Feet");
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

    for (const auto slotID : s_tppAffectedSlots)
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

bool App::AttachmentSlotsModule::IsVisualTagActive(Red::IComponent* aComponent,
                                                   Red::TweakDBID aBaseSlotID, Red::CName aVisualTag)
{
    auto transactionSystem = Red::GetGameSystem<Red::ITransactionSystem>();
    auto owner = Red::ToHandle(Raw::IComponent::Owner::Get(aComponent));

    if (IsVisualTagActive(transactionSystem, owner, aBaseSlotID, aVisualTag))
    {
        return true;
    }
    else
    {
        std::shared_lock _(s_slotsMutex);
        const auto& subSlots = s_extraSlots.find(s_feetSlot);
        if (subSlots != s_extraSlots.end())
        {
            for (const auto& subSlotID : subSlots->second)
            {
                if (IsVisualTagActive(transactionSystem, owner, subSlotID, aVisualTag))
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
    Red::Handle<Red::Entity> itemObject{};
    Red::CallVirtual(aTransactionSystem, "GetItemInSlot", itemObject, aOwner, aSlotID);

    if (!itemObject)
        return false;

    Red::ItemID itemID{};
    Red::CallVirtual(itemObject, "GetItemID", itemID);

    Red::CName itemAppearance{};
    Red::CallVirtual(aTransactionSystem, "GetItemAppearance", itemAppearance, aOwner, itemID);

    if (!itemAppearance || itemAppearance == "empty_appearance_default")
        return false;

    bool matched = false;
    Red::CallVirtual(aTransactionSystem, "MatchVisualTag", matched, itemObject, aVisualTag, false);
    // Red::CallVirtual(aTransactionSystem, "MatchVisualTagByItemID", matched, itemID, aOwner, aVisualTag);

    return matched;
}

void App::AttachmentSlotsModule::OnCheckHairState(Red::game::ui::CharacterCustomizationHairstyleController* aComponent,
                                                  Red::CharacterHairState& aHairState)
{
    if (aHairState == Red::CharacterHairState::Hidden)
    {
        if (IsVisualTagActive(aComponent, s_headSlot, ForceHairTag))
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

        if (Raw::TransactionSystem::IsSlotSpawning(transactionSystem, owner, s_feetSlot))
        {
            aLiftedState = Red::CharacterFeetState::Lifted;
            aFlatState = Red::CharacterFeetState::Flat;
        }
    }

    if (aLiftedState == Red::CharacterFeetState::Lifted)
    {
        if (IsVisualTagActive(aComponent, s_feetSlot, ForceFlatFeetTag))
        {
            aLiftedState = Red::CharacterFeetState::Flat;
            aFlatState = Red::CharacterFeetState::Lifted;
        }
    }
}
