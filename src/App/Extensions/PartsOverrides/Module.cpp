#include "Module.hpp"
#include "Red/Entity.hpp"
#include "Red/ResourcePath.hpp"

namespace
{
constexpr auto ModuleName = "PartsOverrides";

constexpr auto BodyPartTag = Red::CName("PlayerBodyPart");

constexpr auto DefaultAppearanceName = Red::CName("default");
constexpr auto EmptyAppearanceName = Red::CName("empty_appearance_default");
constexpr auto EmptyAppearancePath = Red::ResourcePath(R"(base\characters\appearances\player\items\empty_appearance.app)");
constexpr auto RandomAppearanceName = Red::CName("random");

constexpr auto EmptyAppearanceTags = {
    std::pair(Red::CName("EmptyAppearance:FPP"), "&FPP"),
    std::pair(Red::CName("EmptyAppearance:Male"), "&Male"),
    std::pair(Red::CName("EmptyAppearance:Female"), "&Female"),
};

constexpr auto EnableDebugOutput = false;
}

std::string_view App::PartsOverridesModule::GetName()
{
    return ModuleName;
}

bool App::PartsOverridesModule::Load()
{
    if (!HookBefore<Raw::ItemFactoryRequest::LoadAppearance>(&OnLoadAppearanceResource))
        throw std::runtime_error("Failed to hook [ItemFactoryRequest::LoadAppearance].");

    if (!HookBefore<Raw::ItemFactoryAppearanceChangeRequest::LoadAppearance>(&OnChangeAppearanceResource))
        throw std::runtime_error("Failed to hook [ItemFactoryAppearanceChangeRequest::LoadAppearance].");

    if (!Hook<Raw::EntityTemplate::FindAppearance>(&OnResolveAppearance))
        throw std::runtime_error("Failed to hook [EntityTemplate::FindAppearance].");

    if (!HookAfter<Raw::AppearanceResource::FindAppearance>(&OnResolveDefinition))
        throw std::runtime_error("Failed to hook [AppearanceResource::FindAppearance].");

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

    if (!Hook<Raw::GarmentAssembler::ProcessGarment>(&OnProcessGarment))
        throw std::runtime_error("Failed to hook [GarmentAssembler::ProcessGarment].");

    if (!HookBefore<Raw::GarmentAssembler::ProcessSkinnedMesh>(&OnProcessGarmentMesh))
        throw std::runtime_error("Failed to hook [GarmentAssembler::ProcessSkinnedMesh].");

    if (!HookBefore<Raw::GarmentAssembler::ProcessMorphedMesh>(&OnProcessGarmentMesh))
        throw std::runtime_error("Failed to hook [GarmentAssembler::ProcessMorphedMesh].");

    if (!HookBefore<Raw::GarmentAssembler::OnGameDetach>(&OnGameDetach))
        throw std::runtime_error("Failed to hook [GarmentAssembler::OnGameDetach].");

    if (!HookBefore<Raw::AppearanceChanger::RegisterPart>(&OnRegisterPart))
        throw std::runtime_error("Failed to hook [AppearanceChanger::RegisterPart].");

    if (!Hook<Raw::AppearanceChanger::GetBaseMeshOffset>(&OnGetBaseMeshOffset))
        throw std::runtime_error("Failed to hook [AppearanceChanger::GetBaseMeshOffset].");

    if (!HookBefore<Raw::AppearanceChanger::ComputePlayerGarment>(&OnComputeGarment))
        throw std::runtime_error("Failed to hook [AppearanceChanger::ComputePlayerGarment].");

    if (!HookBefore<Raw::Entity::ReassembleAppearance>(&OnReassembleAppearance))
        throw std::runtime_error("Failed to hook [Entity::ReassembleAppearance].");

    if (!HookAfter<Raw::ResourcePath::Create>(&OnCreateResourcePath))
        throw std::runtime_error("Failed to hook [ResourcePath::Create].");

    s_emptyAppearance = Core::MakeUnique<Red::TemplateAppearance>();
    s_emptyAppearance->name = EmptyAppearanceName;
    s_emptyAppearance->appearanceName = DefaultAppearanceName;
    s_emptyAppearance->appearanceResource = EmptyAppearancePath;

    s_dynamicAppearance = Core::MakeShared<DynamicAppearanceController>();
    s_stateManager = Core::MakeUnique<OverrideStateManager>(s_dynamicAppearance);
    s_tagManager = Core::MakeShared<OverrideTagManager>();

    ConfigureTags();

    return true;
}

