#pragma once

#include "App/Extensions/GarmentOverride/Dynamic.hpp"
#include "App/Extensions/GarmentOverride/Prefix.hpp"
#include "App/Extensions/GarmentOverride/Tags.hpp"
#include "App/Extensions/GarmentOverride/Wrapper.hpp"
#include "Red/AppearanceResource.hpp"
#include "Red/GarmentAssembler.hpp"

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

    void LinkToAppearance(DynamicAppearanceName aAppearance);
    [[nodiscard]] bool IsDynamicPart() const;
    [[nodiscard]] Red::CName GetActiveVariant() const;
    [[nodiscard]] const DynamicPartList& GetActiveVariantParts() const;

private:
    Red::ResourcePath m_path;
    DynamicAppearanceName m_appearance;
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
    [[nodiscard]] bool IsValid() const;

    [[nodiscard]] const Core::Map<Red::CName, Core::SharedPtr<ComponentState>>& GetComponentStates() const;
    [[nodiscard]] const Core::SharedPtr<ComponentState>& GetComponentState(Red::CName aComponentName);
    [[nodiscard]] const Core::SharedPtr<ComponentState>& FindComponentState(Red::CName aComponentName) const;

    [[nodiscard]] const Core::Map<Red::ResourcePath, Core::SharedPtr<ResourceState>>& GetResourceStates() const;
    [[nodiscard]] const Core::SharedPtr<ResourceState>& GetResourceState(Red::ResourcePath aResourcePath);
    [[nodiscard]] const Core::SharedPtr<ResourceState>& FindResourceState(Red::ResourcePath aResourcePath) const;

    [[nodiscard]] const Core::Set<Red::CName>& GetPartComponentNames(Red::ResourcePath aResourcePath) const;

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

    bool SelectDynamicAppearance(DynamicAppearanceName& aSelector, Red::EntityTemplate* aResource,
                                  Red::TemplateAppearance*& aAppearance);
    bool SelectDynamicAppearance(DynamicAppearanceName& aSelector, Red::AppearanceResource* aResource,
                                 Red::Handle<Red::AppearanceDefinition>& aDefinition);
    void ToggleConditionalComponents(Red::DynArray<Red::Handle<Red::IComponent>>& aComponents);
    bool ApplyDynamicAppearance(Red::Handle<Red::IComponent>& aComponent);
    bool ApplyDynamicAppearance(Red::Handle<Red::IComponent>& aComponent, Red::ResourcePath aResource);
    void LinkComponentToPart(Red::Handle<Red::IComponent>& aComponent, Red::ResourcePath aResource);
    void LinkPartToAppearance(Red::ResourcePath aResource, DynamicAppearanceName& aAppearance);
    void LinkPartToAppearance(Red::ResourcePath aResource, Red::CName aAppearance);
    void UpdateDynamicAttributes();

    void RemoveAllOverrides(uint64_t aHash);

private:
    struct WeightedAppearanceMatch
    {
        int8_t weight;
        Red::Handle<Red::AppearanceDefinition>& definition;
    };

    struct WeightedComponentMatch
    {
        int8_t weight;
        Red::Handle<Red::IComponent>& component;
    };

    struct WeightedComponentGroup
    {
        int8_t maxWeight = 0;
        Core::Vector<WeightedComponentMatch> matches;
    };

    bool ApplyDynamicAppearance(Red::Handle<Red::IComponent>& aComponent, const DynamicPartList& aVariant,
                                bool aSetAppearance);

    Red::ResourcePath GetOriginalResource(ComponentWrapper& aComponent);
    Red::CName GetOriginalAppearance(ComponentWrapper& aComponent);
    uint64_t GetOriginalChunkMask(ComponentWrapper& aComponent);

    std::string m_name;
    Red::Entity* m_entity;
    Red::WeakHandle<Red::Entity> m_entityWeak;
    Core::Map<Red::CName, Core::SharedPtr<ComponentState>> m_componentStates;
    Core::Map<Red::ResourcePath, Core::SharedPtr<ResourceState>> m_resourceStates;
    Core::Map<Red::ResourcePath, Core::Set<Red::CName>> m_partComponents;
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

    Core::SharedPtr<EntityState>& GetFirstEntityState();
    Core::SharedPtr<EntityState>& GetEntityState(uint64_t aContext);
    Core::SharedPtr<EntityState>& GetEntityState(Red::Entity* aEntity);

    Core::SharedPtr<EntityState>& FindEntityState(Red::Entity* aEntity);
    Core::SharedPtr<EntityState>& FindEntityState(Red::ResourcePath aPath);
    Core::SharedPtr<EntityState>& FindEntityState(Red::GarmentProcessor* aProcessor);
    Core::SharedPtr<EntityState>& FindEntityState(uintptr_t aPointer);

    void ClearStates();

    void LinkEntityToAssembler(Red::Entity* aEntity, Red::GarmentProcessor* aProcessor);
    void LinkEntityToPointer(Red::Entity* aEntity, uintptr_t aPointer);
    void LinkComponentToPart(Red::Handle<Red::IComponent>& aComponent, Red::ResourcePath aResource);
    void UpdateDynamicAttributes();

private:
    Core::Map<Red::Entity*, Core::SharedPtr<EntityState>> m_entityStates;
    Core::Map<Red::ResourcePath, Core::SharedPtr<EntityState>> m_entityStatesByPath;
    Core::Map<Red::GarmentProcessor*, Core::SharedPtr<EntityState>> m_entityStatesByProcessor;
    Core::Map<uintptr_t, Core::SharedPtr<EntityState>> m_entityStatesByPointer;
    Core::SharedPtr<DynamicAppearanceController> m_dynamicAppearance;
};
}
