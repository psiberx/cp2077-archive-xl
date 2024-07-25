#include "Module.hpp"
#include "App/Extensions/Customization/Module.hpp"
#include "App/Shared/ResourcePathRegistry.hpp"
#include "Core/Facades/Container.hpp"
#include "Red/Entity.hpp"
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

constexpr auto AggregatedHeadAppearanceName = Red::CName("AppearanceName1");
constexpr auto DummyAppearancePartPath = Red::ResourcePath(R"(#non_existing_part.ent)");

std::shared_mutex s_dynamicTagsLock;
Core::Map<uint64_t, Core::Vector<Red::CName>> s_dynamicTagsCache;
}

std::string_view App::GarmentOverrideModule::GetName()
{
    return ModuleName;
}

bool App::GarmentOverrideModule::Load()
{
    HookBefore<Raw::ItemFactoryRequest::LoadAppearance>(&OnLoadAppearanceResource).OrThrow();
    HookBefore<Raw::ItemFactoryAppearanceChangeRequest::LoadAppearance>(&OnChangeAppearanceResource).OrThrow();
    Hook<Raw::EntityTemplate::FindAppearance>(&OnResolveAppearance).OrThrow();
    HookAfter<Raw::AppearanceResource::FindAppearance>(&OnResolveDefinition).OrThrow();
    HookAfter<Raw::AppearanceNameVisualTagsPreset::GetVisualTags>(&OnGetVisualTags).OrThrow();
    HookAfter<Raw::GarmentAssembler::FindState>(&OnFindState).OrThrow();
    HookBefore<Raw::GarmentAssemblerState::AddItem>(&OnAddItem).OrThrow();
    HookBefore<Raw::GarmentAssemblerState::AddCustomItem>(&OnAddCustomItem).OrThrow();
    HookBefore<Raw::GarmentAssemblerState::ChangeItem>(&OnChangeItem).OrThrow();
    HookBefore<Raw::GarmentAssemblerState::ChangeCustomItem>(&OnChangeCustomItem).OrThrow();
    HookBefore<Raw::GarmentAssembler::RemoveItem>(&OnRemoveItem).OrThrow();
    Hook<Raw::GarmentAssembler::ProcessGarment>(&OnProcessGarment).OrThrow();
    HookBefore<Raw::GarmentAssembler::ProcessSkinnedMesh>(&OnProcessGarmentMesh).OrThrow();
    HookBefore<Raw::GarmentAssembler::ProcessMorphedMesh>(&OnProcessGarmentMesh).OrThrow();
    HookBefore<Raw::GarmentAssembler::OnGameDetach>(&OnGameDetach).OrThrow();
    HookBefore<Raw::AppearanceChanger::RegisterPart>(&OnRegisterPart).OrThrow();
    //Hook<Raw::AppearanceChanger::GetBaseMeshOffset>(&OnGetBaseMeshOffset).OrThrow();
    HookBefore<Raw::AppearanceChanger::ComputePlayerGarment>(&OnComputeGarment).OrThrow();
    HookBefore<Raw::Entity::ReassembleAppearance>(&OnReassembleAppearance).OrThrow();

    s_emptyAppearance = Core::MakeUnique<Red::TemplateAppearance>();
    s_emptyAppearance->name = EmptyAppearanceName;
    s_emptyAppearance->appearanceName = DefaultAppearanceName;
    s_emptyAppearance->appearanceResource = EmptyAppearancePath;

    s_dynamicAppearance = Core::MakeShared<DynamicAppearanceController>(Core::Resolve<ResourcePathRegistry>());
    s_stateManager = Core::MakeUnique<OverrideStateManager>(s_dynamicAppearance);
    s_tagManager = Core::MakeShared<OverrideTagManager>();

    return true;
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
    // Unhook<Raw::AppearanceChanger::GetBaseMeshOffset>();
    Unhook<Raw::AppearanceChanger::ComputePlayerGarment>();
    Unhook<Raw::Entity::ReassembleAppearance>();

    s_dynamicAppearance.reset();
    s_stateManager.reset();
    s_tagManager.reset();

    return true;
}