void App::PartsOverridesModule::Reload()
{
    ConfigureTags();
}

bool App::PartsOverridesModule::Unload()
{
    Unhook<Raw::ItemFactoryRequest::LoadAppearance>();
    Unhook<Raw::ItemFactoryAppearanceChangeRequest::LoadAppearance>();
    Unhook<Raw::EntityTemplate::FindAppearance>();
    Unhook<Raw::AppearanceResource::FindAppearance>();
    Unhook<Raw::GarmentAssembler::AddItem>();
    Unhook<Raw::GarmentAssembler::AddCustomItem>();
    Unhook<Raw::GarmentAssembler::ChangeItem>();
    Unhook<Raw::GarmentAssembler::ChangeCustomItem>();
    Unhook<Raw::GarmentAssembler::RemoveItem>();
    Unhook<Raw::GarmentAssembler::ProcessGarment>();
    Unhook<Raw::GarmentAssembler::ProcessSkinnedMesh>();
    Unhook<Raw::GarmentAssembler::ProcessMorphedMesh>();
    Unhook<Raw::GarmentAssembler::OnGameDetach>();
    Unhook<Raw::AppearanceChanger::RegisterPart>();
    Unhook<Raw::AppearanceChanger::GetBaseMeshOffset>();
    Unhook<Raw::AppearanceChanger::ComputePlayerGarment>();
    Unhook<Raw::Entity::ReassembleAppearance>();
    Unhook<Raw::ResourcePath::Create>();

    s_dynamicAppearance.reset();
    s_stateManager.reset();
    s_tagManager.reset();

    return true;
}

void App::PartsOverridesModule::ConfigureTags()
{
    std::unique_lock _(s_mutex);
    for (const auto& unit : m_units)
    {
        for (const auto& tag : unit.tags)
        {
            s_tagManager->DefineTag(tag.first.data(), tag.second);
        }
    }
}

void App::PartsOverridesModule::OnLoadAppearanceResource(Red::ItemFactoryRequest* aRequest)
{
    Raw::ItemFactoryRequest::Entity entityWeak(aRequest);
    Raw::ItemFactoryRequest::EntityTemplate templateToken(aRequest);
    Raw::ItemFactoryRequest::AppearanceName appearanceName(aRequest);

    if (PrepareDynamicAppearanceName(entityWeak, templateToken, appearanceName))
    {
        std::unique_lock _(s_mutex);
        if (auto& entityState = s_stateManager->GetEntityState(entityWeak->instance))
        {
            UpdateDynamicAttributes(entityState);
        }
    }
}

void App::PartsOverridesModule::OnChangeAppearanceResource(Red::ItemFactoryAppearanceChangeRequest* aRequest)
{
    Raw::ItemFactoryAppearanceChangeRequest::Entity entityWeak(aRequest);
    Raw::ItemFactoryAppearanceChangeRequest::EntityTemplate templateToken(aRequest);
    Raw::ItemFactoryAppearanceChangeRequest::AppearanceName appearanceName(aRequest);

    if (PrepareDynamicAppearanceName(entityWeak, templateToken, appearanceName))
    {
        std::unique_lock _(s_mutex);
        if (auto& entityState = s_stateManager->GetEntityState(entityWeak->instance))
        {
            UpdateDynamicAttributes(entityState);
        }
    }
}

