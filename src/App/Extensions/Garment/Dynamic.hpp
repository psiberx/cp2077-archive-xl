#pragma once

#include "App/Shared/ResourcePathRegistry.hpp"
#include "Red/EntityTemplate.hpp"

namespace App
{
using DynamicPartList = Core::Map<Red::CName, Red::CName>;
using DynamicTagList = Core::Set<Red::CName>;

struct DynamicAppearanceName
{
    DynamicAppearanceName();
    explicit DynamicAppearanceName(Red::CName aAppearance);
    explicit DynamicAppearanceName(const Red::CString& aAppearance);

    static bool CheckMark(Red::CName aAppearance);

    Red::CName value;
    Red::CName name;
    Red::CName variant;
    DynamicPartList parts;
    DynamicTagList overrides;
    uint64_t context;
    bool isDynamic;
};

struct DynamicAppearanceRef
{
    explicit DynamicAppearanceRef(Red::CName aReference);

    [[nodiscard]] bool Match(Red::CName aVariant) const;
    [[nodiscard]] bool Match(const DynamicTagList& aConditions) const;
    [[nodiscard]] bool Match(const DynamicTagList& aConditions, const DynamicTagList& aOverrides) const;

    Red::CName value;
    Red::CName name;
    DynamicTagList variants;
    DynamicTagList conditions;
    bool isDynamic;
    bool isConditional;
    int8_t weight;
};

struct DynamicAttributeData
{
    DynamicAttributeData() = default;

    DynamicAttributeData(std::string aValue, std::string aSuffix)
        : value(std::move(aValue))
        , suffix(std::move(aSuffix))
    {
    }

    DynamicAttributeData(const std::string& aValue)
        : value(aValue)
        , suffix(aValue)
    {
    }

    DynamicAttributeData(Red::CName aName)
    {
        if (aName)
        {
            value = aName.ToString();
            suffix = value;
        }
    }

    std::string value;
    std::string suffix;
};

using DynamicAttributeList = Core::Map<Red::CName, DynamicAttributeData>;

Red::CName ExtractDynamicName(const char* aName, size_t aOffset, bool aRegister = false);
Red::CName ExtractDynamicName(const char* aName, size_t aOffset, size_t aSize, bool aRegister = false);

class DynamicAppearanceController
{
public:
    struct DynamicString
    {
        std::string value;
        DynamicTagList attributes;
        bool valid;
        bool missed;
        bool optional;
    };

    DynamicAppearanceController(Core::SharedPtr<ResourcePathRegistry> aPathRegistry);

    [[nodiscard]] DynamicAppearanceName ParseAppearance(Red::CName aAppearance) const;
    [[nodiscard]] DynamicAppearanceRef ParseReference(Red::CName aReference) const;
    [[nodiscard]] bool MatchReference(const DynamicAppearanceRef& aReference, const Red::Handle<Red::Entity>& aEntity,
                                      const DynamicAppearanceName& aAppearance);
    [[nodiscard]] Red::CString ResolveString(const Red::Handle<Red::Entity>& aEntity, const DynamicPartList& aVariant,
                                             const Red::CString& aString);
    [[nodiscard]] Red::CName ResolveName(const Red::Handle<Red::Entity>& aEntity, const DynamicPartList& aVariant,
                                         Red::CName aName);
    [[nodiscard]] Red::ResourcePath ResolvePath(const Red::Handle<Red::Entity>& aEntity,
                                                const DynamicPartList& aVariant, Red::ResourcePath aPath);

    void UpdateState(const Red::Handle<Red::Entity>& aEntity, Red::TweakDBID aEquippedItemID = {});
    void RemoveState(const Red::WeakHandle<Red::Entity>& aEntity);

    bool SupportsDynamicAppearance(const Red::EntityTemplate* aTemplate);
    bool IsDynamicAppearanceName(Red::CName aAppearanceName);
    void MarkDynamicAppearanceName(Red::CName& aAppearanceName, const Red::Handle<Red::Entity>& aEntity);
    void MarkDynamicAppearanceName(Red::CName& aAppearanceName, DynamicAppearanceName& aSelector);
    std::string_view GetBaseAppearanceName(Red::CName aAppearanceName);

    [[nodiscard]] std::string GetPathString(Red::ResourcePath aPath) const;
    [[nodiscard]] std::string GetPathStringOrHash(Red::ResourcePath aPath) const;

    static bool IsMale(const Red::Handle<Red::Entity>& aEntity);

    [[nodiscard]] bool IsDynamicValue(const char* aValue) const;
    [[nodiscard]] bool IsDynamicValue(const std::string& aValue) const;
    [[nodiscard]] bool IsDynamicValue(const std::string_view& aValue) const;
    [[nodiscard]] bool IsDynamicValue(const Red::StringView& aValue) const;
    [[nodiscard]] bool IsDynamicValue(Red::CName aValue) const;

    DynamicString ProcessString(const DynamicAttributeList& aGlobalAttrs, const DynamicPartList& aLocalAttrs,
                                const char* aInput) const;

private:
    struct EntityState
    {
        DynamicAttributeList values;
        DynamicAttributeList defaults;
        DynamicTagList conditions;
        Red::TweakDBID equippedItemID;
        bool valid{false};
    };

    using EntityStatePtr = Core::SharedPtr<EntityState>;

    struct CustomizationData
    {
        bool isMale;
        Red::CName skinColor;
        Red::CName eyesColor;
        Red::CName hairColor;
        Red::CName nailsColor;
    };

    class LockedState
    {
    public:
        LockedState() noexcept = default;

        LockedState(Red::SharedSpinLock& aLock, EntityStatePtr aState) noexcept
            : m_lock(aLock)
            , m_state(std::move(aState))
        {
        }

        LockedState(std::shared_lock<Red::SharedSpinLock>&& aLock, EntityStatePtr aState) noexcept
            : m_lock(std::move(aLock))
            , m_state(std::move(aState))
        {
        }

        [[nodiscard]] explicit operator bool() const noexcept
        {
            return m_state != nullptr;
        }

        [[nodiscard]] const EntityState& operator*() const noexcept
        {
            return *m_state;
        }

        [[nodiscard]] const EntityState* operator->() const noexcept
        {
            return m_state.get();
        }

    private:
        std::shared_lock<Red::SharedSpinLock> m_lock;
        const EntityStatePtr m_state;
    };

    LockedState EnsureState(const Red::Handle<Red::Entity>& aEntity);

    DynamicAttributeList CollectEntityData(const Red::Handle<Red::Entity>& aEntity, Red::TweakDBID aEquippedItemID) const;
    DynamicAttributeData GetSuffixData(const Red::Handle<Red::GameObject>& aPuppet, Red::TweakDBID aSuffixID,
                                       Red::TweakDBID aEquippedItemID) const;
    CustomizationData GetCustomizationData(const Red::Handle<Red::Entity>& aEntity) const;

    mutable Red::SharedSpinLock m_lock;
    Core::Map<Red::Entity*, EntityStatePtr> m_states;
    Core::SharedPtr<ResourcePathRegistry> m_pathRegistry;
};
}
