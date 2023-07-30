#include "Dynamic.hpp"
#include "Red/AppearanceChanger.hpp"
#include "Red/CharacterCustomization.hpp"
#include "Red/Entity.hpp"

namespace
{
constexpr auto DynamicValueMarker = '!';
constexpr auto DynamicReferenceMarker = '!';
constexpr auto ConditionMarker = '&';

constexpr auto AttrOpen = '{';
constexpr auto AttrClose = '}';

constexpr auto GenderAttr = Red::CName("{gender}");
constexpr auto CameraAttr = Red::CName("{camera}");
constexpr auto BodyTypeAttr = Red::CName("{body_type}");
constexpr auto LegsStateAttr = Red::CName("{legs_state}");
constexpr auto SkinColorAttr = Red::CName("{skin_color}");
constexpr auto HairColorAttr = Red::CName("{hair_color}");
constexpr auto VariantAttr = Red::CName("{variant}");

constexpr auto GenderSuffix = Red::TweakDBID("itemsFactoryAppearanceSuffix.Gender");
constexpr auto CameraSuffix = Red::TweakDBID("itemsFactoryAppearanceSuffix.Camera");
constexpr auto BodyTypeSuffix = Red::TweakDBID("itemsFactoryAppearanceSuffix.BodyType");
constexpr auto LegsStateSuffix = Red::TweakDBID("itemsFactoryAppearanceSuffix.LegsState");

constexpr auto MaleSuffixValue = "Male";
constexpr auto FemaleSuffixValue = "Female";

constexpr auto MaleAttrValue = "m";
constexpr auto FemaleAttrValue = "w";

constexpr auto MaleBodyComponent = Red::CName("t0_000_pma_base__full");
constexpr auto FemaleBodyComponent1 = Red::CName("t0_000_pwa_base__full");
constexpr auto FemaleBodyComponent2 = Red::CName("t0_000_pwa_fpp__torso");

constexpr auto DefaultBodyTypeAttrValue = "base"; // FIXME: Better name?
constexpr auto DefaultBodyTypeSuffixValue = "BaseBody";

const std::string s_emptyPathStr;

Red::CName SubName(const char* aName, size_t aOffset, size_t aSize, bool aRegister = false)
{
    if (!aSize)
        return {};

    if (aRegister)
    {
        std::string nameStr(aName + aOffset, aSize);
        return Red::CNamePool::Add(nameStr.c_str());
    }

    return Red::FNV1a64(reinterpret_cast<const uint8_t*>(aName + aOffset), aSize);
}
}

App::DynamicReference App::DynamicAppearanceController::ParseReference(Red::CName aReference, bool aRegister) const
{
    DynamicReference result{};

    std::string_view referenceStr = aReference.ToString();
    const auto dynamicMarkerPos = referenceStr.find(DynamicReferenceMarker);

    if (dynamicMarkerPos != std::string_view::npos)
    {
        result.name = SubName(referenceStr.data(), 0, dynamicMarkerPos);
        result.isDynamic = true;

        referenceStr.remove_prefix(dynamicMarkerPos + 1);

        if (!referenceStr.empty())
        {
            auto conditionPos = referenceStr.find(ConditionMarker);

            result.isConditional = conditionPos != std::string_view::npos;

            if (result.isConditional)
            {
                result.variant = SubName(referenceStr.data(), 0, conditionPos, aRegister);
                referenceStr.remove_prefix(conditionPos + 1);

                while (true)
                {
                    conditionPos = referenceStr.find(ConditionMarker);

                    if (conditionPos == std::string_view::npos)
                    {
                        result.conditions.insert(SubName(referenceStr.data(), 0, referenceStr.size()));
                        break;
                    }

                    result.conditions.insert(SubName(referenceStr.data(), 0, conditionPos));
                    referenceStr.remove_prefix(conditionPos + 1);
                }
            }
            else
            {
                result.variant = SubName(referenceStr.data(), 0, referenceStr.size(), aRegister);
                result.isConditional = !result.variant.IsNone();
            }

            result.weight = result.conditions.size() + (result.variant.IsNone() ? 0 : 10);
        }
    }
    else
    {
        result.name = aReference;
        result.isDynamic = false;
    }

    return result;
}

bool App::DynamicAppearanceController::MatchReference(Red::Entity* aEntity, Red::CName aVariant,
                                                      const DynamicReference& aReference) const
{
    if (aReference.variant && aReference.variant != aVariant)
    {
        return false;
    }

    if (!aReference.conditions.empty())
    {
        const auto stateIt = m_states.find(aEntity);

        if (stateIt == m_states.end())
            return false;

        const auto& state = stateIt.value();

        for (const auto& condition : aReference.conditions)
        {
            if (!state.conditions.contains(condition))
                return false;
        }
    }

    return true;
}

