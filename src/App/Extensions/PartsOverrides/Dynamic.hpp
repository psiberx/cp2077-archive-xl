#pragma once

#include "Red/EntityTemplate.hpp"

namespace App
{
using DynamicPartList = Core::Map<Red::CName, Red::CName>;
using DynamicTagList = Core::Set<Red::CName>;

struct DynamicAppearanceName
{
    DynamicAppearanceName();
    explicit DynamicAppearanceName(Red::CName aAppearance);

    Red::CName value;
    Red::CName name;
    Red::CName variant;
    DynamicPartList parts;
    uint64_t context;
    bool isDynamic;
};

struct DynamicReference
{
    explicit DynamicReference(Red::CName aReference);

    [[nodiscard]] bool Match(Red::CName aVariant) const;
    [[nodiscard]] bool Match(const DynamicTagList& aConditions) const;

    Red::CName value;
    Red::CName name;
    DynamicTagList variants;
    DynamicTagList conditions;
    bool isDynamic;
    bool isConditional;
    int8_t weight;
};

class DynamicAppearanceController
{
public:
    [[nodiscard]] DynamicAppearanceName ParseAppearance(Red::CName aAppearance) const;
    [[nodiscard]] DynamicReference ParseReference(Red::CName aReference) const;
    [[nodiscard]] bool MatchReference(const DynamicReference& aReference, Red::Entity* aEntity,
                                      Red::CName aVariant) const;

    [[nodiscard]] Red::CName ResolveName(Red::Entity* aEntity, const DynamicPartList& aVariant,
                                         Red::CName aName) const;
    [[nodiscard]] Red::ResourcePath ResolvePath(Red::Entity* aEntity, const DynamicPartList& aVariant,
                                                Red::ResourcePath aPath) const;

    void UpdateState(Red::Entity* aEntity);
    void RemoveState(Red::Entity* aEntity);

    [[nodiscard]] bool IsDynamicValue(const char* aValue) const;
    [[nodiscard]] bool IsDynamicValue(const std::string& aValue) const;
    [[nodiscard]] bool IsDynamicValue(const std::string_view& aValue) const;

    void RegisterPath(Red::ResourcePath aPath, const char* aPathStr);
    void RegisterPath(Red::ResourcePath aPath, const std::string& aPathStr);
    void RegisterPath(Red::ResourcePath aPath, const std::string_view& aPathStr);

    bool SupportsDynamicAppearance(const Red::EntityTemplate* aTemplate);
    void MarkDynamicAppearanceName(Red::CName& aAppearanceName, Red::Entity* aEntity);
    void MarkDynamicAppearanceName(Red::CName& aAppearanceName, DynamicAppearanceName& aSelector);

private:
    struct AttributeData
    {
        std::string value;
        std::string suffix;
    };

    using DynamicAttrList = Core::Map<Red::CName, AttributeData>;

    struct EntityState
    {
        DynamicAttrList values;
        DynamicAttrList fallback;
        DynamicTagList conditions;
    };

    struct DynamicString
    {
        bool valid;
        std::string value;
        DynamicTagList attributes;
    };

    DynamicString ProcessString(const DynamicAttrList& aAttrs, const DynamicPartList& aVariant,
                                const char* aInput) const;

    AttributeData GetAttributeData(Red::Entity* aEntity, Red::CName aAttribute) const;
    AttributeData GetSuffixData(Red::Entity* aEntity, Red::TweakDBID aSuffixID) const;
    AttributeData GetSkinColorData(Red::Entity* aEntity) const;
    AttributeData GetHairColorData(Red::Entity* aEntity) const;
    bool IsMale(Red::Entity* aEntity) const;

    [[nodiscard]] const std::string& GetPathStr(Red::ResourcePath aPath) const;

    Core::Map<Red::Entity*, EntityState> m_states;
    Core::Map<Red::ResourcePath, std::string> m_paths;
};
}
