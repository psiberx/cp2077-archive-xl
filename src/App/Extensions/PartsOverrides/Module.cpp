#include "Module.hpp"
#include "Red/Entity.hpp"

namespace
{
constexpr auto ModuleName = "PartsOverrides";
constexpr auto EnableDebugOutput = false;
}

std::string_view App::PartsOverridesModule::GetName()
{
    return ModuleName;
}

bool App::PartsOverridesModule::Load()
{
    if (!HookBefore<Raw::GarmentAssembler::AddItem>(&OnAddItem))
        throw std::runtime_error("Failed to hook [GarmentAssembler::AddItem].");

    if (!HookBefore<Raw::GarmentAssembler::AddCustomItem>(&OnAddCustomItem))
        throw std::runtime_error("Failed to hook [GarmentAssembler::AddCustomItem].");

    if (!HookBefore<Raw::GarmentAssembler::ChangeItem>(&OnChangeItem))
        throw std::runtime_error("Failed to hook [GarmentAssembler::ChangeItem].");

    if (!HookBefore<Raw::GarmentAssembler::ChangeCustomItem>(&OnChangeCustomItem))
        throw std::runtime_error("Failed to hook [GarmentAssembler::ChangeCustomItem].");

    if (!HookBefore<Raw::GarmentAssembler::RemoveItem>(&OnRemoveItem))
        throw std::runtime_error("Failed to hook [GarmentAssembler::RemoveItem].");

    if (!HookBefore<Raw::GarmentAssembler::OnGameDetach>(&OnGameDetach))
        throw std::runtime_error("Failed to hook [GarmentAssembler::OnGameDetach].");

    if (!HookBefore<Raw::AppearanceChanger::ComputePlayerGarment>(&OnComputeGarment))
        throw std::runtime_error("Failed to hook [AppearanceChanger::ComputePlayerGarment].");

    if (!HookBefore<Raw::Entity::ReassembleAppearance>(&OnReassembleAppearance))
        throw std::runtime_error("Failed to hook [Entity::ReassembleAppearance].");

    s_states = Core::MakeUnique<OverrideStateManager>();

    return true;
}

bool App::PartsOverridesModule::Unload()
{
    Unhook<Raw::GarmentAssembler::AddItem>();
    Unhook<Raw::GarmentAssembler::AddCustomItem>();
    Unhook<Raw::GarmentAssembler::ChangeItem>();
    Unhook<Raw::GarmentAssembler::ChangeCustomItem>();
    Unhook<Raw::GarmentAssembler::RemoveItem>();
    Unhook<Raw::GarmentAssembler::OnGameDetach>();
    Unhook<Raw::AppearanceChanger::ComputePlayerGarment>();
    Unhook<Raw::Entity::ReassembleAppearance>();

    s_states.reset();

    return true;
}

void App::PartsOverridesModule::OnAddItem(uintptr_t, Red::WeakHandle<Red::ent::Entity>& aEntityWeak,
                                          Red::GarmentItemAddRequest& aRequest)
{
    if (auto entity = aEntityWeak.Lock())
    {
        std::unique_lock _(s_mutex);
        if (auto& entityState = s_states->GetEntityState(entity))
        {
            if (EnableDebugOutput)
            {
                LogDebug("|{}| [event=AddItem entity={} item={} app={}].",
                         ModuleName, entityState->GetName(), aRequest.hash, aRequest.apperance->name.ToString());
            }

            RegisterOffsetOverrides(entityState, aRequest.hash, aRequest.apperance, aRequest.offset);
            RegisterPartsOverrides(entityState, aRequest.hash, aRequest.apperance);
        }
    }
}

