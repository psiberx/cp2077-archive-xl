#pragma once

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

    void AddChunkMaskOverride(uint64_t aHash, uint64_t aChunkMask);
    bool RemoveChunkMaskOverride(uint64_t aHash);
    uint64_t GetOverriddenChunkMask();

private:
    std::string m_name;
    Core::Map<uint64_t, uint64_t> m_overridenChunkMasks;
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

private:
    Red::ResourcePath m_path;
    Core::Map<uint64_t, int32_t> m_overridenOffsets;
};

class EntityState
{
public:
    EntityState(Red::ent::Entity* aEntity) noexcept;
    EntityState(const EntityState&) = delete;
    EntityState(EntityState&&) noexcept = default;

    [[nodiscard]] const char* GetName() const;
    [[nodiscard]] Red::ent::Entity* GetEntity() const;

    [[nodiscard]] const Core::Map<Red::CName, Core::SharedPtr<ComponentState>>& GetComponentStates() const;
    [[nodiscard]] const Core::SharedPtr<ComponentState>& GetComponentState(Red::CName aComponentName);
    [[nodiscard]] const Core::SharedPtr<ComponentState>& FindComponentState(Red::CName aComponentName) const;

    [[nodiscard]] const Core::Map<Red::ResourcePath, Core::SharedPtr<ResourceState>>& GetResourceStates() const;
    [[nodiscard]] const Core::SharedPtr<ResourceState>& GetResourceState(Red::ResourcePath aResourcePath);
    [[nodiscard]] const Core::SharedPtr<ResourceState>& FindResourceState(Red::ResourcePath aResourcePath) const;

    void AddOverride(uint64_t aHash, Red::ResourcePath aResourcePath, int32_t aOffset);
    void AddOverride(uint64_t aHash, Red::CName aComponentName, uint64_t aChunkMask);
    void AddOverride(uint64_t aHash, Red::CName aVisualTag);
    void RemoveOverrides(uint64_t aHash);

    bool ApplyChunkMasks(Red::Handle<Red::ent::IComponent>& aComponent);
    int32_t GetOrderOffset(Red::ResourcePath aResourcePath);

private:
    uint64_t GetOriginalChunkMask(ComponentWrapper& aComponent);

    std::string m_name;
    Red::ent::Entity* m_entity;
    Core::Map<Red::CName, Core::SharedPtr<ComponentState>> m_componentStates;
    Core::Map<Red::ResourcePath, Core::SharedPtr<ResourceState>> m_resourceStates;
    Core::Map<uint64_t, uint64_t> m_originalChunkMasks;
    Core::SharedPtr<ComponentPrefixResolver> m_prefixResolver;
    Core::SharedPtr<OverrideTagManager> m_tagManager;
};

class OverrideStateManager
{
public:
    Core::SharedPtr<EntityState>& GetEntityState(Red::ent::Entity* aEntity);
    Core::SharedPtr<EntityState>& FindEntityState(Red::ent::Entity* aEntity);
    void Reset();

private:
    Core::Map<Red::ent::Entity*, Core::SharedPtr<EntityState>> m_entityStates;
};
}
