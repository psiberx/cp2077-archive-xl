#include "Module.hpp"
#include "Red/Entity.hpp"
#include "Red/ResourcePath.hpp"
#include "Red/TweakDB.hpp"

namespace
{
constexpr auto ModuleName = "GarmentOverride";

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

const Red::ClassLocator<Red::CMesh> s_meshResourceType;
}

std::string_view App::GarmentOverrideModule::GetName()
{
    return ModuleName;
}

bool App::GarmentOverrideModule::Load()
{
    if (!HookBefore<Raw::ItemFactoryRequest::LoadAppearance>(&OnLoadAppearanceResource))
        throw std::runtime_error("Failed to hook [ItemFactoryRequest::LoadAppearance].");

    if (!HookBefore<Raw::ItemFactoryAppearanceChangeRequest::LoadAppearance>(&OnChangeAppearanceResource))
        throw std::runtime_error("Failed to hook [ItemFactoryAppearanceChangeRequest::LoadAppearance].");

    if (!Hook<Raw::EntityTemplate::FindAppearance>(&OnResolveAppearance))
        throw std::runtime_error("Failed to hook [EntityTemplate::FindAppearance].");

    if (!HookAfter<Raw::AppearanceResource::FindAppearance>(&OnResolveDefinition))
        throw std::runtime_error("Failed to hook [AppearanceResource::FindAppearance].");

    if (!HookAfter<Raw::AppearanceNameVisualTagsPreset::GetVisualTags>(&OnGetVisualTags))
        throw std::runtime_error("Failed to hook [AppearanceNameVisualTagsPreset::GetVisualTags].");

    if (!HookAfter<Raw::GarmentAssembler::FindState>(&OnFindState))
        throw std::runtime_error("Failed to hook [GarmentAssembler::FindState].");

    if (!HookBefore<Raw::GarmentAssemblerState::AddItem>(&OnAddItem))
        throw std::runtime_error("Failed to hook [GarmentAssemblerState::AddItem].");

    if (!HookBefore<Raw::GarmentAssemblerState::AddCustomItem>(&OnAddCustomItem))
        throw std::runtime_error("Failed to hook [GarmentAssemblerState::AddCustomItem].");

    if (!HookBefore<Raw::GarmentAssemblerState::ChangeItem>(&OnChangeItem))
        throw std::runtime_error("Failed to hook [GarmentAssemblerState::ChangeItem].");

    if (!HookBefore<Raw::GarmentAssemblerState::ChangeCustomItem>(&OnChangeCustomItem))
        throw std::runtime_error("Failed to hook [GarmentAssemblerState::ChangeCustomItem].");

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

void App::GarmentOverrideModule::Reload()
{
    ConfigureTags();
}

bool App::GarmentOverrideModule::Unload()
{
    Unhook<Raw::ItemFactoryRequest::LoadAppearance>();
    Unhook<Raw::ItemFactoryAppearanceChangeRequest::LoadAppearance>();
    Unhook<Raw::EntityTemplate::FindAppearance>();
    Unhook<Raw::AppearanceResource::FindAppearance>();
    Unhook<Raw::AppearanceNameVisualTagsPreset::GetVisualTags>();
    Unhook<Raw::GarmentAssembler::FindState>();
    Unhook<Raw::GarmentAssemblerState::AddItem>();
    Unhook<Raw::GarmentAssemblerState::AddCustomItem>();
    Unhook<Raw::GarmentAssemblerState::ChangeItem>();
    Unhook<Raw::GarmentAssemblerState::ChangeCustomItem>();
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

void App::GarmentOverrideModule::ConfigureTags()
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

void App::GarmentOverrideModule::OnLoadAppearanceResource(Red::ItemFactoryRequest* aRequest)
{
    auto& entityWeak = Raw::ItemFactoryRequest::Entity::Ref(aRequest);
    auto& templateToken = Raw::ItemFactoryRequest::EntityTemplate::Ref(aRequest);
    auto& appearanceName = Raw::ItemFactoryRequest::AppearanceName::Ref(aRequest);

    if (!appearanceName)
    {
        auto itemRecord = Raw::ItemFactoryRequest::ItemRecord::Ptr(aRequest);
        if (itemRecord)
        {
            appearanceName = Red::GetFlatValue<Red::CName>({itemRecord->recordID, ".appearanceName"});
        }
    }

    if (PrepareDynamicAppearanceName(entityWeak, templateToken, appearanceName))
    {
        std::unique_lock _(s_mutex);
        if (auto& entityState = s_stateManager->GetEntityState(entityWeak.instance))
        {
#ifndef NDEBUG
            LogDebug("|{}| [event=LoadAppearanceResource entity={}]", ModuleName, entityState->GetName());
#endif
            UpdateDynamicAttributes(entityState);
        }
    }
}

void App::GarmentOverrideModule::OnChangeAppearanceResource(Red::ItemFactoryAppearanceChangeRequest* aRequest)
{
    auto& entityWeak = Raw::ItemFactoryAppearanceChangeRequest::Entity::Ref(aRequest);
    auto& templateToken = Raw::ItemFactoryAppearanceChangeRequest::EntityTemplate::Ref(aRequest);
    auto& appearanceName = Raw::ItemFactoryAppearanceChangeRequest::AppearanceName::Ref(aRequest);

    if (!appearanceName)
    {
        auto itemRecord = Raw::ItemFactoryAppearanceChangeRequest::ItemRecord::Ptr(aRequest);
        if (itemRecord)
        {
            appearanceName = Red::GetFlatValue<Red::CName>({itemRecord->recordID, ".appearanceName"});
        }
    }

    if (PrepareDynamicAppearanceName(entityWeak, templateToken, appearanceName))
    {
        std::unique_lock _(s_mutex);
        if (auto& entityState = s_stateManager->GetEntityState(entityWeak.instance))
        {
#ifndef NDEBUG
            LogDebug("|{}| [event=ChangeAppearanceResource entity={}]", ModuleName, entityState->GetName());
#endif
            UpdateDynamicAttributes(entityState);
        }
    }
}

Red::TemplateAppearance* App::GarmentOverrideModule::OnResolveAppearance(Red::EntityTemplate* aTemplate,
                                                                         Red::CName aSelector)
{
    if (aSelector == EmptyAppearanceName)
        return s_emptyAppearance.get();

    auto appearance = Raw::EntityTemplate::FindAppearance(aTemplate, aSelector);

    if (!IsUniqueAppearanceName(aSelector))
        return appearance;

    if (s_dynamicAppearance->SupportsDynamicAppearance(aTemplate))
    {
        auto selector = s_dynamicAppearance->ParseAppearance(aSelector);
        if (selector.isDynamic)
        {
            if (selector.context)
            {
                std::unique_lock _(s_mutex);
                if (auto& entityState = s_stateManager->GetEntityState(selector.context))
                {
                    SelectDynamicAppearance(entityState, selector, aTemplate, appearance);
                    return appearance;
                }
            }
            else
            {
                if (selector.name != aSelector)
                {
                    appearance = Raw::EntityTemplate::FindAppearance(aTemplate, selector.name);
                }
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

void App::GarmentOverrideModule::OnResolveDefinition(Red::AppearanceResource* aResource,
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

void App::GarmentOverrideModule::OnGetVisualTags(Red::AppearanceNameVisualTagsPreset& aPreset,
                                                 Red::ResourcePath aEntityPath, Red::CName aAppearanceName,
                                                 Red::TagList& aFinalTags)
{
    static std::shared_mutex s_autoTagsLock;
    static Core::Map<uint64_t, Red::TagList> s_autoTagsCache;
    static Core::Set<Red::ResourcePath> s_dynamicAppearanceEntities;

    if (aFinalTags.tags.size > 0 || !aAppearanceName)
        return;

    uint64_t cacheKey;

    {
        std::shared_lock _(s_autoTagsLock);

        if (s_dynamicAppearanceEntities.contains(aEntityPath))
        {
            auto baseName = s_dynamicAppearance->GetBaseAppearanceName(aAppearanceName);
            cacheKey = Red::FNV1a64(reinterpret_cast<const uint8_t*>(baseName.data()), baseName.size(),
                                    aEntityPath.hash);
        }
        else
        {
            cacheKey = Red::FNV1a64(aAppearanceName.ToString(), aEntityPath.hash);
        }

        auto cachedTagsIt = s_autoTagsCache.find(cacheKey);
        if (cachedTagsIt != s_autoTagsCache.end())
        {
            aFinalTags.Add(cachedTagsIt->second);
            return;
        }
    }

    auto loader = Red::ResourceLoader::Get();
    auto entityToken = loader->FindToken<Red::ent::EntityTemplate>(aEntityPath);

    if (!entityToken || !entityToken->IsLoaded())
        return;

    auto entityTemplate = entityToken->Get();
    auto appearanceTemplate = OnResolveAppearance(entityTemplate, aAppearanceName);

    if (!appearanceTemplate)
        return;

    auto appearancePath = appearanceTemplate->appearanceResource.path;
    auto appearanceToken = loader->FindToken<Red::appearance::AppearanceResource>(appearancePath);

    if (!appearanceToken || !appearanceToken->IsLoaded())
        return;

    auto appearanceResource = appearanceToken->Get();

    Red::Handle<Red::AppearanceDefinition> appearanceDefinition;
    Raw::AppearanceResource::FindAppearance(appearanceResource, &appearanceDefinition,
                                            appearanceTemplate->appearanceName, 0, 0);
    OnResolveDefinition(appearanceResource, &appearanceDefinition, appearanceTemplate->appearanceName, 0, 0);

    if (entityTemplate->visualTagsSchema)
    {
        aFinalTags.Add(entityTemplate->visualTagsSchema->visualTags);
    }

    if (!appearanceDefinition)
        return;

    aFinalTags.Add(appearanceDefinition->visualTags);

    std::unique_lock _(s_autoTagsLock);

    if (s_dynamicAppearance->SupportsDynamicAppearance(entityTemplate))
    {
        auto baseName = s_dynamicAppearance->GetBaseAppearanceName(aAppearanceName);
        cacheKey = Red::FNV1a64(reinterpret_cast<const uint8_t*>(baseName.data()), baseName.size(),
                                aEntityPath.hash);

        s_dynamicAppearanceEntities.insert(aEntityPath);
    }

    // fixme: figure out why TagList copy ctor is broken
    Red::TagList autoTags;
    autoTags.Add(aFinalTags);

    s_autoTagsCache.emplace(cacheKey, std::move(autoTags));
}

void App::GarmentOverrideModule::OnFindState(uintptr_t, Red::GarmentAssemblerState* aState,
                                                Red::WeakHandle<Red::Entity>& aEntityWeak)
{
    if (aState->unk00)
    {
        if (auto entity = aEntityWeak.Lock())
        {
            std::unique_lock _(s_mutex);
            s_stateManager->LinkEntityToPointer(entity, aState->unk00);
        }
    }
}

void App::GarmentOverrideModule::OnAddItem(Red::GarmentAssemblerState* aState,
                                           Red::GarmentItemAddRequest& aRequest)
{
    std::unique_lock _(s_mutex);
    if (auto& entityState = s_stateManager->FindEntityState(aState->unk00))
    {
#ifndef NDEBUG
        LogDebug("|{}| [event=AddItem entity={} item={} app={}]",
                 ModuleName, entityState->GetName(), aRequest.hash, aRequest.apperance->name.ToString());
#endif

        UpdatePartAttributes(entityState, aRequest.apperance);
        RegisterOffsetOverrides(entityState, aRequest.hash, aRequest.apperance, aRequest.offset);
        RegisterComponentOverrides(entityState, aRequest.hash, aRequest.apperance);
    }
}

void App::GarmentOverrideModule::OnAddCustomItem(Red::GarmentAssemblerState* aState,
                                                 Red::GarmentItemAddCustomRequest& aRequest)
{
    std::unique_lock _(s_mutex);
    if (auto& entityState = s_stateManager->FindEntityState(aState->unk00))
    {
#ifndef NDEBUG
        LogDebug("|{}| [event=AddCustomItem entity={} item={} app={}]",
                 ModuleName, entityState->GetName(), aRequest.hash, aRequest.apperance->name.ToString());
#endif

        UpdatePartAttributes(entityState, aRequest.apperance);
        RegisterOffsetOverrides(entityState, aRequest.hash, aRequest.apperance, aRequest.offset);
        RegisterComponentOverrides(entityState, aRequest.hash, aRequest.apperance);
        RegisterComponentOverrides(entityState, aRequest.hash, aRequest.overrides);
    }
}

void App::GarmentOverrideModule::OnChangeItem(Red::GarmentAssemblerState* aState,
                                              Red::GarmentItemChangeRequest& aRequest)
{
    std::unique_lock _(s_mutex);
    if (auto& entityState = s_stateManager->FindEntityState(aState->unk00))
    {
#ifndef NDEBUG
        LogDebug("|{}| [event=ChangeItem entity={} item={} app={}]",
                 ModuleName, entityState->GetName(), aRequest.hash, aRequest.apperance->name.ToString());
#endif

        UnregisterComponentOverrides(entityState, aRequest.hash);
        UpdatePartAttributes(entityState, aRequest.apperance);
        RegisterComponentOverrides(entityState, aRequest.hash, aRequest.apperance);
    }
}

void App::GarmentOverrideModule::OnChangeCustomItem(Red::GarmentAssemblerState* aState,
                                                    Red::GarmentItemChangeCustomRequest& aRequest)
{
    std::unique_lock _(s_mutex);
    if (auto& entityState = s_stateManager->FindEntityState(aState->unk00))
    {
#ifndef NDEBUG
        LogDebug("|{}| [event=ChangeCustomItem entity={} item={} app={}]",
                 ModuleName, entityState->GetName(), aRequest.hash, aRequest.apperance->name.ToString());
#endif

        UnregisterComponentOverrides(entityState, aRequest.hash);
        UpdatePartAttributes(entityState, aRequest.apperance);
        RegisterComponentOverrides(entityState, aRequest.hash, aRequest.apperance);
        RegisterComponentOverrides(entityState, aRequest.hash, aRequest.overrides);
    }
}

void App::GarmentOverrideModule::OnRemoveItem(uintptr_t, Red::WeakHandle<Red::Entity>& aEntityWeak,
                                              Red::GarmentItemRemoveRequest& aRequest)
{
    if (auto entity = aEntityWeak.Lock())
    {
        std::unique_lock _(s_mutex);
        if (auto& entityState = s_stateManager->GetEntityState(entity))
        {
#ifndef NDEBUG
            LogDebug("|{}| [event=RemoveItem entity={} item={}]",
                     ModuleName, entityState->GetName(), aRequest.hash);
#endif

            UnregisterOffsetOverrides(entityState, aRequest.hash);
            UnregisterComponentOverrides(entityState, aRequest.hash);
        }
    }
}

void App::GarmentOverrideModule::OnRegisterPart(uintptr_t, Red::Handle<Red::EntityTemplate>& aPart,
                                                Red::Handle<Red::ComponentsStorage>& aComponentStorage,
                                                Red::Handle<Red::AppearanceDefinition>& aAppearance)
{
#ifndef NDEBUG
    LogDebug("|{}| [event=RegisterPart part={}]", ModuleName, aPart->path.hash);
#endif

    std::unique_lock _(s_mutex);
    UpdatePartAssignments(aComponentStorage->components, aPart->path);
}

uintptr_t App::GarmentOverrideModule::OnProcessGarment(Red::SharedPtr<Red::GarmentProcessor>& aProcessor, uintptr_t a2,
                                                       uintptr_t a3, Red::GarmentProcessorParams* aParams)
{
    if (!aParams)
        return Raw::GarmentAssembler::ProcessGarment(aProcessor, a2, a3, aParams);

    std::unique_lock _(s_mutex);
    if (auto& entityState = s_stateManager->FindEntityState(aParams->entity))
    {
#ifndef NDEBUG
        LogDebug("|{}| [event=ProcessGarment entity={}]", ModuleName, entityState->GetName());
#endif

        UpdateDynamicAttributes(entityState);
    }

    auto result = Raw::GarmentAssembler::ProcessGarment(aProcessor, a2, a3, aParams);

    s_stateManager->LinkEntityToAssembler(aParams->entity, aProcessor);

    return result;
}

void App::GarmentOverrideModule::OnProcessGarmentMesh(Red::GarmentProcessor* aProcessor, uint32_t,
                                                      Red::Handle<Red::EntityTemplate>& aPartTemplate,
                                                      Red::SharedPtr<Red::ResourceToken<Red::CMesh>>& aMeshToken,
                                                      Red::Handle<Red::IComponent>& aComponent,
                                                      Red::JobGroup& aJobGroup)
{
    if (aMeshToken->IsFailed())
    {
#ifndef NDEBUG
        LogDebug("|{}| [event=ProcessGarmentMesh comp={}]", ModuleName, aComponent->name.ToString());
#endif
        std::unique_lock _(s_mutex);
        if (auto& entityState = s_stateManager->FindEntityState(aProcessor))
        {
            entityState->ApplyDynamicAppearance(aComponent, aPartTemplate->path);
            aMeshToken = ComponentWrapper(aComponent).LoadResourceToken(true);
        }
    }
}

int64_t App::GarmentOverrideModule::OnGetBaseMeshOffset(Red::Handle<Red::IComponent>& aComponent,
                                                        Red::Handle<Red::EntityTemplate>& aTemplate)
{
    auto offset = Raw::AppearanceChanger::GetBaseMeshOffset(aComponent, aTemplate);

    if (s_garmentOffsetsEnabled)
    {
        offset %= 1000;

#ifndef NDEBUG
        LogDebug("|{}| [event=GetBaseMeshOffset comp={} offset={}]", ModuleName, aComponent->name.ToString(), offset);
#endif
    }

    return offset;
}

void App::GarmentOverrideModule::OnComputeGarment(uintptr_t, Red::Handle<Red::Entity>& aEntity,
                                                  Red::DynArray<int32_t>& aOffsets,
                                                  Red::SharedPtr<Red::GarmentComputeData>& aData,
                                                  uintptr_t, uintptr_t, uintptr_t, bool)
{
    std::unique_lock _(s_mutex);
    if (auto& entityState = s_stateManager->FindEntityState(aEntity))
    {
#ifndef NDEBUG
        LogDebug("|{}| [event=ComputeGarment entity={}]",
                 ModuleName, entityState->GetName());
#endif

        UpdatePartAssignments(entityState, aData->components, aData->resources);
        UpdateDynamicAttributes(entityState);

        ApplyDynamicAppearance(entityState, aData->components);
        ApplyComponentOverrides(entityState, aData->components, false);
        ApplyOffsetOverrides(entityState, aOffsets, aData->resources);
    }
}

void App::GarmentOverrideModule::OnReassembleAppearance(Red::Entity* aEntity, uintptr_t, uintptr_t, uintptr_t,
                                                       uintptr_t, uintptr_t)
{
    std::unique_lock _(s_mutex);
    if (auto& entityState = s_stateManager->FindEntityState(aEntity))
    {
#ifndef NDEBUG
        LogDebug("|{}| [event=ReassembleAppearance entity={}]",
                 ModuleName, entityState->GetName());
#endif

        UpdateDynamicAttributes(entityState);

        ApplyDynamicAppearance(entityState);
        ApplyComponentOverrides(entityState, true);
    }
}

void App::GarmentOverrideModule::OnCreateResourcePath(Red::ResourcePath* aPath, const Red::StringView* aPathStr)
{
    if (aPathStr && s_dynamicAppearance->IsDynamicValue(*aPathStr))
    {
        std::unique_lock _(s_mutex);
        s_dynamicAppearance->RegisterPath(*aPath, *aPathStr);
    }
}

void App::GarmentOverrideModule::OnGameDetach()
{
    std::unique_lock _(s_mutex);
    s_stateManager->ClearStates();
}

void App::GarmentOverrideModule::RegisterOffsetOverrides(Core::SharedPtr<EntityState>& aEntityState, uint64_t aHash,
                                                         Red::Handle<Red::AppearanceDefinition>& aApperance,
                                                         int32_t aOffset)
{
    for (const auto& partValue : aApperance->partsValues)
    {
        aEntityState->AddOffsetOverride(aHash, partValue.resource.path, aOffset);
    }
}

void App::GarmentOverrideModule::RegisterComponentOverrides(Core::SharedPtr<EntityState>& aEntityState, uint64_t aHash,
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

void App::GarmentOverrideModule::RegisterComponentOverrides(Core::SharedPtr<EntityState>& aEntityState, uint64_t aHash,
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

void App::GarmentOverrideModule::UnregisterOffsetOverrides(Core::SharedPtr<EntityState>& aEntityState, uint64_t aHash)
{
    aEntityState->RemoveOffsetOverrides(aHash);
}

void App::GarmentOverrideModule::UnregisterComponentOverrides(Core::SharedPtr<EntityState>& aEntityState, uint64_t aHash)
{
    aEntityState->RemoveChunkMaskOverrides(aHash);
    aEntityState->RemoveAppearanceOverrides(aHash);
}

void App::GarmentOverrideModule::UpdatePartAttributes(Core::SharedPtr<EntityState>& aEntityState,
                                                      Red::Handle<Red::AppearanceDefinition>& aApperance)
{
    for (const auto& partValue : aApperance->partsValues)
    {
        aEntityState->LinkPartToAppearance(partValue.resource.path, aApperance->name);
    }
}

void App::GarmentOverrideModule::UpdatePartAssignments(Red::DynArray<Red::Handle<Red::IComponent>>& aComponents,
                                                       Red::ResourcePath aPartResource)
{
    for (auto i = 0; i < aComponents.size; ++i)
    {
        s_stateManager->LinkComponentToPart(aComponents[i], aPartResource);
    }
}

void App::GarmentOverrideModule::UpdatePartAssignments(Core::SharedPtr<App::EntityState>& aEntityState,
                                                       Red::DynArray<Red::Handle<Red::IComponent>>& aComponents,
                                                       Red::DynArray<Red::ResourcePath>& aPartResources)
{
    for (auto i = 0; i < aComponents.size; ++i)
    {
        aEntityState->LinkComponentToPart(aComponents[i], aPartResources[i]);
    }
}

bool App::GarmentOverrideModule::PrepareDynamicAppearanceName(Red::WeakHandle<Red::Entity>& aEntity,
                                                              Red::ResourceTokenPtr<Red::EntityTemplate>& aTemplateToken,
                                                              Red::CName& aAppearanceName)
{
    if (IsUniqueAppearanceName(aAppearanceName) &&
        s_dynamicAppearance->SupportsDynamicAppearance(aTemplateToken->resource))
    {
        s_dynamicAppearance->MarkDynamicAppearanceName(aAppearanceName, aEntity.instance);
        return true;
    }

    return false;
}

void App::GarmentOverrideModule::SelectDynamicAppearance(Core::SharedPtr<App::EntityState>& aEntityState,
                                                        App::DynamicAppearanceName& aSelector,
                                                        Red::EntityTemplate* aResource,
                                                        Red::TemplateAppearance*& aAppearance)
{
    aEntityState->SelectDynamicAppearance(aSelector, aResource, aAppearance);
}

void App::GarmentOverrideModule::SelectDynamicAppearance(Core::SharedPtr<EntityState>& aEntityState,
                                                         DynamicAppearanceName& aSelector,
                                                         Red::AppearanceResource* aResource,
                                                         Red::Handle<Red::AppearanceDefinition>& aDefinition)
{
    aEntityState->SelectDynamicAppearance(aSelector, aResource, aDefinition);
}

void App::GarmentOverrideModule::ApplyDynamicAppearance(Core::SharedPtr<EntityState>& aEntityState,
                                                        Red::DynArray<Red::Handle<Red::IComponent>>& aComponents)
{
    aEntityState->ToggleConditionalComponents(aComponents);

    for (auto& component : aComponents)
    {
        aEntityState->ApplyDynamicAppearance(component);
    }
}

void App::GarmentOverrideModule::ApplyDynamicAppearance(Core::SharedPtr<EntityState>& aEntityState)
{
    ApplyDynamicAppearance(aEntityState, Raw::Entity::ComponentsStorage(aEntityState->GetEntity())->components);
}

void App::GarmentOverrideModule::ApplyComponentOverrides(Core::SharedPtr<EntityState>& aEntityState, bool aForceUpdate)
{
    ApplyComponentOverrides(aEntityState, Raw::Entity::ComponentsStorage(aEntityState->GetEntity())->components,
                            aForceUpdate);
}

void App::GarmentOverrideModule::ApplyComponentOverrides(Core::SharedPtr<EntityState>& aEntityState,
                                                         Red::DynArray<Red::Handle<Red::IComponent>>& aComponents,
                                                         bool aForceUpdate)
{
    for (auto& component : aComponents)
    {
        auto isAppearnceUpdated = aEntityState->ApplyAppearanceOverride(component);
        auto isChunkMaskUpdated = aEntityState->ApplyChunkMaskOverride(component);

        if (aForceUpdate && (isAppearnceUpdated || isChunkMaskUpdated))
        {
            ComponentWrapper wrapper{component};
            if (wrapper.IsMeshComponent() && !wrapper.IsGarmentComponent())
            {
                Raw::IComponent::UpdateRenderer(component);
            }
        }
    }

#ifndef NDEBUG
    if (!aForceUpdate)
    {
        auto index = 0;
        for (auto& component : aComponents)
        {
            auto wrapper = ComponentWrapper(component);

            if (wrapper.IsMeshComponent())
            {
                LogDebug("|{}| [index={} component={} type={} enabled={} app={} chunks={:064b}]",
                         ModuleName,
                         index,
                         component->name.ToString(),
                         component->GetType()->GetName().ToString(),
                         component->isEnabled,
                         wrapper.GetAppearanceName().ToString(),
                         wrapper.GetChunkMask());
            }
            else
            {
                LogDebug("|{}| [index={} component={} type={} enabled={}]",
                         ModuleName,
                         index,
                         component->name.ToString(),
                         component->GetType()->GetName().ToString(),
                         component->isEnabled);
            }

            ++index;
        }
    }
#endif
}

void App::GarmentOverrideModule::ApplyOffsetOverrides(Core::SharedPtr<App::EntityState>& aEntityState,
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

        for (auto& offset : aOffsets)
        {
            if (offset > 0 && offset < 1000)
            {
                offset *= 100;
            }
        }
    }
    else
    {
        aOffsets.Clear();
    }
}

void App::GarmentOverrideModule::EnableGarmentOffsets()
{
    s_garmentOffsetsEnabled = true;
}

void App::GarmentOverrideModule::DisableGarmentOffsets()
{
    s_garmentOffsetsEnabled = false;
}

void App::GarmentOverrideModule::UpdateDynamicAttributes(Core::SharedPtr<EntityState>& aEntityState)
{
    aEntityState->UpdateDynamicAttributes();
}

void App::GarmentOverrideModule::UpdateDynamicAttributes()
{
    s_stateManager->UpdateDynamicAttributes();
}

bool App::GarmentOverrideModule::IsUniqueAppearanceName(Red::CName aName)
{
    return aName && aName != DefaultAppearanceName && aName != RandomAppearanceName && aName != EmptyAppearanceName;
}
