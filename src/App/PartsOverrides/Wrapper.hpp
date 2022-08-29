#pragma once

#include "stdafx.hpp"

#include <RED4ext/Scripting/Natives/Generated/ent/IComponent.hpp>

namespace App
{
enum class ComponentType
{
    Unsupported,
    MeshComponent,
    SkinnedMeshComponent,
    SkinnedClothComponent,
    MorphTargetSkinnedMeshComponent,
};

class ComponentWrapper
{
public:
    ComponentWrapper(RED4ext::Handle<RED4ext::ent::IComponent>& aComponent);

    [[nodiscard]] bool IsSupported() const;
    [[nodiscard]] uint64_t GetUniqueId();
    [[nodiscard]] uint64_t GetChunkMask() const;
    bool SetChunkMask(uint64_t aChunkMask) const;

private:
    RED4ext::Handle<RED4ext::ent::IComponent>& m_component;
    ComponentType m_type;
    uint64_t m_uniqueId;
};
}