void App::GarmentOverrideModule::Configure()
{
    std::unique_lock _(s_mutex);
    for (const auto& unit : m_configs)
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
                std::unique_lock _(s_mutex);
                if (auto& entityState = s_stateManager->GetFirstEntityState())
                {
                    SelectDynamicAppearance(entityState, selector, aTemplate, appearance);
                    return appearance;
                }
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
        if (selector.isDynamic)
        {
            if (selector.context)
            {
                std::unique_lock _(s_mutex);
                if (auto& entityState = s_stateManager->GetEntityState(selector.context))
                {
                    SelectDynamicAppearance(entityState, selector, aResource, *aDefinition);
                }
            }
            else
            {
                std::unique_lock _(s_mutex);
                if (auto& entityState = s_stateManager->GetFirstEntityState())
                {
                    SelectDynamicAppearance(entityState, selector, aResource, *aDefinition);
                }
            }
        }
        else
        {
            CustomizationModule::FixCustomizationAppearance(aResource, aDefinition, aSelector);
        }
    }
}

void App::GarmentOverrideModule::OnGetVisualTags(Red::AppearanceNameVisualTagsPreset& aPreset,
                                                 Red::ResourcePath aEntityPath, Red::CName aAppearanceName,
                                                 Red::TagList& aFinalTags)
{
    if (!aAppearanceName) // aFinalTags.tags.size > 0
        return;

    auto cacheKey = Red::FNV1a64(aAppearanceName.ToString(), aEntityPath.hash);

    {
        std::shared_lock _(s_dynamicTagsLock);
        const auto& cachedTagsIt = s_dynamicTagsCache.find(cacheKey);
        if (cachedTagsIt != s_dynamicTagsCache.end())
        {
            for (const auto& tag : cachedTagsIt.value())
            {
                aFinalTags.Add(tag);
            }
            return;
        }
    }

    Core::Vector<Red::CName> dynamicTags;

    auto loader = Red::ResourceLoader::Get();
    auto entityToken = loader->FindToken<Red::ent::EntityTemplate>(aEntityPath);

    if (!entityToken)
    {
        entityToken = loader->LoadAsync<Red::ent::EntityTemplate>(aEntityPath);
        Red::WaitForResource(entityToken, std::chrono::milliseconds(500));
    }

    auto entityTemplate = entityToken->Get();

    if (!entityTemplate)
    {
        for (const auto& tag : dynamicTags)
        {
            aFinalTags.Add(tag);
        }

        std::unique_lock _(s_dynamicTagsLock);
        s_dynamicTagsCache.insert_or_assign(cacheKey, std::move(dynamicTags));
        return;
    }

    if (entityTemplate->visualTagsSchema)
    {
        for (const auto& tag : entityTemplate->visualTagsSchema->visualTags.tags)
        {
            dynamicTags.push_back(tag);
        }
    }

    auto appearanceTemplate = OnResolveAppearance(entityTemplate, aAppearanceName);

    if (!appearanceTemplate)
    {
        for (const auto& tag : dynamicTags)
        {
            aFinalTags.Add(tag);
        }

        std::unique_lock _(s_dynamicTagsLock);
        s_dynamicTagsCache.insert_or_assign(cacheKey, std::move(dynamicTags));
        return;
    }

    auto appearancePath = appearanceTemplate->appearanceResource.path;
    auto appearanceToken = loader->FindToken<Red::appearance::AppearanceResource>(appearancePath);

    if (!appearanceToken)
    {
        appearanceToken = loader->LoadAsync<Red::appearance::AppearanceResource>(appearancePath);
        Red::WaitForResource(appearanceToken, std::chrono::milliseconds(500));
    }

    auto appearanceResource = appearanceToken->Get();

    if (!appearanceResource)
    {
        for (const auto& tag : dynamicTags)
        {
            aFinalTags.Add(tag);
        }

        std::unique_lock _(s_dynamicTagsLock);
        s_dynamicTagsCache.insert_or_assign(cacheKey, std::move(dynamicTags));
        return;
    }

    Red::Handle<Red::AppearanceDefinition> appearanceDefinition;
    Raw::AppearanceResource::FindAppearance(appearanceResource, &appearanceDefinition,
                                            appearanceTemplate->appearanceName, 0, 0);
    OnResolveDefinition(appearanceResource, &appearanceDefinition, appearanceTemplate->appearanceName, 0, 0);

    if (!appearanceDefinition)
    {
        for (const auto& tag : dynamicTags)
        {
            aFinalTags.Add(tag);
        }

        std::unique_lock _(s_dynamicTagsLock);
        s_dynamicTagsCache.insert_or_assign(cacheKey, std::move(dynamicTags));
        return;
    }

    for (const auto& tag : appearanceDefinition->visualTags.tags)
    {
        dynamicTags.push_back(tag);
    }

    for (const auto& tag : dynamicTags)
    {
        aFinalTags.Add(tag);
    }

    std::unique_lock _(s_dynamicTagsLock);
    s_dynamicTagsCache.insert_or_assign(cacheKey, std::move(dynamicTags));
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

// int32_t App::GarmentOverrideModule::OnGetBaseMeshOffset(Red::Handle<Red::IComponent>& aComponent,
//                                                         Red::Handle<Red::EntityTemplate>& aTemplate)
// {
//     auto offset = Raw::AppearanceChanger::GetBaseMeshOffset(aComponent, aTemplate);
//
//     if (s_garmentOffsetsEnabled)
//     {
//         if (offset >= -1000)
//         {
//             offset %= 1000;
//
// #ifndef NDEBUG
//             LogDebug("|{}| [event=GetBaseMeshOffset comp={} offset={}]", ModuleName, aComponent->name.ToString(), offset);
// #endif
//         }
//     }
//
//     return offset;
// }

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

        PatchHeadGarmentOverrides(entityState, aData->definition);
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
        s_dynamicAppearance->SupportsDynamicAppearance(aTemplateToken->resource) &&
        s_dynamicAppearance->IsDynamicAppearanceName(aAppearanceName))
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
    ApplyDynamicAppearance(aEntityState, aEntityState->GetEntity()->components);
}