Red::CName App::DynamicAppearanceController::ResolveName(Red::Entity* aEntity, Red::CName aVariant,
                                                         Red::CName aName) const
{
    const auto nameStr = aName.ToString();

    if (!IsDynamicValue(nameStr))
        return aName;

    const auto stateIt = m_states.find(aEntity);

    if (stateIt == m_states.end())
        return aName;

    const auto& state = stateIt.value();
    const auto result = ProcessString(state.values, aVariant, nameStr);

    if (!result.valid)
        return aName;

    return result.value.data();
}

Red::ResourcePath App::DynamicAppearanceController::ResolvePath(Red::Entity* aEntity, Red::CName aVariant,
                                                                Red::ResourcePath aPath) const
{
    const auto pathStr = GetPathStr(aPath);

    if (!IsDynamicValue(pathStr))
        return aPath;

    const auto stateIt = m_states.find(aEntity);

    if (stateIt == m_states.end())
        return aPath;

    const auto& state = stateIt.value();
    auto result = ProcessString(state.values, aVariant, pathStr.data());

    if (!result.valid)
        return aPath;

    Red::ResourcePath finalPath = result.value.data();

    if (result.attributes.contains(BodyTypeAttr))
    {
        if (!Red::ResourceDepot::Get()->ResourceExists(finalPath))
        {
            result = ProcessString(state.fallback, aVariant, pathStr.data());
            finalPath = result.value.data();
        }
    }

    return finalPath;
}

App::DynamicAppearanceController::DynamicString App::DynamicAppearanceController::ProcessString(
    const Core::Map<Red::CName, AttributeData>& aValues, Red::CName aVariant, const char* aInput) const
{
    constexpr auto MaxLength = 512;

    DynamicString result{};

    char buffer[MaxLength + 1];
    char* out = buffer;
    const char* max = buffer + MaxLength;
    const char* str = aInput;

    if (*str == DynamicValueMarker)
    {
        ++str;
    }

    while (str && *str)
    {
        auto* attrOpen = strchr(str, AttrOpen);

        if (!attrOpen)
        {
            if (out == buffer)
                return result;

            while (*str && out < max)
            {
                *out = *str;
                ++out;
                ++str;
            }
            break;
        }

        auto* attrClose = strchr(str, AttrClose);

        if (!attrClose)
        {
            return result;
        }

        while (str != attrOpen && out < max)
        {
            *out = *str;
            ++out;
            ++str;
        }

        if (out == max)
        {
            break;
        }

        str = attrClose + 1;

        const auto attr = Red::FNV1a64(reinterpret_cast<const uint8_t*>(attrOpen), attrClose - attrOpen + 1);
        const char* value = nullptr;

        if (attr == VariantAttr)
        {
            value = aVariant.ToString();
        }
        else
        {
            const auto it = aValues.find(attr);
            if (it != aValues.end())
            {
                value = it.value().value.data();
                result.attributes.insert(attr);
            }
        }

        if (value)
        {
            while (value && *value && out < max)
            {
                *out = *value;
                ++out;
                ++value;
            }

            if (out == max)
            {
                break;
            }
        }
    }

    *out = '\0';

    result.valid = true;
    result.value = buffer;

    return result;
}

void App::DynamicAppearanceController::UpdateState(Red::Entity* aEntity)
{
    auto& state = m_states[aEntity];

    state.values[GenderAttr] = GetSuffixData(aEntity, GenderSuffix);
    state.values[CameraAttr] = GetSuffixData(aEntity, CameraSuffix);
    state.values[BodyTypeAttr] = GetSuffixData(aEntity, BodyTypeSuffix);
    state.values[LegsStateAttr] = GetSuffixData(aEntity, LegsStateSuffix);
    state.values[SkinColorAttr] = GetSkinColorData(aEntity);
    state.values[HairColorAttr] = GetHairColorData(aEntity);

    state.fallback = state.values;
    state.fallback[BodyTypeAttr] = {DefaultBodyTypeAttrValue, DefaultBodyTypeSuffixValue};

    state.conditions.clear();
    for (const auto& [_, attribute] : state.values)
    {
        state.conditions.insert(attribute.suffix.data());
    }

    int x= 1; // fixme
}

void App::DynamicAppearanceController::RemoveState(Red::Entity* aEntity)
{
    m_states.erase(aEntity);
}

