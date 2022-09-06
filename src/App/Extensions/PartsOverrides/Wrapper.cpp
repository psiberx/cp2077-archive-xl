#include "Wrapper.hpp"
#include "Engine/Scripting/RTTILocator.hpp"

#include <RED4ext/Hashing/FNV1a.hpp>
#include <RED4ext/Scripting/Natives/Generated/ent/MeshComponent.hpp>
#include <RED4ext/Scripting/Natives/Generated/ent/MorphTargetSkinnedMeshComponent.hpp>
#include <RED4ext/Scripting/Natives/Generated/ent/SkinnedClothComponent.hpp>
#include <RED4ext/Scripting/Natives/Generated/ent/SkinnedMeshComponent.hpp>

namespace
{
Engine::RTTILocator s_MeshComponentRttiType{"entMeshComponent"};
Engine::RTTILocator s_SkinnedMeshComponentRttiType{"entSkinnedMeshComponent"};
Engine::RTTILocator s_SkinnedClothComponentRttiType{"entSkinnedClothComponent"};
Engine::RTTILocator s_MorphTargetSkinnedMeshComponentRttiType{"entMorphTargetSkinnedMeshComponent"};

template<class T>
inline uint64_t GetComponentChunkMask(T* aComponent)
{
    return aComponent->chunkMask;
}

template<class T>
inline bool SetComponentChunkMask(T* aComponent, uint64_t aChunkMask)
{
    aComponent->chunkMask = aChunkMask;
    return true;
}
}

App::ComponentWrapper::ComponentWrapper(RED4ext::Handle<RED4ext::ent::IComponent>& aComponent)
    : m_component(aComponent)
    , m_type(ComponentType::Unsupported)
    , m_uniqueId(0)
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
    switch (m_type)
    {
    case ComponentType::MeshComponent:
        return GetComponentChunkMask(m_component.GetPtr<RED4ext::ent::MeshComponent>());
    case ComponentType::SkinnedMeshComponent:
        return GetComponentChunkMask(m_component.GetPtr<RED4ext::ent::SkinnedMeshComponent>());
    case ComponentType::SkinnedClothComponent:
        return GetComponentChunkMask(m_component.GetPtr<RED4ext::ent::SkinnedClothComponent>());
    case ComponentType::MorphTargetSkinnedMeshComponent:
        return GetComponentChunkMask(m_component.GetPtr<RED4ext::ent::MorphTargetSkinnedMeshComponent>());
    case ComponentType::Unsupported:
        break;
    }

    return 0;
}

bool App::ComponentWrapper::SetChunkMask(uint64_t aChunkMask) const
{
    switch (m_type)
    {
    case ComponentType::MeshComponent:
        return SetComponentChunkMask(m_component.GetPtr<RED4ext::ent::MeshComponent>(), aChunkMask);
    case ComponentType::SkinnedMeshComponent:
        return SetComponentChunkMask(m_component.GetPtr<RED4ext::ent::SkinnedMeshComponent>(), aChunkMask);
    case ComponentType::SkinnedClothComponent:
        return SetComponentChunkMask(m_component.GetPtr<RED4ext::ent::SkinnedClothComponent>(), aChunkMask);
    case ComponentType::MorphTargetSkinnedMeshComponent:
        return SetComponentChunkMask(m_component.GetPtr<RED4ext::ent::MorphTargetSkinnedMeshComponent>(), aChunkMask);
    case ComponentType::Unsupported:
        break;
    }

    return false;
}

uint64_t App::ComponentWrapper::GetUniqueId()
{
    if (!m_uniqueId)
    {
        // Component Name + Appearance Name + Appearance Resource Path
        m_uniqueId = m_component->name;
        m_uniqueId = RED4ext::FNV1a64(m_component->unk48, sizeof(uint64_t), m_uniqueId);
        m_uniqueId = RED4ext::FNV1a64(m_component->unk68, sizeof(uint64_t), m_uniqueId);
    }

    return m_uniqueId;
}
