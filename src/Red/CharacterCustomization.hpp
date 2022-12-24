#pragma once

#include "Core/Raw.hpp"
#include "Red/Addresses.hpp"

namespace Red
{
enum class CharacterFeetState : uint32_t
{
    Undefined = 0,
    Flat = 1,
    Lifted = 2,
};
}

namespace Raw::CharacterCustomizationFeetController
{
constexpr auto GetOwner = Core::RawFunc<
    /* addr = */ Red::Addresses::CharacterCustomizationFeetController_GetOwner,
    /* type = */ void (*)(Red::game::ui::CharacterCustomizationFeetController* aComponent,
                          Red::Handle<Red::IScriptable>& aOwner)>();

constexpr auto CheckState = Core::RawFunc<
    /* addr = */ Red::Addresses::CharacterCustomizationFeetController_CheckState,
    /* type = */ void (*)(Red::game::ui::CharacterCustomizationFeetController* aComponent,
                          Red::CharacterFeetState& aLiftedState,
                          Red::CharacterFeetState& aFlatState)>();
}
