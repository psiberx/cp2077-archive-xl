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

class EntityState
{
public:
    EntityState(Red::ent::Entity* aEntity) noexcept;
    EntityState(const EntityState&) = delete;
    EntityState(EntityState&&) noexcept = default;

    [[nodiscard]] const char* GetName() const;
    [[nodiscard]] Red::ent::Entity* GetEntity() const;

    [[nodiscard]] Core::Map<Red::CName, Core::SharedPtr<ComponentState>>& GetComponentStates();
    [[nodiscard]] Core::SharedPtr<ComponentState>& GetComponentState(Red::CName aComponentName);
    [[nodiscard]] Core::SharedPtr<ComponentState>& FindComponentState(Red::CName aComponentName);

    void AddOverride(uint64_t aHash, Red::CName aComponentName, uint64_t aChunkMask);
    void AddOverrideTag(uint64_t aHash, Red::CName aTag);
    void RemoveOverrides(uint64_t aHash);
    bool ApplyOverrides(Red::Handle<Red::ent::IComponent>& aComponent);

private:
    uint64_t GetOriginalChunkMask(ComponentWrapper& aComponent);

    std::string m_name;
    Red::ent::Entity* m_entity;
    Core::Map<Red::CName, Core::SharedPtr<ComponentState>> m_componentStates;
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
