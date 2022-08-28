#pragma once

#include "stdafx.hpp"
#include "Garment.hpp"

#include <RED4ext/Scripting/Natives/Generated/ent/Entity.hpp>
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
    [[nodiscard]] uint64_t GetChunkMask() const;
    bool SetChunkMask(uint64_t aChunkMask) const;
    [[nodiscard]] RED4ext::CName GetAppearanceName() const;

private:
    RED4ext::Handle<RED4ext::ent::IComponent>& m_component;
    ComponentType m_type;
};

class ComponentState
{
public:
    ComponentState(RED4ext::CName aName);
    ComponentState(const ComponentState&) = delete;
    ComponentState(ComponentState&&) noexcept = default;

    [[nodiscard]] const char* GetName() const;
    [[nodiscard]] bool IsOverridden() const;

    void AddChunkMaskOverride(uint64_t aHash, uint64_t aChunkMask);
    bool RemoveChunkMaskOverride(uint64_t aHash);
    bool ApplyChunkMask(RED4ext::Handle<RED4ext::ent::IComponent>&);

private:
    std::string m_name;
    Core::Map<ItemHash, uint64_t> m_overridenChunkMasks;
    Core::Map<RED4ext::CName, uint64_t> m_initialChunkMasks;
};

class EntityState
{
public:
    EntityState(RED4ext::ent::Entity* aEntity);
    EntityState(const EntityState&) = delete;
    EntityState(EntityState&&) noexcept = default;

    [[nodiscard]] const char* GetName() const;
    [[nodiscard]] Core::Map<RED4ext::CName, Core::SharedPtr<ComponentState>>& GetComponentStates();
    [[nodiscard]] Core::SharedPtr<ComponentState>& GetComponentState(RED4ext::CName aComponentName);
    [[nodiscard]] Core::SharedPtr<ComponentState>& FindComponentState(RED4ext::CName aComponentName);

private:
    std::string m_name;
    Core::Map<RED4ext::CName, Core::SharedPtr<ComponentState>> m_componentStates;
};

class StateStorage
{
public:
    Core::SharedPtr<EntityState>& GetEntityState(RED4ext::ent::Entity* aEntity);
    Core::SharedPtr<EntityState>& FindEntityState(RED4ext::ent::Entity* aEntity);
    void Reset();

private:
    Core::Map<RED4ext::ent::Entity*, Core::SharedPtr<EntityState>> m_entityStates;
};
}