Red::TemplateAppearance* App::PartsOverridesModule::OnResolveAppearance(Red::EntityTemplate* aTemplate,
                                                                        Red::CName aSelector)
{
    if (aSelector == EmptyAppearanceName)
        return s_emptyAppearance.get();

    auto appearance = Raw::EntityTemplate::FindAppearance(aTemplate, aSelector);

    if (!IsUniqueAppearance(aSelector))
        return appearance;

    if (s_dynamicAppearance->SupportsDynamicAppearance(aTemplate))
    {
        auto selector = s_dynamicAppearance->ParseAppearance(aSelector);
        if (selector.isDynamic && selector.context)
        {
            std::unique_lock _(s_mutex);
            if (auto& entityState = s_stateManager->GetEntityState(selector.context))
            {
                SelectDynamicAppearance(entityState, selector, aTemplate, appearance);
                return appearance;
            }
        }
    }

    if (aTemplate->visualTagsSchema && !aTemplate->visualTagsSchema->visualTags.IsEmpty())
    {
        const auto& visualTags = aTemplate->visualTagsSchema->visualTags;
        std::string_view selectorStr = aSelector.ToString();

        for (const auto& tag : EmptyAppearanceTags)
        {
            if (visualTags.Contains(tag.first) && selectorStr.find(tag.second) != std::string_view::npos)
            {
                {
                    std::unique_lock _(s_mutex);
                    aTemplate->appearances.EmplaceBack(*s_emptyAppearance);
                    appearance = aTemplate->appearances.End() - 1;
                }

                appearance->name = aSelector;
                return appearance;
            }
        }
    }

    return appearance;
}

void App::PartsOverridesModule::OnResolveDefinition(Red::AppearanceResource* aResource,
                                                    Red::Handle<Red::AppearanceDefinition>* aDefinition,
                                                    Red::CName aSelector, uint32_t a4, uint8_t a5)
{
    if (!*aDefinition)
    {
        auto selector = s_dynamicAppearance->ParseAppearance(aSelector);
        if (selector.isDynamic && selector.context)
        {
            std::unique_lock _(s_mutex);
            if (auto& entityState = s_stateManager->GetEntityState(selector.context))
            {
                SelectDynamicAppearance(entityState, selector, aResource, *aDefinition);
            }
        }
    }
}

void App::PartsOverridesModule::OnAddItem(uintptr_t, Red::WeakHandle<Red::Entity>& aEntityWeak,
                                          Red::GarmentItemAddRequest& aRequest)
{
    if (auto entity = aEntityWeak.Lock())
    {
        std::unique_lock _(s_mutex);
        if (auto& entityState = s_stateManager->GetEntityState(entity))
        {
            if constexpr (EnableDebugOutput)
            {
                LogDebug("|{}| [event=AddItem entity={} item={} app={}].",
                         ModuleName, entityState->GetName(), aRequest.hash, aRequest.apperance->name.ToString());
            }

            UpdatePartAttributes(entityState, aRequest.apperance);
            RegisterOffsetOverrides(entityState, aRequest.hash, aRequest.apperance, aRequest.offset);
            RegisterComponentOverrides(entityState, aRequest.hash, aRequest.apperance);
        }
    }
}

void App::PartsOverridesModule::OnAddCustomItem(uintptr_t, Red::WeakHandle<Red::Entity>& aEntityWeak,
                                                Red::GarmentItemAddCustomRequest& aRequest)
{
    if (auto entity = aEntityWeak.Lock())
    {
        std::unique_lock _(s_mutex);
        if (auto& entityState = s_stateManager->GetEntityState(entity))
        {
            if constexpr (EnableDebugOutput)
            {
                LogDebug("|{}| [event=AddCustomItem entity={} item={} app={}].",
                         ModuleName, entityState->GetName(), aRequest.hash, aRequest.apperance->name.ToString());
            }

            UpdatePartAttributes(entityState, aRequest.apperance);
            RegisterOffsetOverrides(entityState, aRequest.hash, aRequest.apperance, aRequest.offset);
            RegisterComponentOverrides(entityState, aRequest.hash, aRequest.apperance);
            RegisterComponentOverrides(entityState, aRequest.hash, aRequest.overrides);
        }
    }
}

