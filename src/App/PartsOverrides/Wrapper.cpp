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
    if (GetChunkMask() == aChunkMask)
        return true;

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
