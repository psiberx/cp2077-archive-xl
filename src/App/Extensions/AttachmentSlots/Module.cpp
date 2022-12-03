#include "Module.hpp"

namespace
{
constexpr auto ModuleName = "AttachmentSlots";

constexpr auto ParentSlotFlat = ".parentSlot";
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

    return true;
}

bool App::AttachmentSlotsModule::Unload()
{
    Unhook<Raw::AttachmentSlots::InitializeSlots>();
    Unhook<Raw::AttachmentSlots::IsSlotSpawning>();

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
