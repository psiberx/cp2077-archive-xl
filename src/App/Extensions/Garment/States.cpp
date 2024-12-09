#include "States.hpp"
#include "Red/Entity.hpp"
#include "Red/Mesh.hpp"

namespace
{
constexpr auto DefaultAppearanceName = Red::CName("default");

Core::SharedPtr<App::EntityState> s_nullEntityState;
Core::SharedPtr<App::ComponentState> s_nullComponentState;
Core::SharedPtr<App::ResourceState> s_nullResourceState;
Core::Set<Red::CName> s_nullComponentSet;
}

App::ComponentState::ComponentState(Red::CName aName) noexcept
    : m_name(aName.ToString())
    , m_chunkMaskChanged(false)
    , m_appearanceChanged(false)
{
}

const char* App::ComponentState::GetName() const
{
    return m_name.c_str();
}

bool App::ComponentState::IsOverridden() const
{
    return HasOverriddenChunkMask() || HasAppearanceOverriddes();
}

void App::ComponentState::AddHidingChunkMaskOverride(uint64_t aHash, uint64_t aChunkMask)
{
    auto it = m_hidingChunkMasks.find(aHash);

    if (it == m_hidingChunkMasks.end())
        it = m_hidingChunkMasks.emplace(aHash, ~0ull).first;

    it.value() &= aChunkMask;

    m_chunkMaskChanged = true;
}

void App::ComponentState::AddShowingChunkMaskOverride(uint64_t aHash, uint64_t aChunkMask)
{
    auto it = m_showingChunkMasks.find(aHash);

    if (it == m_showingChunkMasks.end())
        it = m_showingChunkMasks.emplace(aHash, 0ull).first;

    it.value() |= aChunkMask;

    m_chunkMaskChanged = true;
}

bool App::ComponentState::RemoveChunkMaskOverride(uint64_t aHash)
{
    m_hidingChunkMasks.erase(aHash);
    m_showingChunkMasks.erase(aHash);

    return true;
}

uint64_t App::ComponentState::GetOverriddenChunkMask(uint64_t aOriginalMask)
{
    uint64_t finalChunkMask = aOriginalMask;

    for (const auto& [_, hidingChunkMask] : m_hidingChunkMasks)
        finalChunkMask &= hidingChunkMask;

    for (const auto& [_, showingChunkMask] : m_showingChunkMasks)
        finalChunkMask |= showingChunkMask;

    return finalChunkMask;
}

uint64_t App::ComponentState::GetHidingChunkMask()
{
    uint64_t finalChunkMask = ~0ull;

    for (const auto& [_, hidingChunkMask] : m_hidingChunkMasks)
        finalChunkMask &= hidingChunkMask;

    return finalChunkMask;
}

uint64_t App::ComponentState::GetShowingChunkMask()
{
    uint64_t finalChunkMask = 0ull;

    for (const auto& [_, showingChunkMask] : m_showingChunkMasks)
        finalChunkMask |= showingChunkMask;

    return finalChunkMask;
}

bool App::ComponentState::HasOverriddenChunkMask() const
{
    return !m_hidingChunkMasks.empty() || !m_showingChunkMasks.empty();
}

bool App::ComponentState::ChangesChunkMask() const
{
    return m_chunkMaskChanged;
}

void App::ComponentState::AddAppearanceOverride(uint64_t aHash, Red::CName aAppearance)
{
    auto it = m_appearanceNames.find(aHash);

    if (it == m_appearanceNames.end())
        it = m_appearanceNames.emplace(aHash, DefaultAppearanceName).first;

    it.value() = aAppearance;

    m_appearanceChanged = true;
}

bool App::ComponentState::RemoveAppearanceOverride(uint64_t aHash)
{
    return m_appearanceNames.erase(aHash);
}

Red::CName App::ComponentState::GetAppearanceOverridde()
{
    if (m_appearanceNames.empty())
        return DefaultAppearanceName;

    return m_appearanceNames.begin().value();
}