void App::GarmentOverrideModule::ApplyComponentOverrides(Core::SharedPtr<EntityState>& aEntityState, bool aForceUpdate)
{
    ApplyComponentOverrides(aEntityState, aEntityState->GetEntity()->components, aForceUpdate);
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
            if (wrapper.IsMeshComponent() /*&& !wrapper.IsGarmentComponent()*/)
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

void App::GarmentOverrideModule::PatchHeadGarmentOverrides(Core::SharedPtr<EntityState>& aEntityState,
                                                           Red::Handle<Red::AppearanceDefinition>& aDefinition)
{
    if (aDefinition->name != AggregatedHeadAppearanceName)
        return;

    for (const auto& part : aDefinition->partsValues)
    {
        for (const auto& componentName : aEntityState->GetPartComponentNames(part.resource.path))
        {
            bool hasComponentOverride = false;

            for (auto& partOverride : aDefinition->partsOverrides)
            {
                if (partOverride.partResource.path
                    && partOverride.partResource.path != part.resource.path
                    && partOverride.partResource.path != DummyAppearancePartPath)
                    continue;

                for (auto& componentOverride : partOverride.componentsOverrides)
                {
                    if (componentOverride.componentName == componentName)
                    {
                        hasComponentOverride = true;
                        break;
                    }
                }

                if (hasComponentOverride)
                {
                    break;
                }
            }

            if (!hasComponentOverride)
            {
                Red::appearancePartComponentOverrides componentOverride;
                componentOverride.componentName = componentName;
                componentOverride.meshAppearance = DefaultAppearanceName;
                componentOverride.chunkMask = std::numeric_limits<uint64_t>::max();
                componentOverride.acceptDismemberment = true;

                Red::appearanceAppearancePartOverrides partOverride;
                partOverride.partResource = DummyAppearancePartPath;
                partOverride.componentsOverrides.PushBack(std::move(componentOverride));

                aDefinition->partsOverrides.PushBack(std::move(partOverride));
            }
        }
    }
}

Core::SharedPtr<App::DynamicAppearanceController>& App::GarmentOverrideModule::GetDynamicAppearanceController()
{
    return s_dynamicAppearance;
}

Core::SharedPtr<App::OverrideTagManager>& App::GarmentOverrideModule::GetTagManager()
{
    return s_tagManager;
}
