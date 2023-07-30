#pragma once

namespace App
{
enum class ComponentType
{
    Unknown,
    MeshComponent,
    SkinnedMeshComponent,
    SkinnedClothComponent,
    MorphTargetSkinnedMeshComponent,
};

class ComponentWrapper
{
public:
    ComponentWrapper(Red::Handle<Red::IComponent>& aComponent);

    [[nodiscard]] bool IsMeshComponent() const;
    [[nodiscard]] uint64_t GetUniqueId();

    [[nodiscard]] bool IsEnabled() const;
    bool SetEnabled(bool isEnabled) const;

    [[nodiscard]] Red::ResourcePath GetResource() const;
    bool SetResource(Red::ResourcePath aPath) const;
    bool LoadResource() const;

    [[nodiscard]] Red::CName GetAppearance() const;
    bool SetAppearance(Red::CName aAppearance) const;
    bool LoadAppearance() const;

    [[nodiscard]] uint64_t GetChunkMask() const;
    bool SetChunkMask(uint64_t aChunkMask) const;

private:
    Red::Handle<Red::IComponent>& m_component;
    ComponentType m_type;
    uint64_t m_uniqueId;
};
}