bool App::ComponentState::HasAppearanceOverriddes() const
{
    return !m_appearanceNames.empty();
}

bool App::ComponentState::ChangesAppearance() const
{
    return m_appearanceChanged;
}

void App::ComponentState::LinkToPart(Red::ResourcePath aResource)
{
    m_part = aResource;
}

bool App::ComponentState::IsFromAppearancePart() const
{
    return m_part;
}

Red::ResourcePath App::ComponentState::GetAppearancePart() const
{
    return m_part;
}

App::ResourceState::ResourceState(Red::ResourcePath aPath) noexcept
    : m_path(aPath)
{
}

const Red::ResourcePath& App::ResourceState::GetPath() const
{
    return m_path;
}

bool App::ResourceState::IsOverridden() const
{
    return !m_overridenOffsets.empty();
}

void App::ResourceState::AddOffsetOverride(uint64_t aHash, int32_t aOffset)
{
    auto it = m_overridenOffsets.find(aHash);

    if (it == m_overridenOffsets.end())
        it = m_overridenOffsets.emplace(aHash, 0).first;

    it.value() = aOffset;
}

bool App::ResourceState::RemoveOffsetOverride(uint64_t aHash)
{
    return m_overridenOffsets.erase(aHash);
}

int32_t App::ResourceState::GetOverriddenOffset()
{
    return !m_overridenOffsets.empty() ? m_overridenOffsets.begin()->second :  0;
}

void App::ResourceState::LinkToAppearance(DynamicAppearanceName aAppearance)
{
    m_appearance = std::move(aAppearance);
}

bool App::ResourceState::IsDynamicPart() const
{
    return m_appearance.isDynamic;
}

Red::CName App::ResourceState::GetActiveVariant() const
{
    return m_appearance.variant;
}

const App::DynamicPartList& App::ResourceState::GetActiveVariantParts() const
{
    return m_appearance.parts;
}

App::EntityState::EntityState(Red::Entity* aEntity,
                              Core::SharedPtr<DynamicAppearanceController> aDynamicAppearance) noexcept
    : m_entity(aEntity)
    , m_entityWeak(Red::AsWeakHandle(aEntity))
    , m_dynamicAppearance(std::move(aDynamicAppearance))
    , m_prefixResolver(ComponentPrefixResolver::Get())
{
    m_name.append(aEntity->GetType()->GetName().ToString());
    m_name.append("/");
    m_name.append(std::to_string(aEntity->entityID.hash));
}

App::EntityState::~EntityState()
{
    m_dynamicAppearance->RemoveState(m_entity);
}

const char* App::EntityState::GetName() const
{
    return m_name.c_str();
}

Red::Entity* App::EntityState::GetEntity() const
{
    return m_entity;
}

bool App::EntityState::IsValid() const
{
    return !m_entityWeak.Expired();
}

const Core::Map<Red::CName, Core::SharedPtr<App::ComponentState>>& App::EntityState::GetComponentStates() const
{
    return m_componentStates;
}

const Core::SharedPtr<App::ComponentState>& App::EntityState::GetComponentState(const Red::CName aComponentName)
{
    if (!aComponentName)
        return s_nullComponentState;

    auto it = m_componentStates.find(aComponentName);

    if (it == m_componentStates.end())
        it = m_componentStates.emplace(aComponentName, Core::MakeShared<ComponentState>(aComponentName)).first;

    return it.value();
}

const Core::SharedPtr<App::ComponentState>& App::EntityState::FindComponentState(const Red::CName aComponentName) const
{
    if (!aComponentName)
        return s_nullComponentState;

    const auto& it = m_componentStates.find(aComponentName);

    if (it == m_componentStates.end())
        return s_nullComponentState;

    return it.value();
}

const Core::Map<Red::ResourcePath, Core::SharedPtr<App::ResourceState>>& App::EntityState::GetResourceStates() const
{
    return m_resourceStates;
}

