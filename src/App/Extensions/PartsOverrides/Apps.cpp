#include "Apps.hpp"
#include "Red/CharacterCustomization.hpp"
#include "Red/Entity.hpp"
#include "Red/Rtti/Locator.hpp"

namespace
{
constexpr auto TagOpen = '{';
constexpr auto TagClose = '}';

constexpr auto SkinColorTag = Red::CName("{skin_color}");
constexpr auto HairColorTag = Red::CName("{hair_color}");
constexpr auto GenderTag = Red::CName("{gender}");

constexpr auto MaleBodyComponent = Red::CName("t0_000_pma_base__full");
constexpr auto FemaleBodyComponent1 = Red::CName("t0_000_pwa_base__full");
constexpr auto FemaleBodyComponent2 = Red::CName("t0_000_pwa_fpp__torso");

constexpr auto MaleTagValue = "m";
constexpr auto FemaleTagValue = "w";

Red::Rtti::ClassLocator<Red::game::ui::ICharacterCustomizationSystem> s_customizationSystemType;
}

Red::CName App::DynamicAppearanceResolver::GetAppearance(Red::ent::Entity* aEntity, Red::CName aAppearance)
{
    constexpr auto MaxLength = 255;

    char buffer[MaxLength + 1];
    char* out = buffer;
    const char* max = buffer + MaxLength;
    const char* app = aAppearance.ToString();

    while (app && *app)
    {
        auto* tagOpen = strchr(app, TagOpen);

        if (!tagOpen)
        {
            if (out == buffer)
                return aAppearance;

            strcpy_s(out, out - max, app);
            break;
        }

        auto* tagClose = strchr(app, TagClose);

        if (!tagClose)
        {
            return aAppearance;
        }

        while (app != tagOpen && out < max)
        {
            *out = *app;
            ++out;
            ++app;
        }

        if (out == max)
        {
            break;
        }

        app = tagClose + 1;

        const auto tag = Red::FNV1a64(reinterpret_cast<const uint8_t*>(tagOpen), tagClose - tagOpen + 1);
        auto* value = GetTagValue(tag, aEntity);

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

    return {buffer};
}

const char* App::DynamicAppearanceResolver::GetTagValue(Red::CName aTag, Red::ent::Entity* aEntity)
{
    Red::CName value;

    switch (aTag)
    {
    case SkinColorTag:
        value = GetSkinColor(aEntity);
        break;
    case HairColorTag:
        value = GetHairColor(aEntity);
        break;
    case GenderTag:
        return IsMale(aEntity) ? MaleTagValue : FemaleTagValue;
    }

    return value ? value.ToString() : nullptr;
}

Red::CName App::DynamicAppearanceResolver::GetSkinColor(Red::ent::Entity* aEntity)
{
    for (const auto& component : Raw::Entity::GetComponents(aEntity) | std::views::reverse)
    {
        switch (component->name)
        {
        case MaleBodyComponent:
        case FemaleBodyComponent1:
        case FemaleBodyComponent2:
            return component.GetPtr<Red::ent::MorphTargetSkinnedMeshComponent>()->meshAppearance;
        }
    }

    return {};
}

Red::CName App::DynamicAppearanceResolver::GetHairColor(Red::ent::Entity* aEntity)
{
    static auto* system = Red::CGameEngine::Get()->framework->gameInstance->GetInstance(s_customizationSystemType);

    Red::CName hairColor;
    Raw::CharacterCustomizationHelper::GetHairColor(hairColor, system->ref, IsMale(aEntity));

    return hairColor;
}

bool App::DynamicAppearanceResolver::IsMale(Red::ent::Entity* aEntity)
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

Core::SharedPtr<App::DynamicAppearanceResolver>& App::DynamicAppearanceResolver::Get()
{
    static auto s_instance = Core::MakeShared<DynamicAppearanceResolver>();
    return s_instance;
}
