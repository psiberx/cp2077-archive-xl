#pragma once

namespace App
{
enum class ComponentType
{
    Unsupported,
    MeshComponent,
    SkinnedMeshComponent,
    MorphTargetSkinnedMeshComponent,
};

class ComponentWrapper
{
public:
    explicit ComponentWrapper(Red::IComponent* aComponent);
    explicit ComponentWrapper(const Red::Handle<Red::IComponent>& aComponent);

    [[nodiscard]] bool IsMeshComponent() const;
    [[nodiscard]] uint64_t GetUniqueId();

    [[nodiscard]] bool IsEnabled() const;
    bool SetEnabled(bool isEnabled) const;

    [[nodiscard]] Red::ResourcePath GetResource() const;
    bool SetResource(Red::ResourcePath aPath) const;
    [[nodiscard]] Red::SharedPtr<Red::ResourceToken<Red::CMesh>> LoadResource(bool aWait = false) const;

    [[nodiscard]] Red::CName GetAppearanceName() const;
    bool SetAppearanceName(Red::CName aAppearance) const;
    bool LoadAppearance() const;

    [[nodiscard]] uint64_t GetChunkMask() const;
    bool SetChunkMask(uint64_t aChunkMask) const;

private:
    Red::IComponent* m_component;
    ComponentType m_componentType;
    uint64_t m_uniqueId;
};
}
