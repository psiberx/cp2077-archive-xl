#include "Module.hpp"
#include "Red/TransactionSystem.hpp"

namespace
{
constexpr auto ModuleName = "AttachmentSlots";

constexpr auto ParentSlotFlat = ".parentSlot";

const Red::TweakDBID s_tppAffectedSlots[] = {
    Red::TweakDBID("AttachmentSlots.Head"),
    Red::TweakDBID("AttachmentSlots.Eyes"),
};

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

    if (!HookAfter<Raw::CharacterCustomizationFeetController::CheckState>(&OnCheckFeetState))
        throw std::runtime_error("Failed to hook [CharacterCustomizationFeetController::CheckState].");

    return true;
}

bool App::AttachmentSlotsModule::Unload()
{
    Unhook<Raw::AttachmentSlots::InitializeSlots>();
    Unhook<Raw::AttachmentSlots::IsSlotSpawning>();
    Unhook<Raw::TPPRepresentationComponent::OnAttach>();
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
                    s_slots[parentSlotID].insert(slotID);
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
        const auto& subSlots = s_slots.find(aSlotID);

        if (subSlots != s_slots.end())
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
        const auto& subSlots = s_slots.find(slotID);

        if (subSlots != s_slots.end())
        {
            for (const auto& subSlotID : subSlots->second)
            {
                aComponent->affectedAppearanceSlots.PushBack(subSlotID);
            }
        }
    }
}

void App::AttachmentSlotsModule::OnCheckFeetState(Red::game::ui::CharacterCustomizationFeetController* aComponent,
                                                  Red::CharacterFeetState& aLiftedState,
                                                  Red::CharacterFeetState& aFlatState)
{
    if (aLiftedState == Red::CharacterFeetState::Lifted)
        return;

    auto transactionSystem = Red::GetGameSystem<Red::ITransactionSystem>();

    Red::Handle<Red::IScriptable> ownerObject;
    Raw::CharacterCustomizationFeetController::GetOwner(aComponent, ownerObject);

    if (Raw::TransactionSystem::IsSlotSpawning(transactionSystem, ownerObject, s_feetSlot))
    {
        aLiftedState = Red::CharacterFeetState::Lifted;
        aFlatState = Red::CharacterFeetState::Flat;
    }
}