const Core::SharedPtr<App::ResourceState>& App::EntityState::GetResourceState(Red::ResourcePath aResourcePath)
{
    if (!aResourcePath)
        return s_nullResourceState;

    auto it = m_resourceStates.find(aResourcePath);

    if (it == m_resourceStates.end())
        it = m_resourceStates.emplace(aResourcePath, Core::MakeShared<ResourceState>(aResourcePath)).first;

    return it.value();
}

const Core::SharedPtr<App::ResourceState>& App::EntityState::FindResourceState(Red::ResourcePath aResourcePath) const
{
    if (!aResourcePath)
        return s_nullResourceState;

    const auto& it = m_resourceStates.find(aResourcePath);

    if (it == m_resourceStates.end())
        return s_nullResourceState;

    return it.value();
}

const Core::Set<Red::CName>& App::EntityState::GetPartComponentNames(Red::ResourcePath aResourcePath) const
{
    if (!aResourcePath)
        return s_nullComponentSet;

    const auto& it = m_partComponents.find(aResourcePath);

    if (it == m_partComponents.end())
        return s_nullComponentSet;

    return it.value();
}

void App::EntityState::AddChunkMaskOverride(uint64_t aHash, Red::CName aComponentName, uint64_t aChunkMask, bool aShow)
{
    if (auto& componentState = GetComponentState(aComponentName))
    {
        if (aShow)
            componentState->AddShowingChunkMaskOverride(aHash, aChunkMask);
        else
            componentState->AddHidingChunkMaskOverride(aHash, aChunkMask);
    }
}

void App::EntityState::RemoveChunkMaskOverrides(uint64_t aHash)
{
    for (auto& [_, componentState] : m_componentStates)
    {
        componentState->RemoveChunkMaskOverride(aHash);
    }
}

void App::EntityState::AddAppearanceOverride(uint64_t aHash, Red::CName aComponentName, Red::CName aAppearance)
{
    if (aAppearance && aAppearance != DefaultAppearanceName)
    {
        if (auto& componentState = GetComponentState(aComponentName))
        {
            componentState->AddAppearanceOverride(aHash, aAppearance);
        }
    }
}

void App::EntityState::RemoveAppearanceOverrides(uint64_t aHash)
{
    for (auto& [_, componentState] : m_componentStates)
    {
        componentState->RemoveAppearanceOverride(aHash);
    }
}

void App::EntityState::AddOffsetOverride(uint64_t aHash, Red::ResourcePath aResourcePath, int32_t aOffset)
{
    if (auto& resourceState = GetResourceState(aResourcePath))
    {
        resourceState->AddOffsetOverride(aHash, aOffset);
    }
}

void App::EntityState::RemoveOffsetOverrides(uint64_t aHash)
{
    for (auto& [_, resourceState] : m_resourceStates)
    {
        resourceState->RemoveOffsetOverride(aHash);
    }
}

void App::EntityState::RemoveAllOverrides(uint64_t aHash)
{
    RemoveChunkMaskOverrides(aHash);
    RemoveAppearanceOverrides(aHash);
    RemoveOffsetOverrides(aHash);
}

void App::EntityState::LinkComponentToPart(Red::Handle<Red::IComponent>& aComponent, Red::ResourcePath aResource)
{
    auto componentRef = m_dynamicAppearance->ParseReference(aComponent->name);
    if (auto& componentState = GetComponentState(componentRef.name))
    {
        componentState->LinkToPart(aResource);

        m_partComponents[aResource].insert(aComponent->name);
    }
}

void App::EntityState::LinkPartToAppearance(Red::ResourcePath aResource, DynamicAppearanceName& aAppearance)
{
    if (auto& resourceState = GetResourceState(aResource))
    {
        resourceState->LinkToAppearance(aAppearance);
    }
}

void App::EntityState::LinkPartToAppearance(Red::ResourcePath aResource, Red::CName aAppearance)
{
    if (auto& resourceState = GetResourceState(aResource))
    {
        resourceState->LinkToAppearance(m_dynamicAppearance->ParseAppearance(aAppearance));
    }
}

void App::EntityState::UpdateDynamicAttributes()
{
    m_dynamicAppearance->UpdateState(m_entity);
}