App::DynamicAppearanceController::AttributeData App::DynamicAppearanceController::GetAttributeData(
    Red::Entity* aEntity, Red::CName aAttribute) const
{
    switch (aAttribute)
    {
    case GenderAttr:
        return GetSuffixData(aEntity, GenderSuffix);
    case CameraAttr:
        return GetSuffixData(aEntity, CameraSuffix);
    case BodyTypeAttr:
        return GetSuffixData(aEntity, BodyTypeSuffix);
    case LegsStateAttr:
        return GetSuffixData(aEntity, LegsStateSuffix);
    case SkinColorAttr:
        return GetSkinColorData(aEntity);
    case HairColorAttr:
        return GetHairColorData(aEntity);
    }

    return {};
}

App::DynamicAppearanceController::AttributeData App::DynamicAppearanceController::GetSuffixData(
    Red::Entity* aEntity, Red::TweakDBID aSuffixID) const
{
    AttributeData data;

    {
        auto* handle = reinterpret_cast<Red::Handle<Red::GameObject>*>(&aEntity->ref);

        Red::CString suffixValue;
        Raw::AppearanceChanger::GetSuffixValue({}, 1ull, *handle, aSuffixID, suffixValue);

        data.suffix = suffixValue.c_str();
    }

    if (aSuffixID == GenderSuffix)
    {
        data.value = data.suffix == MaleSuffixValue ? MaleAttrValue : FemaleAttrValue;
    }
    else if (aSuffixID == BodyTypeSuffix && data.suffix.empty())
    {
        data.suffix = DefaultBodyTypeSuffixValue;
        data.value = DefaultBodyTypeAttrValue;
    }
    else
    {
        data.value.resize(data.suffix.size());
        std::transform(data.suffix.begin(), data.suffix.end(), data.value.begin(),
                       [](unsigned char c) { return std::tolower(c); });
    }

    return data;
}

App::DynamicAppearanceController::AttributeData App::DynamicAppearanceController::GetSkinColorData(
    Red::Entity* aEntity) const
{
    const auto& components = Raw::Entity::ComponentsStorage(aEntity)->components;
    for (const auto& component : components | std::views::reverse)
    {
        switch (component->name)
        {
        case MaleBodyComponent:
        case FemaleBodyComponent1:
        case FemaleBodyComponent2:
            const auto appearance = component.GetPtr<Red::ent::MorphTargetSkinnedMeshComponent>()->meshAppearance;
            if (appearance)
            {
                return {appearance.ToString() , ""};
            }
        }
    }

    return {};
}

App::DynamicAppearanceController::AttributeData App::DynamicAppearanceController::GetHairColorData(
    Red::Entity* aEntity) const
{
    static auto system = Red::GetGameSystem<Red::game::ui::ICharacterCustomizationSystem>();

    Red::CName hairColor;
    Raw::CharacterCustomizationHelper::GetHairColor(hairColor, system->ref, IsMale(aEntity));

    return {hairColor.ToString(), ""};
}

bool App::DynamicAppearanceController::IsMale(Red::Entity* aEntity) const
{
    for (const auto& component : Raw::Entity::GetComponents(aEntity) | std::views::reverse)
    {
        switch (component->name)
        {
        case MaleBodyComponent:
            return true;
        case FemaleBodyComponent1:
        case FemaleBodyComponent2:
            return false;
        }
    }

    return false;
}

bool App::DynamicAppearanceController::IsDynamicValue(const char* aValue) const
{
    return aValue && aValue[0] == DynamicValueMarker;
}

bool App::DynamicAppearanceController::IsDynamicValue(const std::string& aValue) const
{
    return aValue.starts_with(DynamicValueMarker);
}

bool App::DynamicAppearanceController::IsDynamicValue(const std::string_view& aValue) const
{
    return aValue.starts_with(DynamicValueMarker);
}

void App::DynamicAppearanceController::RegisterPath(Red::ResourcePath aPath, const char* aPathStr)
{
    m_paths.insert_or_assign(aPath, aPathStr);
}

void App::DynamicAppearanceController::RegisterPath(Red::ResourcePath aPath, const std::string& aPathStr)
{
    m_paths.insert_or_assign(aPath, aPathStr);
}

void App::DynamicAppearanceController::RegisterPath(Red::ResourcePath aPath, const std::string_view& aPathStr)
{
    m_paths.insert_or_assign(aPath, aPathStr);
}

const std::string& App::DynamicAppearanceController::GetPathStr(Red::ResourcePath aPath) const
{
    if (!aPath)
        return s_emptyPathStr;

    auto it = m_paths.find(aPath);

    if (it == m_paths.end())
        return s_emptyPathStr;

    return it.value();
}
