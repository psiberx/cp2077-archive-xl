#pragma once

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
    ComponentWrapper(Red::Handle<Red::ent::IComponent>& aComponent);

    [[nodiscard]] bool IsSupported() const;
    [[nodiscard]] uint64_t GetUniqueId();
    [[nodiscard]] uint64_t GetChunkMask() const;
    bool SetChunkMask(uint64_t aChunkMask) const;

private:
    Red::Handle<Red::ent::IComponent>& m_component;
    ComponentType m_type;
    uint64_t m_uniqueId;
};
}