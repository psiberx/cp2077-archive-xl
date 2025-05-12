#pragma once

#include "App/Shared/ResourcePathRegistry.hpp"
#include "Red/EntityTemplate.hpp"
#include "Red/ResourcePath.hpp"

namespace App
{
using DynamicPartList = Core::Map<Red::CName, Red::CName>;
using DynamicTagList = Core::Set<Red::CName>;

struct DynamicAppearanceName
{
    DynamicAppearanceName();
    explicit DynamicAppearanceName(Red::CName aAppearance);

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
    [[nodiscard]] bool MatchReference(const DynamicAppearanceRef& aReference, Red::Entity* aEntity,
                                      const DynamicAppearanceName& aAppearance) const;

    [[nodiscard]] Red::CName ResolveName(Red::Entity* aEntity, const DynamicPartList& aVariant,
                                         Red::CName aName) const;
    [[nodiscard]] Red::ResourcePath ResolvePath(Red::Entity* aEntity, const DynamicPartList& aVariant,
                                                Red::ResourcePath aPath) const;

    void UpdateState(Red::Entity* aEntity, Red::TweakDBID aEquippedItemID = {});
    void RemoveState(Red::Entity* aEntity);

    bool SupportsDynamicAppearance(const Red::EntityTemplate* aTemplate);
    bool IsDynamicAppearanceName(Red::CName aAppearanceName);
    void MarkDynamicAppearanceName(Red::CName& aAppearanceName, Red::Entity* aEntity);
    void MarkDynamicAppearanceName(Red::CName& aAppearanceName, DynamicAppearanceName& aSelector);
    std::string_view GetBaseAppearanceName(Red::CName aAppearanceName);

    [[nodiscard]] std::string GetPathString(Red::ResourcePath aPath) const;
    [[nodiscard]] std::string GetPathStringOrHash(Red::ResourcePath aPath) const;

    static bool IsMale(Red::Entity* aEntity);

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
        DynamicAttributeList fallbackBody;
        DynamicAttributeList fallbackFeet;
        DynamicTagList conditions;
    };

    struct CustomizationData
    {
        bool isMale;
        Red::CName skinColor;
        Red::CName eyesColor;
        Red::CName hairColor;
    };

    DynamicAttributeData GetSuffixData(Red::Entity* aEntity, Red::TweakDBID aSuffixID,
                                       Red::TweakDBID aEquippedItemID) const;
    CustomizationData GetCustomizationData(Red::Entity* aEntity) const;

    Core::Map<Red::Entity*, EntityState> m_states;
    Core::SharedPtr<ResourcePathRegistry> m_pathRegistry;
};
}