void App::PartsOverridesModule::OnChangeItem(uintptr_t, Red::WeakHandle<Red::Entity>& aEntityWeak,
                                             Red::GarmentItemChangeRequest& aRequest)
{
    if (auto entity = aEntityWeak.Lock())
    {
        std::unique_lock _(s_mutex);
        if (auto& entityState = s_stateManager->GetEntityState(entity))
        {
            if constexpr (EnableDebugOutput)
            {
                LogDebug("|{}| [event=ChangeItem entity={} item={} app={}].",
                         ModuleName, entityState->GetName(), aRequest.hash, aRequest.apperance->name.ToString());
            }

            UnregisterComponentOverrides(entityState, aRequest.hash);
            UpdatePartAttributes(entityState, aRequest.apperance);
            RegisterComponentOverrides(entityState, aRequest.hash, aRequest.apperance);
        }
    }
}

void App::PartsOverridesModule::OnChangeCustomItem(uintptr_t, Red::WeakHandle<Red::Entity>& aEntityWeak,
                                                   Red::GarmentItemChangeCustomRequest& aRequest)
{
    if (auto entity = aEntityWeak.Lock())
    {
        std::unique_lock _(s_mutex);
        if (auto& entityState = s_stateManager->GetEntityState(entity))
        {
            if constexpr (EnableDebugOutput)
            {
                LogDebug("|{}| [event=ChangeCustomItem entity={} item={} app={}].",
                         ModuleName, entityState->GetName(), aRequest.hash, aRequest.apperance->name.ToString());
            }

            UnregisterComponentOverrides(entityState, aRequest.hash);
            UpdatePartAttributes(entityState, aRequest.apperance);
            RegisterComponentOverrides(entityState, aRequest.hash, aRequest.apperance);
            RegisterComponentOverrides(entityState, aRequest.hash, aRequest.overrides);
        }
    }
}

void App::PartsOverridesModule::OnRemoveItem(uintptr_t, Red::WeakHandle<Red::Entity>& aEntityWeak,
                                             Red::GarmentItemRemoveRequest& aRequest)
{
    if (auto entity = aEntityWeak.Lock())
    {
        std::unique_lock _(s_mutex);
        if (auto& entityState = s_stateManager->GetEntityState(entity))
        {
            if constexpr (EnableDebugOutput)
            {
                LogDebug("|{}| [event=RemoveItem entity={} item={}].",
                         ModuleName, entityState->GetName(), aRequest.hash);
            }

            UnregisterOffsetOverrides(entityState, aRequest.hash);
            UnregisterComponentOverrides(entityState, aRequest.hash);
        }
    }
}

void App::PartsOverridesModule::OnRegisterPart(uintptr_t, Red::Handle<Red::EntityTemplate>& aPart,
                                               Red::Handle<Red::ComponentsStorage>& aComponentStorage,
                                               Red::Handle<Red::AppearanceDefinition>& aAppearance)
{
    std::unique_lock _(s_mutex);
    UpdatePartAssignments(aComponentStorage->components, aPart->path);
    UpdateDynamicAttributes();
}

uintptr_t App::PartsOverridesModule::OnProcessGarment(Red::SharedPtr<Red::GarmentProcessor>& aProcessor,
                                                      uintptr_t a2, uintptr_t a3, Red::Handle<Red::Entity>& aEntity)
{
    std::unique_lock _(s_mutex);
    if (auto& entityState = s_stateManager->FindEntityState(aEntity))
    {
        UpdateDynamicAttributes(entityState);
    }

    auto result = Raw::GarmentAssembler::ProcessGarment(aProcessor, a2, a3, aEntity);

    s_stateManager->LinkEntityToAssembler(aEntity, aProcessor);

    return result;
}

