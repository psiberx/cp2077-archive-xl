#include "States.hpp"

namespace
{
constexpr auto DefaultAppearanceName = Red::CName("default");
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
    return !m_hidingChunkMasks.empty() || !m_showingChunkMasks.empty() || !m_appearanceNames.empty();
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

uint64_t App::ComponentState::GetOverriddenChunkMask(uint64_t aOriginalMask) const
{
    uint64_t finalChunkMask = aOriginalMask;

    for (const auto& hidingChunkMask : m_hidingChunkMasks | std::views::values)
    {
        finalChunkMask &= hidingChunkMask;
    }

    for (const auto& showingChunkMask : m_showingChunkMasks | std::views::values)
    {
        finalChunkMask |= showingChunkMask;
    }

    return finalChunkMask;
}

uint64_t App::ComponentState::GetHidingChunkMask() const
{
    uint64_t finalChunkMask = ~0ull;

    for (const auto& hidingChunkMask : m_hidingChunkMasks | std::views::values)
    {
        finalChunkMask &= hidingChunkMask;
    }

    return finalChunkMask;
}

uint64_t App::ComponentState::GetShowingChunkMask() const
{
    uint64_t finalChunkMask = 0ull;

    for (const auto& showingChunkMask : m_showingChunkMasks | std::views::values)
    {
        finalChunkMask |= showingChunkMask;
    }

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

Red::CName App::ComponentState::GetAppearanceOverridde() const
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

int32_t App::ResourceState::GetOverriddenOffset() const
{
    return !m_overridenOffsets.empty() ? m_overridenOffsets.begin()->second :  0;
}

void App::ResourceState::LinkToAppearance(DynamicAppearanceName aAppearance)
{
    m_appearance = std::move(aAppearance);
}

const App::DynamicAppearanceName& App::ResourceState::GetActiveAppearance() const
{
    return m_appearance;
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

const App::DynamicTagList& App::ResourceState::GetActiveVariantOverrides() const
{
    return m_appearance.overrides;
}

App::EntityState::EntityState(const Red::Handle<Red::Entity>& aEntity,
                              Core::SharedPtr<DynamicAppearanceController> aDynamicAppearance) noexcept
    : m_entityWeak(aEntity)
    , m_dynamicAppearance(std::move(aDynamicAppearance))
    , m_prefixResolver(ComponentPrefixResolver::Get())
{
    m_name.append(aEntity->GetType()->GetName().ToString());
    m_name.append("/");
    m_name.append(std::to_string(aEntity->entityID.hash));
}

App::EntityState::~EntityState()
{
    m_dynamicAppearance->RemoveState(m_entityWeak);
}

const char* App::EntityState::GetName() const
{
    return m_name.c_str();
}

Red::Handle<Red::Entity> App::EntityState::GetEntity() const
{
    return m_entityWeak.Lock();
}

bool App::EntityState::IsValid() const
{
    return !m_entityWeak.Expired();
}

Core::SharedPtr<App::ComponentState> App::EntityState::GetComponentState(const Red::CName aComponentName)
{
    if (!aComponentName)
    {
        return {};
    }

    auto it = m_componentStates.find(aComponentName);

    if (it == m_componentStates.end())
    {
        it = m_componentStates.emplace(aComponentName, Core::MakeShared<ComponentState>(aComponentName)).first;
    }

    return it.value();
}

Core::SharedPtr<App::ComponentState> App::EntityState::FindComponentState(const Red::CName aComponentName) const
{
    if (!aComponentName)
    {
        return {};
    }

    const auto it = m_componentStates.find(aComponentName);

    if (it == m_componentStates.end())
    {
        return {};
    }

    return it.value();
}

Core::SharedPtr<App::ResourceState> App::EntityState::GetResourceState(Red::ResourcePath aResourcePath)
{
    if (!aResourcePath)
    {
        return {};
    }

    auto it = m_resourceStates.find(aResourcePath);

    if (it == m_resourceStates.end())
    {
        it = m_resourceStates.emplace(aResourcePath, Core::MakeShared<ResourceState>(aResourcePath)).first;
    }

    return it.value();
}

Core::SharedPtr<App::ResourceState> App::EntityState::FindResourceState(Red::ResourcePath aResourcePath) const
{
    if (!aResourcePath)
    {
        return {};
    }

    const auto it = m_resourceStates.find(aResourcePath);

    if (it == m_resourceStates.end())
    {
        return {};
    }

    return it.value();
}

Core::Set<Red::CName> App::EntityState::GetPartComponentNames(Red::ResourcePath aResourcePath) const
{
    if (!aResourcePath)
    {
        return {};
    }

    std::shared_lock _(m_stateLock);

    const auto it = m_partComponents.find(aResourcePath);

    if (it == m_partComponents.end())
    {
        return {};
    }

    return it.value();
}

void App::EntityState::AddChunkMaskOverride(uint64_t aHash, Red::CName aComponentName, uint64_t aChunkMask, bool aShow)
{
    std::unique_lock _(m_stateLock);

    auto componentState = GetComponentState(aComponentName);

    if (componentState)
    {
        if (aShow)
        {
            componentState->AddShowingChunkMaskOverride(aHash, aChunkMask);
        }
        else
        {
            componentState->AddHidingChunkMaskOverride(aHash, aChunkMask);
        }
    }
}

void App::EntityState::RemoveChunkMaskOverrides(uint64_t aHash)
{
    std::unique_lock _(m_stateLock);

    for (const auto& componentState : m_componentStates | std::views::values)
    {
        componentState->RemoveChunkMaskOverride(aHash);
    }
}

void App::EntityState::AddAppearanceOverride(uint64_t aHash, Red::CName aComponentName, Red::CName aAppearance)
{
    if (aAppearance && aAppearance != DefaultAppearanceName)
    {
        std::unique_lock _(m_stateLock);

        auto componentState = GetComponentState(aComponentName);

        if (componentState)
        {
            componentState->AddAppearanceOverride(aHash, aAppearance);
        }
    }
}

void App::EntityState::RemoveAppearanceOverrides(uint64_t aHash)
{
    std::unique_lock _(m_stateLock);

    for (const auto& componentState : m_componentStates | std::views::values)
    {
        componentState->RemoveAppearanceOverride(aHash);
    }
}

void App::EntityState::AddOffsetOverride(uint64_t aHash, Red::ResourcePath aResourcePath, int32_t aOffset)
{
    std::unique_lock _(m_stateLock);

    auto resourceState = GetResourceState(aResourcePath);

    if (resourceState)
    {
        resourceState->AddOffsetOverride(aHash, aOffset);
    }
}

void App::EntityState::RemoveOffsetOverrides(uint64_t aHash)
{
    std::unique_lock _(m_stateLock);

    for (const auto& resourceState : m_resourceStates | std::views::values)
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
    {
        std::shared_lock _(m_stateLock);

        if (!FindResourceState(aResource))
        {
            return;
        }
    }

    auto componentRef = m_dynamicAppearance->ParseReference(aComponent->name);

    std::unique_lock _(m_stateLock);

    auto componentState = GetComponentState(componentRef.name);

    if (componentState)
    {
        componentState->LinkToPart(aResource);
        m_partComponents[aResource].insert(aComponent->name);
    }
}

void App::EntityState::LinkPartToAppearance(Red::ResourcePath aResource, DynamicAppearanceName& aAppearance)
{
    std::unique_lock _(m_stateLock);

    auto resourceState = GetResourceState(aResource);

    if (resourceState)
    {
        resourceState->LinkToAppearance(aAppearance);
    }
}

void App::EntityState::LinkPartToAppearance(Red::ResourcePath aResource, Red::CName aAppearance)
{
    auto appearance = m_dynamicAppearance->ParseAppearance(aAppearance);

    std::unique_lock _(m_stateLock);

    auto resourceState = GetResourceState(aResource);

    if (resourceState)
    {
        resourceState->LinkToAppearance(std::move(appearance));
    }
}

void App::EntityState::UpdateDynamicAttributes()
{
    if (const auto entity = m_entityWeak.Lock())
    {
        m_dynamicAppearance->UpdateState(entity);
    }
}

void App::EntityState::UpdateDynamicAttributes(Red::TweakDBID aEquippedItemID)
{
    if (const auto entity = m_entityWeak.Lock())
    {
        m_dynamicAppearance->UpdateState(entity, aEquippedItemID);
    }
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

        const auto baseAppearanceIndex = baseAppearance - &aResource->appearances.Front();

        aResource->appearances.EmplaceBack();
        aAppearance = &aResource->appearances.Back();

        baseAppearance = &aResource->appearances[baseAppearanceIndex];

        aAppearance->name = aSelector.value;
        aAppearance->appearanceResource = baseAppearance->appearanceResource;
        aAppearance->appearanceName = baseAppearance->appearanceName
                                         ? baseAppearance->appearanceName
                                         : baseAppearance->name;

        m_dynamicAppearance->MarkDynamicAppearanceName(aAppearance->appearanceName, aSelector);
    }

    return true;
}

bool App::EntityState::SelectDynamicAppearance(DynamicAppearanceName& aSelector, Red::AppearanceResource* aResource,
                                               Red::Handle<Red::AppearanceDefinition>& aDefinition)
{
    const auto entity = m_entityWeak.Lock();

    if (!entity)
    {
        return false;
    }

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
            else if (m_dynamicAppearance->MatchReference(appearanceRef, entity, aSelector))
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

void App::EntityState::ToggleConditionalComponents(const Red::DynArray<Red::Handle<Red::IComponent>>& aComponents)
{
    const auto entity = m_entityWeak.Lock();

    if (!entity)
    {
        return;
    }

    Core::Map<Red::CName, WeightedComponentGroup> groups;

    for (const auto& component : aComponents)
    {
        auto componentRef = m_dynamicAppearance->ParseReference(component->name);

        if (!componentRef.isConditional)
        {
            auto& group = groups[componentRef.name];
            group.matches.push_back({0, component});
            continue;
        }

        DynamicAppearanceName activeAppearance;

        {
            std::shared_lock _(m_stateLock);

            const auto componentState = FindComponentState(componentRef.name);

            if (!componentState || !componentState->IsFromAppearancePart())
            {
                continue;
            }

            const auto resourceState = FindResourceState(componentState->GetAppearancePart());

            if (!resourceState /*|| !resourceState->IsDynamicPart()*/)
            {
                continue;
            }

            activeAppearance = resourceState->GetActiveAppearance();
        }

        component->isEnabled = m_dynamicAppearance->MatchReference(componentRef, entity, activeAppearance);

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

    for (const auto& group : groups | std::views::values)
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

void App::EntityState::ApplyDynamicAppearance(const Red::DynArray<Red::Handle<Red::IComponent>>& aComponents)
{
    const auto entity = m_entityWeak.Lock();

    if (!entity)
    {
        return;
    }

    for (const auto& component : aComponents)
    {
        ComponentWrapper componentWrapper(component);

        if (!componentWrapper.IsMeshComponent())
        {
            continue;
        }

        const auto componentRef = m_dynamicAppearance->ParseReference(component->name);

        DynamicPartList variantParts;
        bool setAppearance = false;

        {
            std::shared_lock _(m_stateLock);

            const auto componentState = FindComponentState(componentRef.name);

            if (!componentState || !componentState->IsFromAppearancePart())
            {
                continue;
            }

            const auto resourceState = FindResourceState(componentState->GetAppearancePart());

            if (!resourceState /*|| !resourceState->IsDynamicPart()*/)
            {
                continue;
            }

            variantParts = resourceState->GetActiveVariantParts();
            setAppearance = !componentState->ChangesAppearance();
        }

        ApplyDynamicAppearance(componentWrapper, entity, variantParts, setAppearance);
    }
}

bool App::EntityState::ApplyDynamicAppearance(const Red::Handle<Red::IComponent>& aComponent,
                                              Red::ResourcePath aResource)
{
    const auto entity = m_entityWeak.Lock();

    if (!entity)
    {
        return false;
    }

    ComponentWrapper componentWrapper(aComponent);

    if (!componentWrapper.IsMeshComponent())
    {
        return false;
    }

    DynamicPartList variantParts;

    {
        std::shared_lock _(m_stateLock);

        const auto resourceState = FindResourceState(aResource);

        if (!resourceState /*|| !resourceState->IsDynamicPart()*/)
        {
            return false;
        }

        variantParts = resourceState->GetActiveVariantParts();
    }

    return ApplyDynamicAppearance(componentWrapper, entity, variantParts, true);
}

bool App::EntityState::ApplyDynamicAppearance(const ComponentWrapper& aComponentWrapper,
                                              const Red::Handle<Red::Entity>& aEntity,
                                              const DynamicPartList& aVariant,
                                              bool aSetAppearance)
{
    auto updated = false;

    const auto originalResource = GetOriginalResource(aComponentWrapper);
    const auto finalResource = m_dynamicAppearance->ResolvePath(aEntity, aVariant, originalResource);

    if (finalResource != originalResource && finalResource != aComponentWrapper.GetResourcePath())
    {
        aComponentWrapper.SetResourcePath(finalResource);
        updated = true;
    }

    if (aSetAppearance)
    {
        const auto originalAppearance = GetOriginalAppearance(aComponentWrapper);
        const auto finalAppearance = m_dynamicAppearance->ResolveName(aEntity, aVariant, originalAppearance);

        if (finalAppearance != originalAppearance && finalAppearance != aComponentWrapper.GetAppearanceName())
        {
            aComponentWrapper.SetAppearanceName(finalAppearance);
            updated = true;
        }
    }

    return updated;
}

bool App::EntityState::ApplyAppearanceOverride(const Red::Handle<Red::IComponent>& aComponent)
{
    if (!aComponent->isEnabled)
    {
        return false;
    }

    ComponentWrapper component(aComponent);

    if (!component.IsMeshComponent())
    {
        return false;
    }

    const auto entity = m_entityWeak.Lock();

    if (!entity)
    {
        return false;
    }

    const auto componentRef = m_dynamicAppearance->ParseReference(aComponent->name);

    Red::CName finalAppearance;
    DynamicPartList activeVariant;

    {
        std::shared_lock _(m_stateLock);

        auto componentState = FindComponentState(componentRef.name);

        if (!componentState)
        {
            const auto prefixName = m_prefixResolver->GetPrefix(aComponent->name);
            componentState = FindComponentState(prefixName);

            if (!componentState)
            {
                return false;
            }
        }

        if (componentState->HasAppearanceOverriddes())
        {
            finalAppearance = componentState->GetAppearanceOverridde();
        }
        else if (componentState->ChangesAppearance())
        {
            finalAppearance = GetOriginalAppearance(component);
        }

        if (!finalAppearance)
        {
            return false;
        }

        const auto resourceState = FindResourceState(componentState->GetAppearancePart());

        if (!resourceState)
        {
            return false;
        }

        activeVariant = resourceState->GetActiveVariantParts();
    }

    finalAppearance = m_dynamicAppearance->ResolveName(entity, activeVariant, finalAppearance);

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
    auto prefixName = m_prefixResolver->GetPrefix(aComponent->name);

    uint64_t finalChunkMask = 0;

    {
        std::shared_lock _(m_stateLock);

        auto componentState = FindComponentState(componentRef.name);
        auto prefixState = FindComponentState(prefixName);

        if ((!componentState || !componentState->ChangesChunkMask())
            && (!prefixState || !prefixState->ChangesChunkMask()))
        {
            return false;
        }

        finalChunkMask = GetOriginalChunkMask(component);

        if (componentState)
        {
            finalChunkMask |= componentState->GetShowingChunkMask();
        }

        if (prefixState)
        {
            finalChunkMask |= prefixState->GetShowingChunkMask();
        }

        if (componentState)
        {
            finalChunkMask &= componentState->GetHidingChunkMask();
        }

        if (prefixState)
        {
            finalChunkMask &= prefixState->GetHidingChunkMask();
        }
    }

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
    std::shared_lock _(m_stateLock);

    auto resourceState = FindResourceState(aResourcePath);

    if (!resourceState)
    {
        return 0;
    }

    return resourceState->GetOverriddenOffset();
}

Red::ResourcePath App::EntityState::GetOriginalResource(const ComponentWrapper& aComponent)
{
    const auto componentId = aComponent.GetUniqueId();

    {
        std::shared_lock _(m_originalLock);

        const auto it = m_originalResources.find(componentId);
        if (it != m_originalResources.end())
        {
            return it.value();
        }
    }

    const auto value = aComponent.GetResourcePath();

    {
        std::shared_lock _(m_originalLock);

        m_originalResources.emplace(componentId, value);
    }

    return value;
}

Red::CName App::EntityState::GetOriginalAppearance(const ComponentWrapper& aComponent)
{
    const auto componentId = aComponent.GetUniqueId();

    {
        std::shared_lock _(m_originalLock);

        const auto it = m_originalAppearances.find(componentId);
        if (it != m_originalAppearances.end())
        {
            return it.value();
        }
    }

    const auto value = aComponent.GetAppearanceName();

    {
        std::shared_lock _(m_originalLock);

        m_originalAppearances.emplace(componentId, value);
    }

    return value;
}

uint64_t App::EntityState::GetOriginalChunkMask(const ComponentWrapper& aComponent)
{
    const auto componentId = aComponent.GetUniqueId();

    {
        std::shared_lock _(m_originalLock);

        const auto it = m_originalChunkMasks.find(componentId);
        if (it != m_originalChunkMasks.end())
        {
            return it.value();
        }
    }

    const auto value = aComponent.GetChunkMask();

    {
        std::shared_lock _(m_originalLock);

        m_originalChunkMasks.emplace(componentId, value);
    }

    return value;
}

App::OverrideStateManager::OverrideStateManager(Core::SharedPtr<DynamicAppearanceController> aDynamicAppearance)
    : m_dynamicAppearance(std::move(aDynamicAppearance))
{
}

Core::SharedPtr<App::EntityState> App::OverrideStateManager::GetFirstEntityState()
{
    std::shared_lock _(m_lock);

    if (m_entityStates.empty())
        return {};

    return m_entityStates.begin().value();
}

Core::SharedPtr<App::EntityState> App::OverrideStateManager::GetEntityState(uint64_t aContext)
{
    return GetEntityState(reinterpret_cast<Red::Entity*>(aContext));
}

Core::SharedPtr<App::EntityState> App::OverrideStateManager::GetEntityState(Red::Entity* aEntity)
{
    if (!aEntity)
    {
        return {};
    }

    {
        std::shared_lock _(m_lock);

        auto it = m_entityStates.find(aEntity);
        if (it != m_entityStates.end())
        {
            return it.value();
        }
    }

    const auto entity = Red::AsHandle(aEntity);

    if (!entity)
    {
        return {};
    }

    auto state = Core::MakeShared<EntityState>(entity, m_dynamicAppearance);

    {
        std::unique_lock _(m_lock);

        m_entityStates.emplace(aEntity, state);

        auto dynamicPath = std::to_string(aEntity->templatePath.hash) + "_0.app";
        for (char i = '0'; i < '3'; ++i)
        {
            *(dynamicPath.end() - 5) = i;
            m_entityStatesByPath.emplace(dynamicPath.c_str(), state);
        }
    }

    return state;
}

Core::SharedPtr<App::EntityState> App::OverrideStateManager::FindEntityState(Red::Entity* aEntity)
{
    if (!aEntity)
        return {};

    std::shared_lock _(m_lock);

    auto it = m_entityStates.find(aEntity);

    if (it == m_entityStates.end())
        return {};

    return it.value();
}

Core::SharedPtr<App::EntityState> App::OverrideStateManager::FindEntityState(Red::ResourcePath aPath)
{
    if (!aPath)
        return {};

    std::shared_lock _(m_lock);

    auto it = m_entityStatesByPath.find(aPath);

    if (it == m_entityStatesByPath.end())
        return {};

    return it.value();
}

Core::SharedPtr<App::EntityState> App::OverrideStateManager::FindEntityState(Red::GarmentProcessingContext* aProcessor)
{
    if (!aProcessor)
        return {};

    std::shared_lock _(m_lock);

    auto it = m_entityStatesByProcessor.find(aProcessor);

    if (it == m_entityStatesByProcessor.end())
        return {};

    return it.value();
}

Core::SharedPtr<App::EntityState> App::OverrideStateManager::FindEntityState(uintptr_t aPointer)
{
    if (!aPointer)
        return {};

    std::shared_lock _(m_lock);

    auto it = m_entityStatesByPointer.find(aPointer);

    if (it == m_entityStatesByPointer.end())
        return {};

    return it.value();
}

void App::OverrideStateManager::ClearStates()
{
    std::unique_lock _(m_lock);

    m_entityStates.clear();
    m_entityStatesByPath.clear();
    m_entityStatesByProcessor.clear();
    m_entityStatesByPointer.clear();
}

void App::OverrideStateManager::LinkEntityToAssembler(Red::Entity* aEntity, Red::GarmentProcessingContext* aProcessor)
{
    std::unique_lock _(m_lock);

    auto it = m_entityStates.find(aEntity);

    if (it != m_entityStates.end())
    {
        m_entityStatesByProcessor[aProcessor] = it.value();
    }
}

void App::OverrideStateManager::LinkEntityToPointer(Red::Entity* aEntity, uintptr_t aPointer)
{
    auto entityState = GetEntityState(aEntity);

    std::unique_lock _(m_lock);

    m_entityStatesByPointer[aPointer] = entityState;
}

void App::OverrideStateManager::LinkComponentToPart(Red::Handle<Red::IComponent>& aComponent,
                                                    Red::ResourcePath aResource)
{
    Core::Vector<Red::Entity*> expired;

    {
        std::shared_lock _(m_lock);

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
    }

    if (!expired.empty())
    {
        std::unique_lock _(m_lock);

        for (const auto& entityKey : expired)
        {
            m_entityStates.erase(entityKey);
        }
    }
}

void App::OverrideStateManager::UpdateDynamicAttributes()
{
    std::shared_lock _(m_lock);

    for (const auto& entityState : m_entityStates | std::views::values)
    {
        if (entityState->IsValid())
        {
            entityState->UpdateDynamicAttributes();
        }
    }
}
