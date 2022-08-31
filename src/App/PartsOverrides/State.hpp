#pragma once

#include "Prefix.hpp"
#include "Tags.hpp"
#include "Wrapper.hpp"

#include <RED4ext/Scripting/Natives/Generated/ent/Entity.hpp>
#include <RED4ext/Scripting/Natives/Generated/ent/IComponent.hpp>

namespace App
{
class ComponentState
{
public:
    ComponentState(RED4ext::CName aName) noexcept;
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
    EntityState(RED4ext::ent::Entity* aEntity) noexcept;
    EntityState(const EntityState&) = delete;
    EntityState(EntityState&&) noexcept = default;

    [[nodiscard]] const char* GetName() const;
    [[nodiscard]] RED4ext::ent::Entity* GetEntity() const;

    [[nodiscard]] Core::Map<RED4ext::CName, Core::SharedPtr<ComponentState>>& GetComponentStates();
    [[nodiscard]] Core::SharedPtr<ComponentState>& GetComponentState(RED4ext::CName aComponentName);
    [[nodiscard]] Core::SharedPtr<ComponentState>& FindComponentState(RED4ext::CName aComponentName);

    void AddOverride(uint64_t aHash, RED4ext::CName aComponentName, uint64_t aChunkMask);
    void AddOverrideTag(uint64_t aHash, RED4ext::CName aTag);
    void RemoveOverrides(uint64_t aHash);
    bool ApplyOverrides(RED4ext::Handle<RED4ext::ent::IComponent>& aComponent);

private:
    uint64_t GetOriginalChunkMask(ComponentWrapper& aComponent);

    std::string m_name;
    RED4ext::ent::Entity* m_entity;
    Core::Map<RED4ext::CName, Core::SharedPtr<ComponentState>> m_componentStates;
    Core::Map<uint64_t, uint64_t> m_originalChunkMasks;
    Core::SharedPtr<ComponentPrefixResolver> m_prefixResolver;
    Core::SharedPtr<OverrideTagManager> m_tagManager;
};

class OverrideStateManager
{
public:
    Core::SharedPtr<EntityState>& GetEntityState(RED4ext::ent::Entity* aEntity);
    Core::SharedPtr<EntityState>& FindEntityState(RED4ext::ent::Entity* aEntity);
    void Reset();

private:
    Core::Map<RED4ext::ent::Entity*, Core::SharedPtr<EntityState>> m_entityStates;
};
}
