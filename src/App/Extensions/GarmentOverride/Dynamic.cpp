#include "Dynamic.hpp"
#include "App/Extensions/GarmentOverride/Wrapper.hpp"
#include "App/Extensions/PuppetState/Module.hpp"
#include "App/Extensions/PuppetState/System.hpp"
#include "App/Utils/Num.hpp"
#include "App/Utils/Str.hpp"
#include "Red/AppearanceChanger.hpp"
#include "Red/CharacterCustomization.hpp"
#include "Red/Entity.hpp"
#include "Red/TweakDB.hpp"

namespace
{
constexpr auto DynamicAppearanceTag = Red::CName("DynamicAppearance");

constexpr auto ContextMarker = '%';
constexpr auto VariantMarker = '!';
constexpr auto PartSeparator = '+';
constexpr auto PartNameGlue = '.';
constexpr auto ConditionMarker = '&';
constexpr auto ReferenceMarkers = "!&";
constexpr auto TransientMarkers = "%&";
constexpr auto AllMarkers = "!%&";

constexpr auto DynamicValueMarker = '*';
constexpr auto AttrOpen = '{';
constexpr auto AttrClose = '}';
constexpr auto ConditionEqual = "=";

constexpr auto GenderAttr = Red::CName("gender");
constexpr auto CameraAttr = Red::CName("camera");
constexpr auto BodyTypeAttr = Red::CName("body");
constexpr auto ArmsStateAttr = Red::CName("arms");
constexpr auto FeetStateAttr = Red::CName("feet");
constexpr auto InnerSleevesAttr = Red::CName("sleeves");
constexpr auto SkinColorAttr = Red::CName("skin_color");
constexpr auto HairColorAttr = Red::CName("hair_color");
constexpr auto EyesColorAttr = Red::CName("eyes_color");
constexpr auto VariantAttr = Red::CName("variant");

constexpr auto GenderSuffix = Red::TweakDBID("itemsFactoryAppearanceSuffix.Gender");
constexpr auto CameraSuffix = Red::TweakDBID("itemsFactoryAppearanceSuffix.Camera");
constexpr auto BodyTypeSuffix = App::PuppetStateModule::BodyTypeSuffixID;
constexpr auto ArmsStateSuffix = App::PuppetStateModule::ArmsStateSuffixID;
constexpr auto FeetStateSuffix = App::PuppetStateModule::FeetStateSuffixID;
constexpr auto InnerSleevesSuffix = Red::TweakDBID("itemsFactoryAppearanceSuffix.Partial");

constexpr auto MaleAttrValue = "m";
constexpr auto FemaleAttrValue = "w";
constexpr auto MaleSuffixValue = "Male";
constexpr auto FemaleSuffixValue = "Female";

constexpr auto DefaultBodyTypeAttrValue = "base_body";
constexpr auto DefaultBodyTypeSuffixValue = App::PuppetStateSystem::BaseBodyName;
constexpr auto DefaultFeetStateAttrValue = "flat";
constexpr auto DefaultFeetStateSuffixValue = "Flat";

constexpr auto MaleBodyComponent = Red::CName("t0_000_pma_base__full");
constexpr auto FemaleBodyComponent1 = Red::CName("t0_000_pwa_base__full");
constexpr auto FemaleBodyComponent2 = Red::CName("t0_000_pwa_fpp__torso");

constexpr auto MaleEyesComponent = Red::CName("he_000_pma__basehead");
constexpr auto FemaleEyesComponent = Red::CName("MorphTargetSkinnedMesh3637");

const std::string s_emptyPathStr;

Red::CName ExtractName(const char* aName, size_t aOffset, size_t aSize, bool aRegister = false)
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

App::DynamicAppearanceName::DynamicAppearanceName()
    : isDynamic(false)
    , context(0)
{
}

App::DynamicAppearanceName::DynamicAppearanceName(Red::CName aAppearance)
    : isDynamic(false)
    , context(0)
    , value(aAppearance)
{
    std::string_view str = aAppearance.ToString();

    auto markerPos = str.find_first_of(VariantMarker);
    if (markerPos != std::string_view::npos)
    {
        {
            auto contextPos = str.find_last_of(ContextMarker);
            if (contextPos != std::string_view::npos)
            {
                ParseInt(str.data() + contextPos + 1, str.size() - contextPos - 1, context);
                str.remove_suffix(str.size() - contextPos);
            }
        }

        {
            auto suffixPos = str.find_last_of(ConditionMarker, markerPos);
            if (suffixPos != std::string_view::npos)
            {
                str.remove_suffix(str.size() - suffixPos);
            }
        }

        isDynamic = true;
        name = ExtractName(str.data(), 0, markerPos);

        str.remove_prefix(markerPos + 1);

        if (!str.empty())
        {
            variant = ExtractName(str.data(), 0, str.size(), true);
            parts[VariantAttr] = variant;

            uint8_t partNum[2]{PartNameGlue, '1'};
            size_t skip;

            while (!str.empty())
            {
                markerPos = str.find(PartSeparator);

                if (markerPos == 0)
                {
                    str.remove_prefix(1);
                    continue;
                }

                if (markerPos == std::string_view::npos)
                {
                    markerPos = str.size();
                    skip = str.size();
                }
                else
                {
                    skip = markerPos + 1;
                }

                auto partName = Red::FNV1a64(partNum, 2, VariantAttr);
                auto partValue = ExtractName(str.data(), 0, markerPos, true);
                parts[partName] = partValue;

                str.remove_prefix(skip);
                ++partNum[1];
            }
        }
    }
    else
    {
        name = aAppearance;
    }
}

bool App::DynamicAppearanceName::CheckMark(Red::CName aAppearance)
{
    std::string_view str = aAppearance.ToString();
    return str.find_first_of(ContextMarker) != std::string_view::npos;
}

App::DynamicAppearanceRef::DynamicAppearanceRef(Red::CName aReference)
    : isDynamic(false)
    , isConditional(false)
    , value(aReference)
{
    std::string_view str = aReference.ToString();

    auto markerPos = str.find_first_of(ReferenceMarkers);
    if (markerPos != std::string_view::npos)
    {
        isDynamic = true;
        name = ExtractName(str.data(), 0, markerPos);

        str.remove_prefix(markerPos);

        if (str.size() > 1)
        {
            if (str[0] == VariantMarker)
            {
                str.remove_prefix(1);

                while (!str.empty())
                {
                    markerPos = str.find_first_of(ReferenceMarkers);

                    if (markerPos == 0 && str[0] == ConditionMarker)
                    {
                        break;
                    }

                    if (markerPos == std::string_view::npos)
                    {
                        variants.insert(ExtractName(str.data(), 0, str.size()));
                        // don't remove prefix for condition marker check
                        break;
                    }

                    variants.insert(ExtractName(str.data(), 0, markerPos));

                    if (str[markerPos] != VariantMarker)
                    {
                        str.remove_prefix(markerPos);
                        break;
                    }

                    str.remove_prefix(markerPos + 1);
                }
            }

            if (str[0] == ConditionMarker)
            {
                str.remove_prefix(1);

                while (!str.empty())
                {
                    markerPos = str.find(ConditionMarker);

                    if (markerPos == std::string_view::npos)
                    {
                        conditions.insert(ExtractName(str.data(), 0, str.size()));
                        break;
                    }

                    conditions.insert(ExtractName(str.data(), 0, markerPos));

                    str.remove_prefix(markerPos + 1);
                }
            }

            weight = static_cast<int8_t>((variants.empty() ? 0 : 100) + conditions.size());
            isConditional = weight > 0;
        }
    }
    else
    {
        name = aReference;
    }
}

bool App::DynamicAppearanceRef::Match(Red::CName aVariant) const
{
    return variants.contains(aVariant);
}

bool App::DynamicAppearanceRef::Match(const DynamicTagList& aConditions) const
{
    for (const auto& condition : conditions)
    {
        if (!aConditions.contains(condition))
            return false;
    }

    return true;
}

App::DynamicAppearanceName App::DynamicAppearanceController::ParseAppearance(Red::CName aAppearance) const
{
    return DynamicAppearanceName(aAppearance);
}

App::DynamicAppearanceRef App::DynamicAppearanceController::ParseReference(Red::CName aReference) const
{
    return DynamicAppearanceRef(aReference);
}

bool App::DynamicAppearanceController::MatchReference(const DynamicAppearanceRef& aReference, Red::Entity* aEntity,
                                                      Red::CName aVariant) const
{
    if (!aReference.variants.empty() )
    {
        if (!aReference.Match(aVariant))
            return false;
    }

    if (!aReference.conditions.empty())
    {
        const auto stateIt = m_states.find(aEntity);

        if (stateIt == m_states.end())
            return false;

        const auto& state = stateIt.value();

        if (!aReference.Match(state.conditions))
            return false;
    }

    return true;
}

Red::CName App::DynamicAppearanceController::ResolveName(Red::Entity* aEntity, const DynamicPartList& aVariant,
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

Red::ResourcePath App::DynamicAppearanceController::ResolvePath(Red::Entity* aEntity, const DynamicPartList& aVariant,
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

    if (!Red::ResourceDepot::Get()->ResourceExists(finalPath))
    {
        if (result.attributes.contains(BodyTypeAttr) || result.attributes.contains(FeetStateAttr))
        {
            result = ProcessString(state.fallback, aVariant, pathStr.data());
            finalPath = result.value.data();
        }
    }

    return finalPath;
}

App::DynamicAppearanceController::DynamicString App::DynamicAppearanceController::ProcessString(
    const DynamicAttributeList& aGlobalAttrs, const DynamicPartList& aLocalAttrs, const char* aInput) const
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

        const auto attr = Red::FNV1a64(reinterpret_cast<const uint8_t*>(attrOpen + 1), attrClose - attrOpen - 1);
        const char* value = nullptr;

        {
            const auto localIt = aLocalAttrs.find(attr);
            if (localIt != aLocalAttrs.end())
            {
                value = localIt.value().ToString();
            }
            else
            {
                const auto globalIt = aGlobalAttrs.find(attr);
                if (globalIt != aGlobalAttrs.end())
                {
                    value = globalIt.value().value.data();
                    result.attributes.insert(attr);
                }
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
    state.values[ArmsStateAttr] = GetSuffixData(aEntity, ArmsStateSuffix);
    state.values[FeetStateAttr] = GetSuffixData(aEntity, FeetStateSuffix);
    state.values[InnerSleevesAttr] = GetSuffixData(aEntity, InnerSleevesSuffix);

    auto custimizationData = GetCustomizationData(aEntity);
    state.values[SkinColorAttr] = {custimizationData.skinColor};
    state.values[HairColorAttr] = {custimizationData.hairColor};
    state.values[EyesColorAttr] = {custimizationData.eyesColor};

    state.fallback = state.values;
    state.fallback[BodyTypeAttr] = {DefaultBodyTypeAttrValue, DefaultBodyTypeSuffixValue};
    state.fallback[FeetStateAttr] = {DefaultFeetStateAttrValue, DefaultFeetStateSuffixValue};

    state.conditions.clear();
    for (const auto& [attributeName, attributeData] : state.values)
    {
        if (!attributeData.value.empty())
        {
            uint64_t condition = Red::FNV1a64(attributeData.value.data(), Red::FNV1a64(ConditionEqual, attributeName));
            state.conditions.insert(condition);
        }
    }
}

void App::DynamicAppearanceController::RemoveState(Red::Entity* aEntity)
{
    m_states.erase(aEntity);
}

App::DynamicAttributeData App::DynamicAppearanceController::GetSuffixData(Red::Entity* aEntity,
                                                                          Red::TweakDBID aSuffixID) const
{
    DynamicAttributeData data{};

    if (Red::RecordExists(aSuffixID))
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
    else if (!data.suffix.empty())
    {
        data.value = Str::SnakeCase(data.suffix);
    }

    return data;
}

App::DynamicAppearanceController::CustomizationData App::DynamicAppearanceController::GetCustomizationData(
    Red::Entity* aEntity) const
{
    static auto system = Red::GetGameSystem<Red::game::ui::ICharacterCustomizationSystem>();

    CustomizationData data{};

    for (const auto& component : aEntity->components | std::views::reverse)
    {
        switch (component->name)
        {
        case MaleBodyComponent:
            data.isMale = true;
            data.skinColor = ComponentWrapper(component).GetAppearanceName();
            break;
        case FemaleBodyComponent1:
        case FemaleBodyComponent2:
            data.isMale = false;
            data.skinColor = ComponentWrapper(component).GetAppearanceName();
            break;
        case MaleEyesComponent:
        case FemaleEyesComponent:
            data.eyesColor = ComponentWrapper(component).GetAppearanceName();
            break;
        }
    }

    Raw::CharacterCustomizationHelper::GetHairColor(data.hairColor, system->ref, data.isMale);

    return data;
}

bool App::DynamicAppearanceController::IsMale(Red::Entity* aEntity)
{
    for (const auto& component : aEntity->components | std::views::reverse)
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

bool App::DynamicAppearanceController::IsDynamicValue(const Red::StringView& aValue) const
{
    return aValue.size > 0 && IsDynamicValue(aValue.data);
}

bool App::DynamicAppearanceController::IsDynamicValue(Red::CName aValue) const
{
    return IsDynamicValue(aValue.ToString());
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

void App::DynamicAppearanceController::RegisterPath(Red::ResourcePath aPath, const Red::StringView& aPathStr)
{
    m_paths.insert_or_assign(aPath, std::string(aPathStr.data, aPathStr.size));
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

bool App::DynamicAppearanceController::SupportsDynamicAppearance(const Red::EntityTemplate* aTemplate)
{
    return aTemplate->visualTagsSchema &&
           aTemplate->visualTagsSchema->visualTags.Contains(DynamicAppearanceTag);
}

void App::DynamicAppearanceController::MarkDynamicAppearanceName(Red::CName& aAppearanceName, Red::Entity* aEntity)
{
    std::string_view appearanceName = aAppearanceName.ToString();

    {
        auto suffixPos = appearanceName.find_first_of(TransientMarkers);
        if (suffixPos != std::string_view::npos)
        {
            appearanceName.remove_suffix(appearanceName.size() - suffixPos);
        }
    }

    std::string dynamicName(appearanceName);

    if (dynamicName.find(VariantMarker) == std::string::npos)
    {
        dynamicName += VariantMarker;
    }

    dynamicName += ContextMarker;
    dynamicName += std::to_string(reinterpret_cast<uint64_t>(aEntity));

    aAppearanceName = Red::CNamePool::Add(dynamicName.c_str());
}

void App::DynamicAppearanceController::MarkDynamicAppearanceName(Red::CName& aAppearanceName,
                                                                 DynamicAppearanceName& aSelector)
{
    std::string dynamicName(aAppearanceName.ToString());
    dynamicName += VariantMarker;

    if (aSelector.variant)
    {
        dynamicName += aSelector.variant.ToString();
    }

    dynamicName += ContextMarker;
    dynamicName += std::to_string(aSelector.context);

    aAppearanceName = Red::CNamePool::Add(dynamicName.c_str());
}

std::string_view App::DynamicAppearanceController::GetBaseAppearanceName(Red::CName aAppearanceName)
{
    std::string_view baseName = aAppearanceName.ToString();

    auto markerPos = baseName.find_first_of(AllMarkers);
    if (markerPos != std::string_view::npos)
    {
        baseName.remove_suffix(baseName.size() - markerPos);
    }

    return baseName;
}