void App::PartsOverridesModule::OnProcessGarmentMesh(Red::GarmentProcessor* aProcessor, uint32_t,
                                                     Red::Handle<Red::EntityTemplate>& aPartTemplate,
                                                     Red::SharedPtr<Red::ResourceToken<Red::CMesh>>& aMeshToken,
                                                     Red::Handle<Red::IComponent>& aComponent,
                                                     Red::JobGroup& aJobGroup)
{
    if (aMeshToken->IsFailed())
    {
        std::unique_lock _(s_mutex);
        if (auto& entityState = s_stateManager->FindEntityState(aProcessor))
        {
            entityState->ApplyDynamicAppearance(aComponent, aPartTemplate->path);
            aMeshToken = ComponentWrapper(aComponent).LoadResource(true);
        }
    }
}

int64_t App::PartsOverridesModule::OnGetBaseMeshOffset(Red::Handle<Red::IComponent>& aComponent,
                                                       Red::Handle<Red::EntityTemplate>& aTemplate)
{
    if (s_garmentOffsetsEnabled)
    {
        if (!aTemplate || !aTemplate->visualTagsSchema || !aTemplate->visualTagsSchema->visualTags.Contains(BodyPartTag))
        {
            if constexpr (EnableDebugOutput)
            {
                LogDebug("|{}| [event=GetBaseMeshOffset comp={} offset=0].", ModuleName, aComponent->name.ToString());
            }

            return 0;
        }
    }

    return Raw::AppearanceChanger::GetBaseMeshOffset(aComponent, aTemplate);
}

void App::PartsOverridesModule::OnComputeGarment(Red::Handle<Red::Entity>& aEntity,
                                                 Red::DynArray<int32_t>& aOffsets,
                                                 Red::SharedPtr<Red::GarmentComputeData>& aData,
                                                 uintptr_t, uintptr_t, uintptr_t, bool)
{
    std::unique_lock _(s_mutex);
    if (auto& entityState = s_stateManager->FindEntityState(aEntity))
    {
        if constexpr (EnableDebugOutput)
        {
            LogDebug("|{}| [event=ComputeGarment entity={}].",
                     ModuleName, entityState->GetName());
        }

        UpdatePartAssignments(entityState, aData->components, aData->resources);
        UpdateDynamicAttributes(entityState);

        ApplyDynamicAppearance(entityState, aData->components);
        ApplyComponentOverrides(entityState, aData->components, true);
        ApplyOffsetOverrides(entityState, aOffsets, aData->resources);
    }
}

void App::PartsOverridesModule::OnReassembleAppearance(Red::Entity* aEntity, uintptr_t, uintptr_t, uintptr_t,
                                                       uintptr_t, uintptr_t)
{
    std::unique_lock _(s_mutex);
    if (auto& entityState = s_stateManager->FindEntityState(aEntity))
    {
        if constexpr (EnableDebugOutput)
        {
            LogDebug("|{}| [event=ReassembleAppearance entity={}].",
                     ModuleName, entityState->GetName());
        }

        ApplyDynamicAppearance(entityState);
        ApplyComponentOverrides(entityState, true);
    }
}

void App::PartsOverridesModule::OnCreateResourcePath(Red::ResourcePath* aPath, const std::string_view* aPathStr)
{
    if (aPathStr && s_dynamicAppearance->IsDynamicValue(*aPathStr))
    {
        std::unique_lock _(s_mutex);
        s_dynamicAppearance->RegisterPath(*aPath, *aPathStr);
    }
}