void App::PartsOverridesModule::OnAddCustomItem(uintptr_t, Red::WeakHandle<Red::ent::Entity>& aEntityWeak,
                                                Red::GarmentItemAddCustomRequest& aRequest)
{
    if (auto entity = aEntityWeak.Lock())
    {
        std::unique_lock _(s_mutex);
        if (auto& entityState = s_states->GetEntityState(entity))
        {
            if (EnableDebugOutput)
            {
                LogDebug("|{}| [event=AddCustomItem entity={} item={} app={}].",
                         ModuleName, entityState->GetName(), aRequest.hash, aRequest.apperance->name.ToString());
            }

            RegisterOffsetOverrides(entityState, aRequest.hash, aRequest.apperance, aRequest.offset);
            RegisterPartsOverrides(entityState, aRequest.hash, aRequest.apperance);
            RegisterPartsOverrides(entityState, aRequest.hash, aRequest.overrides);
        }
    }
}

void App::PartsOverridesModule::OnChangeItem(uintptr_t, Red::WeakHandle<Red::ent::Entity>& aEntityWeak,
                                             Red::GarmentItemChangeRequest& aRequest)
{
    if (auto entity = aEntityWeak.Lock())
    {
        std::unique_lock _(s_mutex);
        if (auto& entityState = s_states->GetEntityState(entity))
        {
            if (EnableDebugOutput)
            {
                LogDebug("|{}| [event=ChangeItem entity={} item={} app={}].",
                         ModuleName, entityState->GetName(), aRequest.hash, aRequest.apperance->name.ToString());
            }

            UnregisterPartsOverrides(entityState, aRequest.hash);
            RegisterPartsOverrides(entityState, aRequest.hash, aRequest.apperance);
        }
    }
}

void App::PartsOverridesModule::OnChangeCustomItem(uintptr_t, Red::WeakHandle<Red::ent::Entity>& aEntityWeak,
                                                   Red::GarmentItemChangeCustomRequest& aRequest)
{
    if (auto entity = aEntityWeak.Lock())
    {
        std::unique_lock _(s_mutex);
        if (auto& entityState = s_states->GetEntityState(entity))
        {
            if (EnableDebugOutput)
            {
                LogDebug("|{}| [event=ChangeCustomItem entity={} item={} app={}].",
                         ModuleName, entityState->GetName(), aRequest.hash, aRequest.apperance->name.ToString());
            }

            UnregisterPartsOverrides(entityState, aRequest.hash);
            RegisterPartsOverrides(entityState, aRequest.hash, aRequest.apperance);
            RegisterPartsOverrides(entityState, aRequest.hash, aRequest.overrides);
        }
    }
}

void App::PartsOverridesModule::OnRemoveItem(uintptr_t, Red::WeakHandle<Red::ent::Entity>& aEntityWeak,
                                             Red::GarmentItemRemoveRequest& aRequest)
{
    if (auto entity = aEntityWeak.Lock())
    {
        std::unique_lock _(s_mutex);
        if (auto& entityState = s_states->GetEntityState(entity))
        {
            if (EnableDebugOutput)
            {
                LogDebug("|{}| [event=RemoveItem entity={} item={}].",
                         ModuleName, entityState->GetName(), aRequest.hash);
            }

            UnregisterOffsetOverrides(entityState, aRequest.hash);
            UnregisterPartsOverrides(entityState, aRequest.hash);
        }
    }
}

void App::PartsOverridesModule::OnComputeGarment(Red::Handle<Red::ent::Entity>& aEntity,
                                                 Red::DynArray<int32_t>& aOffsets,
                                                 Red::SharedPtr<Red::GarmentComputeData>& aData,
                                                 uintptr_t, uintptr_t, uintptr_t, bool)
{
    std::unique_lock _(s_mutex);
    if (auto& entityState = s_states->FindEntityState(aEntity))
    {
        if (EnableDebugOutput)
        {
            LogDebug("|{}| [event=ComputeGarment entity={}].",
                     ModuleName, entityState->GetName());
        }

        ApplyChunkMasks(entityState, aData->components);
        ApplyOffsets(entityState, aOffsets, aData->resources);
    }
}

