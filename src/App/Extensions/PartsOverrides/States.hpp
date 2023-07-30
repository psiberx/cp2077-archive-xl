#pragma once

#include "Dynamic.hpp"
#include "Prefix.hpp"
#include "Tags.hpp"
#include "Wrapper.hpp"

namespace App
{
class ComponentState
{
public:
    ComponentState(Red::CName aName) noexcept;
    ComponentState(const ComponentState&) = delete;
    ComponentState(ComponentState&&) noexcept = default;

    [[nodiscard]] const char* GetName() const;
    [[nodiscard]] bool IsOverridden() const;

    void AddHidingChunkMaskOverride(uint64_t aHash, uint64_t aChunkMask);
    void AddShowingChunkMaskOverride(uint64_t aHash, uint64_t aChunkMask);
    bool RemoveChunkMaskOverride(uint64_t aHash);
    uint64_t GetOverriddenChunkMask(uint64_t aOriginalMask);
    uint64_t GetHidingChunkMask();
    uint64_t GetShowingChunkMask();
    [[nodiscard]] bool HasOverriddenChunkMask() const;
    [[nodiscard]] bool ChangesChunkMask() const;

    void AddAppearanceOverride(uint64_t aHash, Red::CName aAppearance);
    bool RemoveAppearanceOverride(uint64_t aHash);
    Red::CName GetAppearanceOverridde();
    [[nodiscard]] bool HasAppearanceOverriddes() const;
    [[nodiscard]] bool ChangesAppearance() const;

    void LinkToPart(Red::ResourcePath aResource);
    [[nodiscard]] bool IsFromAppearancePart() const;
    [[nodiscard]] Red::ResourcePath GetAppearancePart() const;

private:
    std::string m_name;
    Red::ResourcePath m_part;
    Core::Map<uint64_t, uint64_t> m_hidingChunkMasks;
    Core::Map<uint64_t, uint64_t> m_showingChunkMasks;
    Core::Map<uint64_t, Red::CName> m_appearanceNames;
    bool m_chunkMaskChanged;
    bool m_appearanceChanged;
};

class ResourceState
{
public:
    ResourceState(Red::ResourcePath aPath) noexcept;
    ResourceState(const ResourceState&) = delete;
    ResourceState(ResourceState&&) noexcept = default;

    [[nodiscard]] const Red::ResourcePath& GetPath() const;
    [[nodiscard]] bool IsOverridden() const;

    void AddOffsetOverride(uint64_t aHash, int32_t aOffset);
    bool RemoveOffsetOverride(uint64_t aHash);
    int32_t GetOverriddenOffset();

    void LinkToAppearance(Red::CName aAppearance, bool isDynamic, Red::CName aAvariant);
    [[nodiscard]] bool IsDynamicPart() const;
    [[nodiscard]] Red::CName GetActiveVariant() const;

private:
    Red::ResourcePath m_path;
    Red::CName m_appearance;
    Red::CName m_variant;
    bool m_isDynamic;
    Core::Map<uint64_t, int32_t> m_overridenOffsets;
};

class EntityState
{
public:
    EntityState(Red::Entity* aEntity, Core::SharedPtr<DynamicAppearanceController> aDynamicAppearance) noexcept;
    EntityState(const EntityState&) = delete;
    EntityState(EntityState&&) noexcept = default;
    ~EntityState();

    [[nodiscard]] const char* GetName() const;
    [[nodiscard]] Red::Entity* GetEntity() const;

    [[nodiscard]] const Core::Map<Red::CName, Core::SharedPtr<ComponentState>>& GetComponentStates() const;
    [[nodiscard]] const Core::SharedPtr<ComponentState>& GetComponentState(Red::CName aComponentName);
    [[nodiscard]] const Core::SharedPtr<ComponentState>& FindComponentState(Red::CName aComponentName) const;

    [[nodiscard]] const Core::Map<Red::ResourcePath, Core::SharedPtr<ResourceState>>& GetResourceStates() const;
    [[nodiscard]] const Core::SharedPtr<ResourceState>& GetResourceState(Red::ResourcePath aResourcePath);
    [[nodiscard]] const Core::SharedPtr<ResourceState>& FindResourceState(Red::ResourcePath aResourcePath) const;

    bool ApplyChunkMaskOverride(Red::Handle<Red::IComponent>& aComponent);
    void AddChunkMaskOverride(uint64_t aHash, Red::CName aComponentName, uint64_t aChunkMask, bool aShow = false);
    void RemoveChunkMaskOverrides(uint64_t aHash);

    bool ApplyAppearanceOverride(Red::Handle<Red::IComponent>& aComponent);
    void AddAppearanceOverride(uint64_t aHash, Red::CName aComponentName, Red::CName aAppearance);
    void RemoveAppearanceOverrides(uint64_t aHash);

    bool ApplyOffsetOverrides(const Red::DynArray<Red::ResourcePath>& aResources, Red::DynArray<int32_t>& aOffsets) const;
    [[nodiscard]] int32_t GetOffsetOverride(Red::ResourcePath aResourcePath) const;
    void AddOffsetOverride(uint64_t aHash, Red::ResourcePath aResourcePath, int32_t aOffset);
    void RemoveOffsetOverrides(uint64_t aHash);

    void ProcessConditionalComponents(Red::DynArray<Red::Handle<Red::IComponent>>& aComponents);
    bool ApplyDynamicAppearance(Red::Handle<Red::IComponent>& aComponent);
    void LinkComponentToPart(Red::Handle<Red::IComponent>& aComponent, Red::ResourcePath aResource);
    void LinkPartToAppearance(Red::ResourcePath aResource, Red::CName aAppearance);
    void UpdateDynamicAttributes();

    void RemoveAllOverrides(uint64_t aHash);

private:
    Red::ResourcePath GetOriginalResource(ComponentWrapper& aComponent);
    Red::CName GetOriginalAppearance(ComponentWrapper& aComponent);
    uint64_t GetOriginalChunkMask(ComponentWrapper& aComponent);

    std::string m_name;
    Red::Entity* m_entity;
    Core::Map<Red::CName, Core::SharedPtr<ComponentState>> m_componentStates;
    Core::Map<Red::ResourcePath, Core::SharedPtr<ResourceState>> m_resourceStates;
    Core::Map<uint64_t, Red::ResourcePath> m_originalResources;
    Core::Map<uint64_t, Red::CName> m_originalAppearances;
    Core::Map<uint64_t, uint64_t> m_originalChunkMasks;
    Core::SharedPtr<DynamicAppearanceController> m_dynamicAppearance;
    Core::SharedPtr<ComponentPrefixResolver> m_prefixResolver;
};

class OverrideStateManager
{
public:
    explicit OverrideStateManager(Core::SharedPtr<DynamicAppearanceController> aDynamicAppearance);

    Core::SharedPtr<EntityState>& GetEntityState(Red::Entity* aEntity);
    Core::SharedPtr<EntityState>& FindEntityState(Red::Entity* aEntity);
    Core::SharedPtr<EntityState>& FindEntityState(Red::ResourcePath aPath);
    void ClearStates();

    void LinkComponentToPart(Red::Handle<Red::IComponent>& aComponent, Red::ResourcePath aResource);

private:
    Core::Map<Red::Entity*, Core::SharedPtr<EntityState>> m_entityStates;
    Core::Map<Red::ResourcePath, Core::SharedPtr<EntityState>> m_entityStatesByPath;
    Core::SharedPtr<DynamicAppearanceController> m_dynamicAppearance;
};
}