void App::PartsOverridesModule::OnGameDetach()
{
    std::unique_lock _(s_mutex);
    s_stateManager->ClearStates();
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

void App::PartsOverridesModule::RegisterComponentOverrides(Core::SharedPtr<EntityState>& aEntityState, uint64_t aHash,
                                                       Red::Handle<Red::AppearanceDefinition>& aApperance)
{
    RegisterComponentOverrides(aEntityState, aHash, aApperance->partsOverrides);

    for (const auto& visualTag : aApperance->visualTags.tags)
    {
        for (auto& [componentName, chunkMask] : s_tagManager->GetOverrides(visualTag))
        {
            aEntityState->AddChunkMaskOverride(aHash, componentName, chunkMask.mask, chunkMask.set);
        }
    }
}

void App::PartsOverridesModule::RegisterComponentOverrides(Core::SharedPtr<EntityState>& aEntityState, uint64_t aHash,
                                                       Red::DynArray<Red::AppearancePartOverrides>& aOverrides)
{
    for (const auto& partOverrides : aOverrides)
    {
        if (!partOverrides.partResource.path)
        {
            for (const auto& componentOverride : partOverrides.componentsOverrides)
            {
                aEntityState->AddChunkMaskOverride(aHash, componentOverride.componentName, componentOverride.chunkMask);
                aEntityState->AddAppearanceOverride(aHash, componentOverride.componentName, componentOverride.meshAppearance);
            }
        }
    }
}

void App::PartsOverridesModule::UnregisterOffsetOverrides(Core::SharedPtr<EntityState>& aEntityState, uint64_t aHash)
{
    aEntityState->RemoveOffsetOverrides(aHash);
}

void App::PartsOverridesModule::UnregisterComponentOverrides(Core::SharedPtr<EntityState>& aEntityState, uint64_t aHash)
{
    aEntityState->RemoveChunkMaskOverrides(aHash);
    aEntityState->RemoveAppearanceOverrides(aHash);
}

void App::PartsOverridesModule::UpdatePartAttributes(Core::SharedPtr<EntityState>& aEntityState,
                                                     Red::Handle<Red::AppearanceDefinition>& aApperance)
{
    for (const auto& partValue : aApperance->partsValues)
    {
        aEntityState->LinkPartToAppearance(partValue.resource.path, aApperance->name);
    }
}

void App::PartsOverridesModule::UpdatePartAssignments(Red::DynArray<Red::Handle<Red::IComponent>>& aComponents,
                                                      Red::ResourcePath aPartResource)
{
    for (auto i = 0; i < aComponents.size; ++i)
    {
        s_stateManager->LinkComponentToPart(aComponents[i], aPartResource);
    }
}

void App::PartsOverridesModule::UpdatePartAssignments(Core::SharedPtr<App::EntityState>& aEntityState,
                                                      Red::DynArray<Red::Handle<Red::IComponent>>& aComponents,
                                                      Red::DynArray<Red::ResourcePath>& aPartResources)
{
    for (auto i = 0; i < aComponents.size; ++i)
    {
        aEntityState->LinkComponentToPart(aComponents[i], aPartResources[i]);
    }
}

bool App::PartsOverridesModule::PrepareDynamicAppearanceName(Red::WeakHandle<Red::Entity>& aEntity,
                                                             Red::ResourceTokenPtr<Red::EntityTemplate>& aTemplateToken,
                                                             Red::CName& aAppearanceName)
{
    if (IsUniqueAppearance(aAppearanceName) &&
        s_dynamicAppearance->SupportsDynamicAppearance(aTemplateToken->resource))
    {
        s_dynamicAppearance->MarkDynamicAppearanceName(aAppearanceName, aEntity.instance);
        return true;
    }

    return false;
}

void App::PartsOverridesModule::SelectDynamicAppearance(Core::SharedPtr<App::EntityState>& aEntityState,
                                                        App::DynamicAppearanceName& aSelector,
                                                        Red::EntityTemplate* aResource,
                                                        Red::TemplateAppearance*& aAppearance)
{
    aEntityState->SelectTemplateAppearance(aSelector, aResource, aAppearance);
}

void App::PartsOverridesModule::SelectDynamicAppearance(Core::SharedPtr<EntityState>& aEntityState,
                                                        DynamicAppearanceName& aSelector,
                                                        Red::AppearanceResource* aResource,
                                                        Red::Handle<Red::AppearanceDefinition>& aDefinition)
{
    aEntityState->SelectConditionalAppearance(aSelector, aResource, aDefinition);
}

void App::PartsOverridesModule::ApplyDynamicAppearance(Core::SharedPtr<EntityState>& aEntityState,
                                                       Red::DynArray<Red::Handle<Red::IComponent>>& aComponents)
{
    aEntityState->ProcessConditionalComponents(aComponents);

    for (auto& component : aComponents)
    {
        aEntityState->ApplyDynamicAppearance(component);
    }
}

void App::PartsOverridesModule::ApplyDynamicAppearance(Core::SharedPtr<EntityState>& aEntityState)
{
    ApplyDynamicAppearance(aEntityState, Raw::Entity::ComponentsStorage(aEntityState->GetEntity())->components);
}

void App::PartsOverridesModule::ApplyComponentOverrides(Core::SharedPtr<EntityState>& aEntityState,
                                                        Red::DynArray<Red::Handle<Red::IComponent>>& aComponents,
                                                        bool aVerbose)
{
    for (auto& component : aComponents)
    {
        aEntityState->ApplyAppearanceOverride(component);
        aEntityState->ApplyChunkMaskOverride(component);
    }

    if constexpr (EnableDebugOutput)
    {
        if (aVerbose)
        {
            auto index = 0;
            for (auto& component : aComponents)
            {
                auto wrapper = ComponentWrapper(component);

                if (wrapper.IsMeshComponent())
                {
                    LogDebug("|{}| [entity={} index={} component={} type={} enabled={} app={} chunks={:064b}].",
                             ModuleName,
                             aEntityState->GetName(),
                             index,
                             component->name.ToString(),
                             component->GetType()->GetName().ToString(),
                             component->isEnabled,
                             wrapper.GetAppearance().ToString(),
                             wrapper.GetChunkMask());
                }
                else
                {
                    LogDebug("|{}| [entity={} index={} component={} type={} enabled={}].",
                             ModuleName,
                             aEntityState->GetName(),
                             index,
                             component->name.ToString(),
                             component->GetType()->GetName().ToString(),
                             component->isEnabled);
                }

                ++index;
            }
        }
    }
}

void App::PartsOverridesModule::ApplyComponentOverrides(Core::SharedPtr<EntityState>& aEntityState, bool aVerbose)
{
    ApplyComponentOverrides(aEntityState, Raw::Entity::GetComponents(aEntityState->GetEntity()), aVerbose);
}

void App::PartsOverridesModule::ApplyOffsetOverrides(Core::SharedPtr<App::EntityState>& aEntityState,
                                                     Red::DynArray<int32_t>& aOffsets,
                                                     Red::DynArray<Red::ResourcePath>& aResourcePaths)
{
    if (aOffsets.size != aResourcePaths.size)
    {
        aOffsets.Reserve(aResourcePaths.size);
    }

    if (s_garmentOffsetsEnabled)
    {
        aEntityState->ApplyOffsetOverrides(aResourcePaths, aOffsets);
    }
    else
    {
        aOffsets.Clear();
    }
}

void App::PartsOverridesModule::EnableGarmentOffsets()
{
    s_garmentOffsetsEnabled = true;
}

void App::PartsOverridesModule::DisableGarmentOffsets()
{
    s_garmentOffsetsEnabled = false;
}

void App::PartsOverridesModule::UpdateDynamicAttributes(Core::SharedPtr<EntityState>& aEntityState)
{
    aEntityState->UpdateDynamicAttributes();
}

void App::PartsOverridesModule::UpdateDynamicAttributes()
{
    s_stateManager->UpdateDynamicAttributes();
}

bool App::PartsOverridesModule::IsUniqueAppearance(Red::CName aName)
{
    return aName && aName != DefaultAppearanceName && aName != RandomAppearanceName && aName != EmptyAppearanceName;
}
