#include "State.hpp"
#include "Raws.hpp"

namespace
{
Core::SharedPtr<App::EntityState> s_nullEntityState;
Core::SharedPtr<App::ComponentState> s_nullComponentState;
}

Core::SharedPtr<App::EntityState>& App::OverrideStateManager::GetEntityState(RED4ext::ent::Entity* aEntity)
{
    if (!aEntity)
        return s_nullEntityState;

    auto it = m_entityStates.find(aEntity);

    if (it == m_entityStates.end())
        it = m_entityStates.emplace(aEntity, Core::MakeShared<EntityState>(aEntity)).first;

    return it.value();
}

Core::SharedPtr<App::EntityState>& App::OverrideStateManager::FindEntityState(RED4ext::ent::Entity* aEntity)
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

App::EntityState::EntityState(RED4ext::ent::Entity* aEntity) noexcept
    : m_entity(aEntity)
    , m_prefixResolver(ComponentPrefixResolver::Get())
    , m_tagManager(OverrideTagManager::Get())
{
    m_name.append(aEntity->GetType()->GetName().ToString());
    m_name.append("/");
    m_name.append(std::to_string(*reinterpret_cast<uint64_t*>(aEntity->unk40 + 8)));
}

Core::Map<RED4ext::CName, Core::SharedPtr<App::ComponentState>>& App::EntityState::GetComponentStates()
{
    return m_componentStates;
}

Core::SharedPtr<App::ComponentState>& App::EntityState::GetComponentState(const RED4ext::CName aComponentName)
{
    if (!aComponentName)
        return s_nullComponentState;

    auto it = m_componentStates.find(aComponentName);

    if (it == m_componentStates.end())
        it = m_componentStates.emplace(aComponentName, Core::MakeShared<ComponentState>(aComponentName)).first;

    return it.value();
}

Core::SharedPtr<App::ComponentState>& App::EntityState::FindComponentState(const RED4ext::CName aComponentName)
{
    if (!aComponentName)
        return s_nullComponentState;

    auto it = m_componentStates.find(aComponentName);

    if (it == m_componentStates.end())
        return s_nullComponentState;

    return it.value();
}

const char* App::EntityState::GetName() const
{
    return m_name.c_str();
}

RED4ext::ent::Entity* App::EntityState::GetEntity() const
{
    return m_entity;
}

void App::EntityState::AddOverride(uint64_t aHash, RED4ext::CName aComponentName, uint64_t aChunkMask)
{
    GetComponentState(aComponentName)->AddChunkMaskOverride(aHash, aChunkMask);
}

void App::EntityState::AddOverrideTag(uint64_t aHash, RED4ext::CName aTag)
{
    for (auto& [componentName, chunkMask] : m_tagManager->GetOverrides(aTag))
    {
        GetComponentState(componentName)->AddChunkMaskOverride(aHash, chunkMask);
    }
}

void App::EntityState::RemoveOverrides(uint64_t aHash)
{
    for (auto& [_, componentState] : m_componentStates)
    {
        componentState->RemoveChunkMaskOverride(aHash);
    }
}

bool App::EntityState::ApplyOverrides(RED4ext::Handle<RED4ext::ent::IComponent>& aComponent)
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

App::ComponentState::ComponentState(RED4ext::CName aName) noexcept
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
