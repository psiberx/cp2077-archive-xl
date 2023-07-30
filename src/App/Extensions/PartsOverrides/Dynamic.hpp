#pragma once

namespace App
{
struct DynamicReference
{
    Red::CName name;
    Red::CName variant;
    Core::Set<Red::CName> conditions;
    bool isDynamic;
    bool isConditional;
    uint8_t weight;
};

class DynamicAppearanceController
{
public:
    [[nodiscard]] DynamicReference ParseReference(Red::CName aReference, bool aRegister = false) const;
    [[nodiscard]] bool MatchReference(Red::Entity* aEntity, Red::CName aVariant,
                                      const DynamicReference& aReference) const;

    [[nodiscard]] Red::CName ResolveName(Red::Entity* aEntity, Red::CName aVariant, Red::CName aName) const;
    [[nodiscard]] Red::ResourcePath ResolvePath(Red::Entity* aEntity, Red::CName aVariant,
                                                Red::ResourcePath aPath) const;

    void UpdateState(Red::Entity* aEntity);
    void RemoveState(Red::Entity* aEntity);

    [[nodiscard]] bool IsDynamicValue(const char* aValue) const;
    [[nodiscard]] bool IsDynamicValue(const std::string& aValue) const;
    [[nodiscard]] bool IsDynamicValue(const std::string_view& aValue) const;

    void RegisterPath(Red::ResourcePath aPath, const char* aPathStr);
    void RegisterPath(Red::ResourcePath aPath, const std::string& aPathStr);
    void RegisterPath(Red::ResourcePath aPath, const std::string_view& aPathStr);

private:
    struct AttributeData
    {
        std::string value;
        std::string suffix;
    };

    struct EntityState
    {
        Core::Map<Red::CName, AttributeData> values;
        Core::Map<Red::CName, AttributeData> fallback;
        Core::Set<Red::CName> conditions;
    };

    struct DynamicString
    {
        bool valid;
        std::string value;
        Core::Set<Red::CName> attributes;
    };

    DynamicString ProcessString(const Core::Map<Red::CName, AttributeData>& aValues,
                                Red::CName aVariant, const char* aInput) const;

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
