#pragma once

namespace App
{
enum class ComponentType
{
    Unsupported,
    MeshComponent,
    SkinnedMeshComponent,
    GarmentSkinnedMeshComponent,
    MorphTargetSkinnedMeshComponent,
};

class ComponentWrapper
{
public:
    explicit ComponentWrapper(Red::IComponent* aComponent);
    explicit ComponentWrapper(const Red::Handle<Red::IComponent>& aComponent);

    [[nodiscard]] bool IsMeshComponent() const;
    [[nodiscard]] bool IsGarmentComponent() const;
    [[nodiscard]] uint64_t GetUniqueId();

    [[nodiscard]] bool IsEnabled() const;
    bool SetEnabled(bool isEnabled) const;

    [[nodiscard]] Red::ResourcePath GetResourcePath() const;
    bool SetResourcePath(Red::ResourcePath aPath) const;

    bool LoadResource(bool aRefresh, bool aWait) const;
    [[nodiscard]] Red::SharedPtr<Red::ResourceToken<>> LoadResourceToken(bool aWait = false) const;
    [[nodiscard]] Red::SharedPtr<Red::ResourceToken<Red::CMesh>> LoadMeshToken(bool aWait = false) const;

    [[nodiscard]] Red::CName GetAppearanceName() const;
    bool SetAppearanceName(Red::CName aAppearance) const;
    bool LoadAppearance() const;

    [[nodiscard]] uint64_t GetChunkMask() const;
    bool SetChunkMask(uint64_t aChunkMask) const;

    bool RefreshAppearance() const;

private:
    Red::IComponent* m_component;
    ComponentType m_componentType;
    uint64_t m_uniqueId;
};
}
