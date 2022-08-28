#include "Module.hpp"
#include "Engine/Scripting/RTTILocator.hpp"

namespace
{
constexpr auto ModuleName = "PartsOverrides";
}

std::string_view App::PartsOverridesModule::GetName()
{
    return ModuleName;
}

bool App::PartsOverridesModule::Load()
{
    if (!HookBefore<Raw::GarmentAssembler::AddItem>(&OnAddGarmentItem))
        throw std::runtime_error("Failed to hook [GarmentAssembler::AddItem].");

    if (!HookBefore<Raw::GarmentAssembler::OverrideItem>(&OnOverrideGarmentItem))
        throw std::runtime_error("Failed to hook [GarmentAssembler::OverrideItem].");

    if (!HookBefore<Raw::GarmentAssembler::RemoveItem>(&OnRemoveGarmentItem))
        throw std::runtime_error("Failed to hook [GarmentAssembler::RemoveItem].");

    if (!HookBefore<Raw::GarmentAssembler::OnGameDetach>(&OnGameDetach))
        throw std::runtime_error("Failed to hook [GarmentAssembler::OnGameDetach].");

    if (!HookBefore<Raw::AppearanceChanger::ComputePlayerGarment>(&OnComputeGarment))
        throw std::runtime_error("Failed to hook [AppearanceChanger::ComputePlayerGarment].");

    if (!HookBefore<Raw::Entity::ReassembleAppearance>(&OnReassembleAppearance))
        throw std::runtime_error("Failed to hook [Entity::ReassembleAppearance].");

    s_states = Core::MakeUnique<StateStorage>();

    return true;
}

bool App::PartsOverridesModule::Unload()
{
    Unhook<Raw::GarmentAssembler::AddItem>();
    Unhook<Raw::GarmentAssembler::OverrideItem>();
    Unhook<Raw::GarmentAssembler::RemoveItem>();
    Unhook<Raw::GarmentAssembler::OnGameDetach>();
    Unhook<Raw::AppearanceChanger::ComputePlayerGarment>();
    Unhook<Raw::Entity::ReassembleAppearance>();

    s_states.reset();

    return true;
}

void App::PartsOverridesModule::OnAddGarmentItem(uintptr_t, RED4ext::WeakHandle<RED4ext::ent::Entity>& aEntityWeak,
                                                 GarmentItemAddRequest& aRequest)
{
    if (auto entity = aEntityWeak.Lock())
    {
        std::lock_guard _(s_lock);
        if (auto& entityState = s_states->GetEntityState(entity))
        {
            RegisterOverrides(entityState, aRequest.hash, aRequest.apperance);
        }
    }
}

void App::PartsOverridesModule::OnOverrideGarmentItem(uintptr_t, RED4ext::WeakHandle<RED4ext::ent::Entity>& aEntityWeak,
                                                      GarmentItemOverrideRequest& aRequest)
{
    if (auto entity = aEntityWeak.Lock())
    {
        std::lock_guard _(s_lock);
        if (auto& entityState = s_states->GetEntityState(entity))
        {
            RegisterOverrides(entityState, aRequest.hash, aRequest.apperance);
        }
    }
}

void App::PartsOverridesModule::OnRemoveGarmentItem(uintptr_t, RED4ext::WeakHandle<RED4ext::ent::Entity>& aEntityWeak,
                                                    GarmentItemRemoveRequest& aRequest)
{
    if (auto entity = aEntityWeak.Lock())
    {
        std::lock_guard _(s_lock);
        if (auto& entityState = s_states->GetEntityState(entity))
        {
            UnregisterOverrides(entityState, aRequest.hash);
        }
    }
}

void App::PartsOverridesModule::OnComputeGarment(RED4ext::Handle<RED4ext::ent::Entity>& aEntity, uintptr_t,
                                                 RED4ext::SharedPtr<GarmentComputeData>& aData, uintptr_t,
                                                 uintptr_t, uintptr_t, bool)
{
    std::lock_guard _(s_lock);
    if (auto& entityState = s_states->FindEntityState(aEntity))
    {
        ApplyOverrides(entityState, aData->components);
    }
}

void App::PartsOverridesModule::OnReassembleAppearance(RED4ext::ent::Entity* aEntity, uintptr_t, uintptr_t, uintptr_t,
                                                       uintptr_t, uintptr_t)
{
    std::lock_guard _(s_lock);
    if (auto& entityState = s_states->FindEntityState(aEntity))
    {
        auto& components = Raw::Entity::GetComponents::Call(aEntity);

        ApplyOverrides(entityState, components);
    }
}

void App::PartsOverridesModule::OnGameDetach(uintptr_t)
{
    std::lock_guard _(s_lock);
    s_states->Reset();
}

void App::PartsOverridesModule::RegisterOverrides(Core::SharedPtr<EntityState>& aEntityState, ItemHash aHash,
                                                  ItemAppearance& aApperance)
{
    for (const auto& partsOverride : aApperance->partsOverrides)
    {
        if (partsOverride.partResource.path)
            continue;

        for (const auto& componentOverride : partsOverride.componentsOverrides)
        {
            auto& componentState = aEntityState->GetComponentState(componentOverride.componentName);
            componentState->AddChunkMaskOverride(aHash, componentOverride.chunkMask);

            // LogDebug("|{}| Override added [entity={} component={} item={} mask={}].",
            //          ModuleName, aEntityState->GetName(), componentState->GetName(), aHash, componentOverride.chunkMask);
        }
    }
}

void App::PartsOverridesModule::UnregisterOverrides(Core::SharedPtr<EntityState>& aEntityState, ItemHash aHash)
{
    for (auto& [_, componentState] : aEntityState->GetComponentStates())
    {
        if (componentState->RemoveChunkMaskOverride(aHash))
        {
            // LogDebug("|{}| Override removed [entity={} component={} item={}].",
            //          ModuleName, aEntityState->GetName(), componentState->GetName(), aHash);
        }
    }
}

void App::PartsOverridesModule::ApplyOverrides(Core::SharedPtr<EntityState>& aEntityState,
                                               RED4ext::DynArray<RED4ext::Handle<RED4ext::ent::IComponent>>& aComponents)
{
    auto index = 0;
    for (auto& component : aComponents)
    {
        // LogDebug("|{}| [entity={} index={} component={} type={}].",
        //          ModuleName,
        //          aEntityState->GetName(),
        //          index++,
        //          component->name.ToString(),
        //          component->GetType()->GetName().ToString());

        if (auto& componentState = aEntityState->FindComponentState(component->name))
        {
            componentState->ApplyChunkMask(component);
        }
    }
}