void App::EntityState::UpdateDynamicAttributes(Red::TweakDBID aEquippedItemID)
{
    m_dynamicAppearance->UpdateState(m_entity, aEquippedItemID);
}

bool App::EntityState::SelectDynamicAppearance(App::DynamicAppearanceName& aSelector, Red::EntityTemplate* aResource,
                                               Red::TemplateAppearance*& aAppearance)
{
    if (!aSelector.isDynamic)
        return false;

    if (!aAppearance)
    {
        auto baseAppearance = Raw::EntityTemplate::FindAppearance(aResource, aSelector.name);

        if (!baseAppearance)
            return false;

        const auto baseAppearanceIndex = baseAppearance - aResource->appearances.Begin();

        aResource->appearances.EmplaceBack();
        aAppearance = aResource->appearances.End() - 1;

        baseAppearance = &aResource->appearances[baseAppearanceIndex];

        aAppearance->appearanceResource = baseAppearance->appearanceResource;
        aAppearance->appearanceName = baseAppearance->appearanceName
                                         ? baseAppearance->appearanceName
                                         : baseAppearance->name;

        m_dynamicAppearance->MarkDynamicAppearanceName(aAppearance->appearanceName, aSelector);
    }

    aAppearance->name = aAppearance->appearanceName;

    return true;
}

bool App::EntityState::SelectDynamicAppearance(DynamicAppearanceName& aSelector, Red::AppearanceResource* aResource,
                                               Red::Handle<Red::AppearanceDefinition>& aDefinition)
{
    int8_t maxWeight = -1;
    int8_t numCandidates = 0;
    Red::Handle<Red::AppearanceDefinition> match;

    for (auto& definition : aResource->appearances)
    {
        auto appearanceRef = m_dynamicAppearance->ParseReference(definition->name);
        if (appearanceRef.name == aSelector.name)
        {
            ++numCandidates;
            if (!appearanceRef.isConditional)
            {
                if (maxWeight < 0)
                {
                    maxWeight = 0;
                    match = definition;
                }
            }
            else if (m_dynamicAppearance->MatchReference(appearanceRef, m_entity, aSelector.variant))
            {
                if (maxWeight < appearanceRef.weight)
                {
                    maxWeight = appearanceRef.weight;
                    match = definition;
                }
            }
        }
    }

    if (match)
    {
        if (match->name != aSelector.value)
        {
            auto definition = Red::MakeHandle<Red::AppearanceDefinition>();
            for (const auto prop : Red::GetClass<Red::AppearanceDefinition>()->props)
            {
                prop->SetValue(definition.instance, prop->GetValuePtr<void>(match.instance));
            }

            definition->name = aSelector.value;

            {
                std::unique_lock _(*Raw::AppearanceResource::Mutex(aResource));
                aResource->appearances.PushBack(definition);
            }

            aDefinition = std::move(definition);
        }
        else
        {
            aDefinition = std::move(match);
        }

        for (const auto& partValue : aDefinition->partsValues)
        {
            LinkPartToAppearance(partValue.resource.path, aSelector);
        }

        return true;
    }

    if (aSelector.isDynamic && numCandidates > 0)
    {
        aDefinition = Red::MakeHandle<Red::AppearanceDefinition>();
        aDefinition->name = aSelector.value;

        return true;
    }

    return false;
}