void App::PartsOverridesModule::OnReassembleAppearance(Red::ent::Entity* aEntity, uintptr_t, uintptr_t, uintptr_t,
                                                       uintptr_t, uintptr_t)
{
    std::unique_lock _(s_mutex);
    if (auto& entityState = s_states->FindEntityState(aEntity))
    {
        if (EnableDebugOutput)
        {
            LogDebug("|{}| [event=ReassembleAppearance entity={}].",
                     ModuleName, entityState->GetName());
        }

        ApplyChunkMasks(entityState, true);
    }
}

void App::PartsOverridesModule::OnGameDetach(uintptr_t)
{
    std::unique_lock _(s_mutex);
    s_states->Reset();
}

void App::PartsOverridesModule::RegisterOffsetOverrides(Core::SharedPtr<EntityState>& aEntityState, uint64_t aHash,
                                                        Red::Handle<Red::AppearanceDefinition>& aApperance,
                                                        int32_t aOffset)
{
    for (const auto& partValue : aApperance->partsValues)
    {
        aEntityState->AddOffsetOverride(aHash, partValue.resource.path, aOffset);
    }
}

void App::PartsOverridesModule::RegisterPartsOverrides(Core::SharedPtr<EntityState>& aEntityState, uint64_t aHash,
                                                       Red::Handle<Red::AppearanceDefinition>& aApperance)
{
    RegisterPartsOverrides(aEntityState, aHash, aApperance->partsOverrides);

    for (const auto& visualTag : aApperance->visualTags.tags)
    {
        aEntityState->AddChunkMaskOverride(aHash, visualTag);
    }
}

void App::PartsOverridesModule::RegisterPartsOverrides(Core::SharedPtr<EntityState>& aEntityState, uint64_t aHash,
                                                       Red::DynArray<Red::AppearancePartOverrides>& aOverrides)
{
    for (const auto& partOverrides : aOverrides)
    {
        if (!partOverrides.partResource.path)
        {
            for (const auto& componentOverride : partOverrides.componentsOverrides)
            {
                aEntityState->AddChunkMaskOverride(aHash, componentOverride.componentName, componentOverride.chunkMask);
            }
        }
    }
}

void App::PartsOverridesModule::UnregisterOffsetOverrides(Core::SharedPtr<EntityState>& aEntityState, uint64_t aHash)
{
    aEntityState->RemoveOffsetOverrides(aHash);
}

void App::PartsOverridesModule::UnregisterPartsOverrides(Core::SharedPtr<EntityState>& aEntityState, uint64_t aHash)
{
    aEntityState->RemoveChunkMaskOverrides(aHash);
}

void App::PartsOverridesModule::ApplyOffsets(Core::SharedPtr<App::EntityState>& aEntityState,
                                             Red::DynArray<int32_t>& aOffsets,
                                             Red::DynArray<Red::ResourcePath>& aResourcePaths)
{
    if (aOffsets.size != aResourcePaths.size)
    {
        aOffsets.Reserve(aResourcePaths.size);
        aOffsets.Clear();
    }

    for (auto& resourcePath : aResourcePaths)
    {
        aOffsets.PushBack(aEntityState->GetOrderOffset(resourcePath));
    }
}

void App::PartsOverridesModule::ApplyChunkMasks(Core::SharedPtr<EntityState>& aEntityState,
                                                Red::DynArray<Red::Handle<Red::ent::IComponent>>& aComponents,
                                                bool aVerbose)
{
    auto index = 0;

    for (auto& component : aComponents)
    {
        aEntityState->ApplyChunkMasks(component);

        if (EnableDebugOutput && aVerbose && component->isEnabled)
        {
            LogDebug("|{}| [entity={} index={} component={} type={}].",
                     ModuleName,
                     aEntityState->GetName(),
                     index++,
                     component->name.ToString(),
                     component->GetType()->GetName().ToString());
        }
    }
}

void App::PartsOverridesModule::ApplyChunkMasks(Core::SharedPtr<EntityState>& aEntityState, bool aVerbose)
{
    ApplyChunkMasks(aEntityState, Raw::Entity::GetComponents(aEntityState->GetEntity()), aVerbose);
}
