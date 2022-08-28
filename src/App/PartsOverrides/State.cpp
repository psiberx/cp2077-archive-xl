#include "State.hpp"
#include "Raws.hpp"
#include "Engine/Scripting/RTTILocator.hpp"

#include <RED4ext/Scripting/Natives/Generated/ent/MeshComponent.hpp>
#include <RED4ext/Scripting/Natives/Generated/ent/MorphTargetSkinnedMeshComponent.hpp>
#include <RED4ext/Scripting/Natives/Generated/ent/SkinnedClothComponent.hpp>
#include <RED4ext/Scripting/Natives/Generated/ent/SkinnedMeshComponent.hpp>
#include <utility>

namespace
{
Engine::RTTILocator s_MeshComponentRttiType{"entMeshComponent"};
Engine::RTTILocator s_SkinnedMeshComponentRttiType{"entSkinnedMeshComponent"};
Engine::RTTILocator s_SkinnedClothComponentRttiType{"entSkinnedClothComponent"};
Engine::RTTILocator s_MorphTargetSkinnedMeshComponentRttiType{"entMorphTargetSkinnedMeshComponent"};
}

Core::SharedPtr<App::EntityState>& App::StateStorage::GetEntityState(RED4ext::ent::Entity* aEntity)
{
    auto it = m_entityStates.find(aEntity);

    if (it == m_entityStates.end())
        it = m_entityStates.emplace(aEntity, Core::MakeShared<EntityState>(aEntity)).first;

    return it.value();
}

Core::SharedPtr<App::EntityState>& App::StateStorage::FindEntityState(RED4ext::ent::Entity* aEntity)
{
    static Core::SharedPtr<EntityState> s_nullState;

    auto it = m_entityStates.find(aEntity);

    if (it == m_entityStates.end())
        return s_nullState;

    return it.value();
}

void App::StateStorage::Reset()
{
    m_entityStates.clear();
}

App::EntityState::EntityState(RED4ext::ent::Entity* aEntity)
{
    m_name.append(aEntity->GetType()->GetName().ToString());
    m_name.append("/");
    m_name.append(std::to_string(reinterpret_cast<uintptr_t>(aEntity)));
}

Core::Map<RED4ext::CName, Core::SharedPtr<App::ComponentState>>& App::EntityState::GetComponentStates()
{
    return m_componentStates;
}

Core::SharedPtr<App::ComponentState>& App::EntityState::GetComponentState(const RED4ext::CName aComponentName)
{
    auto it = m_componentStates.find(aComponentName);

    if (it == m_componentStates.end())
        it = m_componentStates.emplace(aComponentName, Core::MakeShared<ComponentState>(aComponentName)).first;

    return it.value();
}

Core::SharedPtr<App::ComponentState>& App::EntityState::FindComponentState(const RED4ext::CName aComponentName)
{
    static Core::SharedPtr<ComponentState> s_nullState;

    auto it = m_componentStates.find(aComponentName);

    if (it == m_componentStates.end())
        return s_nullState;

    return it.value();
}

const char* App::EntityState::GetName() const
{
    return m_name.c_str();
}

App::ComponentWrapper::ComponentWrapper(RED4ext::Handle<RED4ext::ent::IComponent>& aComponent)
    : m_component(aComponent)
    , m_type(ComponentType::Unsupported)
{
    auto rttiType = m_component->GetType();

    if (rttiType->IsA(s_MeshComponentRttiType))
    {
        m_type = ComponentType::MeshComponent;
    }
    else if (rttiType->IsA(s_SkinnedMeshComponentRttiType))
    {
        m_type = ComponentType::SkinnedMeshComponent;
    }
    else if (rttiType->IsA(s_SkinnedClothComponentRttiType))
    {
        m_type = ComponentType::SkinnedClothComponent;
    }
    else if (rttiType->IsA(s_MorphTargetSkinnedMeshComponentRttiType))
    {
        m_type = ComponentType::MorphTargetSkinnedMeshComponent;
    }
}

bool App::ComponentWrapper::IsSupported() const
{
    return m_type != ComponentType::Unsupported;
}

uint64_t App::ComponentWrapper::GetChunkMask() const
{
    uint64_t mask = 0;

    switch (m_type)
    {
    case ComponentType::MeshComponent:
    {
        mask = m_component.GetPtr<RED4ext::ent::MeshComponent>()->chunkMask;
        break;
    }
    case ComponentType::SkinnedMeshComponent:
    {
        mask = m_component.GetPtr<RED4ext::ent::SkinnedMeshComponent>()->chunkMask;
        break;
    }
    case ComponentType::SkinnedClothComponent:
    {
        mask = m_component.GetPtr<RED4ext::ent::SkinnedClothComponent>()->chunkMask;
        break;
    }
    case ComponentType::MorphTargetSkinnedMeshComponent:
    {
        mask = m_component.GetPtr<RED4ext::ent::MorphTargetSkinnedMeshComponent>()->chunkMask;
        break;
    }
    case ComponentType::Unsupported:
        break;
    }

    return mask;
}

bool App::ComponentWrapper::SetChunkMask(uint64_t aChunkMask) const
{
    bool success = false;

    switch (m_type)
    {
    case ComponentType::MeshComponent:
    {
        m_component.GetPtr<RED4ext::ent::MeshComponent>()->chunkMask = aChunkMask;
        success = true;
        break;
    }
    case ComponentType::SkinnedMeshComponent:
    {
        m_component.GetPtr<RED4ext::ent::SkinnedMeshComponent>()->chunkMask = aChunkMask;
        success = true;
        break;
    }
    case ComponentType::SkinnedClothComponent:
    {
        m_component.GetPtr<RED4ext::ent::SkinnedClothComponent>()->chunkMask = aChunkMask;
        success = true;
        break;
    }
    case ComponentType::MorphTargetSkinnedMeshComponent:
    {
        m_component.GetPtr<RED4ext::ent::MorphTargetSkinnedMeshComponent>()->chunkMask = aChunkMask;
        success = true;
        break;
    }
    case ComponentType::Unsupported:
        break;
    }

    return success;
}

RED4ext::CName App::ComponentWrapper::GetAppearanceName() const
{
    return *(&m_component->name + 1);
}

App::ComponentState::ComponentState(RED4ext::CName aName)
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
    m_overridenChunkMasks.insert_or_assign(aHash, aChunkMask);
}

bool App::ComponentState::RemoveChunkMaskOverride(uint64_t aHash)
{
    return m_overridenChunkMasks.erase(aHash);
}

bool App::ComponentState::ApplyChunkMask(RED4ext::Handle<RED4ext::ent::IComponent>& aComponent)
{
    ComponentWrapper component(aComponent);

    if (!component.IsSupported())
        return false;

    auto appearanceName = component.GetAppearanceName();
    auto initialChunkMaskIt = m_initialChunkMasks.find(appearanceName);

    if (initialChunkMaskIt == m_initialChunkMasks.end())
        initialChunkMaskIt = m_initialChunkMasks.emplace(appearanceName, component.GetChunkMask()).first;

    auto finalChunkMask = initialChunkMaskIt->second;

    for (const auto& [_, overridenChunkMask] : m_overridenChunkMasks)
        finalChunkMask &= overridenChunkMask;

    return component.SetChunkMask(finalChunkMask);
}