void App::EntityState::ToggleConditionalComponents(Red::DynArray<Red::Handle<Red::IComponent>>& aComponents)
{
    Core::Map<Red::CName, WeightedComponentGroup> groups;

    for (auto& component : aComponents)
    {
        auto componentRef = m_dynamicAppearance->ParseReference(component->name);

        if (!componentRef.isConditional)
        {
            auto& group = groups[componentRef.name];
            group.matches.push_back({0, component});
            continue;
        }

        const auto& componentState = FindComponentState(componentRef.name);

        if (!componentState || !componentState->IsFromAppearancePart())
            continue;

        const auto& resourceState = FindResourceState(componentState->GetAppearancePart());

        if (!resourceState /*|| !resourceState->IsDynamicPart()*/)
            continue;

        const auto activeVariant = resourceState->GetActiveVariant();

        component->isEnabled = m_dynamicAppearance->MatchReference(componentRef, m_entity, activeVariant);

        if (component->isEnabled)
        {
            auto& group = groups[componentRef.name];
            group.matches.push_back({componentRef.weight, component});

            if (group.maxWeight < componentRef.weight)
            {
                group.maxWeight = componentRef.weight;
            }
        }
    }

    for (auto& [_, group] : groups)
    {
        if (group.maxWeight > 0 && group.matches.size() > 1)
        {
            bool forceDisable = false;
            for (auto& match : group.matches)
            {
                if (match.weight < group.maxWeight || forceDisable)
                {
                    match.component->isEnabled = false;
                }
                else
                {
                    forceDisable = true;
                }
            }
        }
    }
}

bool App::EntityState::ApplyDynamicAppearance(const Red::Handle<Red::IComponent>& aComponent)
{
    if (!aComponent->isEnabled)
        return false;

    const auto componentRef = m_dynamicAppearance->ParseReference(aComponent->name);
    const auto& componentState = FindComponentState(componentRef.name);

    if (!componentState || !componentState->IsFromAppearancePart())
        return false;

    const auto& resourceState = FindResourceState(componentState->GetAppearancePart());

    if (!resourceState /*|| !resourceState->IsDynamicPart()*/)
        return false;

    return ApplyDynamicAppearance(aComponent, resourceState->GetActiveVariantParts(),
                                  !componentState->ChangesAppearance());
}

bool App::EntityState::ApplyDynamicAppearance(const Red::Handle<Red::IComponent>& aComponent,
                                              Red::ResourcePath aResource)
{
    const auto& resourceState = FindResourceState(aResource);

    if (!resourceState /*|| !resourceState->IsDynamicPart()*/)
        return false;

    return ApplyDynamicAppearance(aComponent, resourceState->GetActiveVariantParts(), true);
}

bool App::EntityState::ApplyDynamicAppearance(const Red::Handle<Red::IComponent>& aComponent,
                                              const DynamicPartList& aVariant,
                                              bool aSetAppearance)
{
    ComponentWrapper componentWrapper(aComponent);
    auto updated = false;

    if (componentWrapper.IsMeshComponent())
    {
        const auto originalResource = GetOriginalResource(componentWrapper);
        const auto finalResource = m_dynamicAppearance->ResolvePath(m_entity, aVariant, originalResource);

        if (finalResource != originalResource && finalResource != componentWrapper.GetResourcePath())
        {
            componentWrapper.SetResourcePath(finalResource);
            updated = true;
        }

        if (aSetAppearance)
        {
            const auto originalAppearance = GetOriginalAppearance(componentWrapper);
            const auto finalAppearance = m_dynamicAppearance->ResolveName(m_entity, aVariant, originalAppearance);

            if (finalAppearance != originalAppearance && finalAppearance != componentWrapper.GetAppearanceName())
            {
                componentWrapper.SetAppearanceName(finalAppearance);
                updated = true;
            }
        }
    }

    return updated;
}

bool App::EntityState::ApplyAppearanceOverride(const Red::Handle<Red::IComponent>& aComponent)
{
    if (!aComponent->isEnabled)
        return false;

    ComponentWrapper component(aComponent);

    if (!component.IsMeshComponent())
        return false;

    auto componentRef = m_dynamicAppearance->ParseReference(aComponent->name);
    auto& componentState = FindComponentState(componentRef.name);
    auto& prefixState = FindComponentState(m_prefixResolver->GetPrefix(aComponent->name));

    Red::CName finalAppearance;

    if (componentState && componentState->HasAppearanceOverriddes())
    {
        finalAppearance = componentState->GetAppearanceOverridde();
    }
    else if (prefixState && prefixState->HasAppearanceOverriddes())
    {
        finalAppearance = prefixState->GetAppearanceOverridde();
    }
    else if ((componentState && componentState->ChangesAppearance()) ||
             (prefixState && prefixState->ChangesAppearance()))
    {
        finalAppearance = GetOriginalAppearance(component);
    }

    if (!finalAppearance)
        return false;

    const auto& resourceState = FindResourceState(componentState->GetAppearancePart());
    const auto& activeVariant = resourceState->GetActiveVariantParts();
    finalAppearance = m_dynamicAppearance->ResolveName(m_entity, activeVariant, finalAppearance);

    return finalAppearance && component.SetAppearanceName(finalAppearance) && component.RefreshAppearance();
}

