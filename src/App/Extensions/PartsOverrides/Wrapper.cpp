#include "Wrapper.hpp"
#include "Red/Rtti/Locator.hpp"

namespace
{
Red::Rtti::ClassLocator<Red::ent::MeshComponent> s_MeshComponentRttiType;
Red::Rtti::ClassLocator<Red::ent::SkinnedMeshComponent> s_SkinnedMeshComponentRttiType;
Red::Rtti::ClassLocator<Red::ent::SkinnedClothComponent> s_SkinnedClothComponentRttiType;
Red::Rtti::ClassLocator<Red::ent::MorphTargetSkinnedMeshComponent> s_MorphTargetSkinnedMeshComponentRttiType;

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

App::ComponentWrapper::ComponentWrapper(Red::Handle<Red::ent::IComponent>& aComponent)
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
        return GetComponentChunkMask(m_component.GetPtr<Red::ent::MeshComponent>());
    case ComponentType::SkinnedMeshComponent:
        return GetComponentChunkMask(m_component.GetPtr<Red::ent::SkinnedMeshComponent>());
    case ComponentType::SkinnedClothComponent:
        return GetComponentChunkMask(m_component.GetPtr<Red::ent::SkinnedClothComponent>());
    case ComponentType::MorphTargetSkinnedMeshComponent:
        return GetComponentChunkMask(m_component.GetPtr<Red::ent::MorphTargetSkinnedMeshComponent>());
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
        return SetComponentChunkMask(m_component.GetPtr<Red::ent::MeshComponent>(), aChunkMask);
    case ComponentType::SkinnedMeshComponent:
        return SetComponentChunkMask(m_component.GetPtr<Red::ent::SkinnedMeshComponent>(), aChunkMask);
    case ComponentType::SkinnedClothComponent:
        return SetComponentChunkMask(m_component.GetPtr<Red::ent::SkinnedClothComponent>(), aChunkMask);
    case ComponentType::MorphTargetSkinnedMeshComponent:
        return SetComponentChunkMask(m_component.GetPtr<Red::ent::MorphTargetSkinnedMeshComponent>(), aChunkMask);
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
        m_uniqueId = Red::FNV1a64(m_component->unk48, sizeof(uint64_t), m_uniqueId);
        m_uniqueId = Red::FNV1a64(m_component->unk68, sizeof(uint64_t), m_uniqueId);
    }

    return m_uniqueId;
}
