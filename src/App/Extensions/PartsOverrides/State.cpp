#include "State.hpp"

namespace
{
Core::SharedPtr<App::EntityState> s_nullEntityState;
Core::SharedPtr<App::ComponentState> s_nullComponentState;
Core::SharedPtr<App::ResourceState> s_nullResourceState;
}

Core::SharedPtr<App::EntityState>& App::OverrideStateManager::GetEntityState(Red::ent::Entity* aEntity)
{
    if (!aEntity)
        return s_nullEntityState;

    auto it = m_entityStates.find(aEntity);

    if (it == m_entityStates.end())
        it = m_entityStates.emplace(aEntity, Core::MakeShared<EntityState>(aEntity)).first;

    return it.value();
}

Core::SharedPtr<App::EntityState>& App::OverrideStateManager::FindEntityState(Red::ent::Entity* aEntity)
{
    if (!aEntity)
        return s_nullEntityState;

    auto it = m_entityStates.find(aEntity);

    if (it == m_entityStates.end())
        return s_nullEntityState;

    return it.value();
}

void App::OverrideStateManager::Reset()
{
    m_entityStates.clear();
}

App::EntityState::EntityState(Red::ent::Entity* aEntity) noexcept
    : m_entity(aEntity)
    , m_prefixResolver(ComponentPrefixResolver::Get())
    , m_tagManager(OverrideTagManager::Get())
{
    m_name.append(aEntity->GetType()->GetName().ToString());
    m_name.append("/");
    m_name.append(std::to_string(*reinterpret_cast<uint64_t*>(aEntity->unk40 + 8)));
}

const char* App::EntityState::GetName() const
{
    return m_name.c_str();
}

Red::ent::Entity* App::EntityState::GetEntity() const
{
    return m_entity;
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

void App::EntityState::AddOverride(uint64_t aHash, Red::ResourcePath aResourcePath, int32_t aOffset)
{
    if (auto& resourceState = GetResourceState(aResourcePath))
    {
        resourceState->AddOffsetOverride(aHash, aOffset);
    }
}

void App::EntityState::AddOverride(uint64_t aHash, Red::CName aComponentName, uint64_t aChunkMask)
{
    if (auto& componentState = GetComponentState(aComponentName))
    {
        componentState->AddChunkMaskOverride(aHash, aChunkMask);
    }
}

void App::EntityState::AddOverride(uint64_t aHash, Red::CName aVisualTag)
{
    for (auto& [componentName, chunkMask] : m_tagManager->GetOverrides(aVisualTag))
    {
        if (auto& componentState = GetComponentState(componentName))
        {
            componentState->AddChunkMaskOverride(aHash, chunkMask);
        }
    }
}

void App::EntityState::RemoveOverrides(uint64_t aHash)
{
    for (auto& [_, componentState] : m_componentStates)
    {
        componentState->RemoveChunkMaskOverride(aHash);
    }

    for (auto& [_, resourceState] : m_resourceStates)
    {
        resourceState->RemoveOffsetOverride(aHash);
    }
}

bool App::EntityState::ApplyChunkMasks(Red::Handle<Red::ent::IComponent>& aComponent)
{
    auto& componentState = FindComponentState(aComponent->name);
    auto& prefixState = FindComponentState(m_prefixResolver->GetPrefix(aComponent->name));

    if (!componentState && !prefixState)
        return false;

    ComponentWrapper component(aComponent);

    if (!component.IsSupported())
        return false;

    uint64_t finalChunkMask = GetOriginalChunkMask(component);

    if (componentState)
        finalChunkMask &= componentState->GetOverriddenChunkMask();

    if (prefixState)
        finalChunkMask &= prefixState->GetOverriddenChunkMask();

    return component.SetChunkMask(finalChunkMask);
}

uint64_t App::EntityState::GetOriginalChunkMask(ComponentWrapper& aComponent)
{
    auto componentId = aComponent.GetUniqueId();
    auto originalChunkMaskIt = m_originalChunkMasks.find(componentId);

    if (originalChunkMaskIt == m_originalChunkMasks.end())
        originalChunkMaskIt = m_originalChunkMasks.emplace(componentId, aComponent.GetChunkMask()).first;

    return originalChunkMaskIt.value();
}

int32_t App::EntityState::GetOrderOffset(Red::ResourcePath aResourcePath)
{
    auto& resourceState = FindResourceState(aResourcePath);

    if (!resourceState)
        return 0;

    return resourceState->GetOverriddenOffset();
}

App::ComponentState::ComponentState(Red::CName aName) noexcept
    : m_name(aName.ToString())
{
}

const char* App::ComponentState::GetName() const
{
    return m_name.c_str();
}

bool App::ComponentState::IsOverridden() const
{
    return !m_overridenChunkMasks.empty();
}

void App::ComponentState::AddChunkMaskOverride(uint64_t aHash, uint64_t aChunkMask)
{
    auto it = m_overridenChunkMasks.find(aChunkMask);

    if (it == m_overridenChunkMasks.end())
        it = m_overridenChunkMasks.emplace(aHash, ~0ull).first;

    it.value() &= aChunkMask;
}

bool App::ComponentState::RemoveChunkMaskOverride(uint64_t aHash)
{
    return m_overridenChunkMasks.erase(aHash);
}

uint64_t App::ComponentState::GetOverriddenChunkMask()
{
    uint64_t finalChunkMask = ~0ull;

    for (const auto& [_, overridenChunkMask] : m_overridenChunkMasks)
        finalChunkMask &= overridenChunkMask;

    return finalChunkMask;
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
    auto it = m_overridenOffsets.find(aOffset);

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