bool App::EntityState::ApplyChunkMaskOverride(const Red::Handle<Red::IComponent>& aComponent)
{
    if (!aComponent->isEnabled)
        return false;

    ComponentWrapper component(aComponent);

    if (!component.IsMeshComponent())
        return false;

    auto componentRef = m_dynamicAppearance->ParseReference(aComponent->name);
    auto& componentState = FindComponentState(componentRef.name);
    auto& prefixState = FindComponentState(m_prefixResolver->GetPrefix(aComponent->name));

    if ((!componentState || !componentState->ChangesChunkMask())
        && (!prefixState || !prefixState->ChangesChunkMask()))
        return false;

    uint64_t finalChunkMask = GetOriginalChunkMask(component);

    if (componentState)
        finalChunkMask |= componentState->GetShowingChunkMask();

    if (prefixState)
        finalChunkMask |= prefixState->GetShowingChunkMask();

    if (componentState)
        finalChunkMask &= componentState->GetHidingChunkMask();

    if (prefixState)
        finalChunkMask &= prefixState->GetHidingChunkMask();

    return component.SetChunkMask(finalChunkMask);
}

bool App::EntityState::ApplyOffsetOverrides(const Red::DynArray<Red::ResourcePath>& aResources,
                                            Red::DynArray<int32_t>& aOffsets) const
{
    aOffsets.Clear();

    for (auto& resourcePath : aResources)
    {
        aOffsets.PushBack(GetOffsetOverride(resourcePath));
    }

    return true;
}

int32_t App::EntityState::GetOffsetOverride(Red::ResourcePath aResourcePath) const
{
    auto& resourceState = FindResourceState(aResourcePath);

    if (!resourceState)
        return 0;

    return resourceState->GetOverriddenOffset();
}

Red::ResourcePath App::EntityState::GetOriginalResource(ComponentWrapper& aComponent)
{
    auto componentId = aComponent.GetUniqueId();
    auto it = m_originalResources.find(componentId);

    if (it == m_originalResources.end())
        it = m_originalResources.emplace(componentId, aComponent.GetResourcePath()).first;

    return it.value();
}

Red::CName App::EntityState::GetOriginalAppearance(ComponentWrapper& aComponent)
{
    auto componentId = aComponent.GetUniqueId();
    auto it = m_originalAppearances.find(componentId);

    if (it == m_originalAppearances.end())
        it = m_originalAppearances.emplace(componentId, aComponent.GetAppearanceName()).first;

    return it.value();
}

uint64_t App::EntityState::GetOriginalChunkMask(ComponentWrapper& aComponent)
{
    auto componentId = aComponent.GetUniqueId();
    auto it = m_originalChunkMasks.find(componentId);

    if (it == m_originalChunkMasks.end())
        it = m_originalChunkMasks.emplace(componentId, aComponent.GetChunkMask()).first;

    return it.value();
}

App::OverrideStateManager::OverrideStateManager(Core::SharedPtr<DynamicAppearanceController> aDynamicAppearance)
    : m_dynamicAppearance(std::move(aDynamicAppearance))
{
}

Core::SharedPtr<App::EntityState>& App::OverrideStateManager::GetFirstEntityState()
{
    if (m_entityStates.empty())
        return s_nullEntityState;

    return m_entityStates.begin().value();
}

Core::SharedPtr<App::EntityState>& App::OverrideStateManager::GetEntityState(uint64_t aContext)
{
    return GetEntityState(reinterpret_cast<Red::Entity*>(aContext));
}

Core::SharedPtr<App::EntityState>& App::OverrideStateManager::GetEntityState(Red::Entity* aEntity)
{
    if (!aEntity)
        return s_nullEntityState;

    auto it = m_entityStates.find(aEntity);

    if (it == m_entityStates.end())
    {
        it = m_entityStates.emplace(aEntity, Core::MakeShared<EntityState>(aEntity, m_dynamicAppearance)).first;

        auto dynamicPath = std::to_string(aEntity->templatePath.hash) + "_0.app";
        for (char i = '0'; i < '3'; ++i)
        {
            *(dynamicPath.end() - 5) = i;
            m_entityStatesByPath.emplace(dynamicPath.c_str(), it.value());
        }
    }

    return it.value();
}

Core::SharedPtr<App::EntityState>& App::OverrideStateManager::FindEntityState(Red::Entity* aEntity)
{
    if (!aEntity)
        return s_nullEntityState;

    auto it = m_entityStates.find(aEntity);

    if (it == m_entityStates.end())
        return s_nullEntityState;

    return it.value();
}

Core::SharedPtr<App::EntityState>& App::OverrideStateManager::FindEntityState(Red::ResourcePath aPath)
{
    if (!aPath)
        return s_nullEntityState;

    auto it = m_entityStatesByPath.find(aPath);

    if (it == m_entityStatesByPath.end())
        return s_nullEntityState;

    return it.value();
}

Core::SharedPtr<App::EntityState>& App::OverrideStateManager::FindEntityState(Red::GarmentProcessingContext* aProcessor)
{
    if (!aProcessor)
        return s_nullEntityState;

    auto it = m_entityStatesByProcessor.find(aProcessor);

    if (it == m_entityStatesByProcessor.end())
        return s_nullEntityState;

    return it.value();
}

Core::SharedPtr<App::EntityState>& App::OverrideStateManager::FindEntityState(uintptr_t aPointer)
{
    if (!aPointer)
        return s_nullEntityState;

    auto it = m_entityStatesByPointer.find(aPointer);

    if (it == m_entityStatesByPointer.end())
        return s_nullEntityState;

    return it.value();
}

void App::OverrideStateManager::ClearStates()
{
    m_entityStates.clear();
    m_entityStatesByPath.clear();
    m_entityStatesByProcessor.clear();
    m_entityStatesByPointer.clear();
}

void App::OverrideStateManager::LinkEntityToAssembler(Red::Entity* aEntity, Red::GarmentProcessingContext* aProcessor)
{
    auto it = m_entityStates.find(aEntity);

    if (it != m_entityStates.end())
    {
        m_entityStatesByProcessor[aProcessor] = it.value();
    }
}

void App::OverrideStateManager::LinkEntityToPointer(Red::Entity* aEntity, uintptr_t aPointer)
{
    auto it = m_entityStates.find(aEntity);

    if (it != m_entityStates.end())
    {
        m_entityStatesByPointer[aPointer] = it.value();
    }
    else
    {
        m_entityStatesByPointer[aPointer] = GetEntityState(aEntity);
    }
}

void App::OverrideStateManager::LinkComponentToPart(Red::Handle<Red::IComponent>& aComponent,
                                                    Red::ResourcePath aResource)
{
    Core::Vector<Red::Entity*> expired;

    for (auto& [entityKey, entityState] : m_entityStates)
    {
        if (entityState->IsValid())
        {
            entityState->LinkComponentToPart(aComponent, aResource);
        }
        else
        {
            expired.push_back(entityKey);
        }
    }

    for (const auto& entityKey : expired)
    {
        m_entityStates.erase(entityKey);
    }
}

void App::OverrideStateManager::UpdateDynamicAttributes()
{
    for (auto& [_, entityState] : m_entityStates)
    {
        if (entityState->IsValid())
        {
            entityState->UpdateDynamicAttributes();
        }
    }
}
