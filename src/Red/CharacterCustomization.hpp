#pragma once

#include "Red/AppearanceChanger.hpp"

namespace Red
{
enum class CharacterBodyPartState : uint32_t
{
    Undefined = 0,
    Hidden = 1,
    Visible = 2,
};
}

namespace Raw::CharacterCustomizationHairstyleController
{
constexpr auto OnDetach = Core::RawFunc<
    /* addr = */ Red::AddressLib::CharacterCustomizationHairstyleController_OnDetach,
    /* type = */ void (*)(Red::game::ui::CharacterCustomizationHairstyleController* aComponent, uintptr_t a2)>();

constexpr auto CheckState = Core::RawFunc<
    /* addr = */ Red::AddressLib::CharacterCustomizationHairstyleController_CheckState,
    /* type = */ void (*)(Red::game::ui::CharacterCustomizationHairstyleController* aComponent,
                          Red::CharacterBodyPartState& aHairState)>();
}

namespace Raw::CharacterCustomizationGenitalsController
{
constexpr auto OnAttach = Core::RawFunc<
    /* addr = */ Red::AddressLib::CharacterCustomizationGenitalsController_OnAttach,
    /* type = */ void (*)(Red::game::ui::CharacterCustomizationGenitalsController* aComponent)>();

constexpr auto CheckState = Core::RawFunc<
    /* addr = */ Red::AddressLib::CharacterCustomizationGenitalsController_CheckState,
    /* type = */ void (*)(Red::game::ui::CharacterCustomizationGenitalsController* aComponent,
                          Red::CharacterBodyPartState& aUpperState,
                          Red::CharacterBodyPartState& aBottomState)>();
}

namespace Raw::CharacterCustomizationFeetController
{
// constexpr auto GetOwner = Core::RawFunc<
//     /* addr = */ Red::AddressLib::CharacterCustomizationFeetController_GetOwner,
//     /* type = */ void (*)(Red::game::ui::CharacterCustomizationFeetController* aComponent,
//                           Red::Handle<Red::Entity>& aOwner)>();

constexpr auto CheckState = Core::RawFunc<
    /* addr = */ Red::AddressLib::CharacterCustomizationFeetController_CheckState,
    /* type = */ void (*)(Red::game::ui::CharacterCustomizationFeetController* aComponent,
                          Red::CharacterBodyPartState& aLiftedState,
                          Red::CharacterBodyPartState& aFlatState)>();
}

namespace Raw::CharacterCustomizationState
{
using HeadGroups = Core::OffsetPtr<0x70, Red::DynArray<Red::gameuiCustomizationGroup>>;
using BodyGroups = Core::OffsetPtr<0x80, Red::DynArray<Red::gameuiCustomizationGroup>>;
using ArmsGroups = Core::OffsetPtr<0x90, Red::DynArray<Red::gameuiCustomizationGroup>>;

constexpr auto GetHeadAppearances1 = Core::RawFunc<
    /* addr = */ Red::AddressLib::CharacterCustomizationState_GetHeadAppearances1,
    /* type = */ void (*)(Red::gameuiICharacterCustomizationState* aState,
                          Red::CName aGroupName, bool aIsFPP,
                          Red::DynArray<Red::AppearanceDescriptor>& aAppearances)>();

constexpr auto GetHeadAppearances2 = Core::RawFunc<
    /* addr = */ Red::AddressLib::CharacterCustomizationState_GetHeadAppearances2,
    /* type = */ void (*)(Red::gameuiICharacterCustomizationState* aState,
                          Red::CName aGroupName, bool aIsFPP,
                          Red::DynArray<Red::AppearanceDescriptor>& aAppearances)>();

constexpr auto GetBodyAppearances1 = Core::RawFunc<
    /* addr = */ Red::AddressLib::CharacterCustomizationState_GetBodyAppearances1,
    /* type = */ void (*)(Red::gameuiICharacterCustomizationState* aState,
                          Red::CName aGroupName, bool aIsFPP,
                          Red::DynArray<Red::AppearanceDescriptor>& aAppearances)>();

constexpr auto GetBodyAppearances2 = Core::RawFunc<
    /* addr = */ Red::AddressLib::CharacterCustomizationState_GetBodyAppearances2,
    /* type = */ void (*)(Red::gameuiICharacterCustomizationState* aState,
                          Red::CName aGroupName, bool aIsFPP,
                          Red::DynArray<Red::AppearanceDescriptor>& aAppearances)>();

constexpr auto GetArmsAppearances1 = Core::RawFunc<
    /* addr = */ Red::AddressLib::CharacterCustomizationState_GetArmsAppearances1,
    /* type = */ void (*)(Red::gameuiICharacterCustomizationState* aState,
                          Red::CName aGroupName, bool aIsFPP,
                          Red::DynArray<Red::AppearanceDescriptor>& aAppearances)>();

constexpr auto GetArmsAppearances2 = Core::RawFunc<
    /* addr = */ Red::AddressLib::CharacterCustomizationState_GetArmsAppearances2,
    /* type = */ void (*)(Red::gameuiICharacterCustomizationState* aState,
                          Red::CName aGroupName, bool aIsFPP,
                          Red::DynArray<Red::AppearanceDescriptor>& aAppearances)>();
}

namespace Raw::CharacterCustomizationSystem
{
using MaleResource = Core::OffsetPtr<0x48, Red::SharedPtr<Red::ResourceToken<Red::gameuiCharacterCustomizationInfoResource>>>;
using FemaleResource = Core::OffsetPtr<0x58, Red::SharedPtr<Red::ResourceToken<Red::gameuiCharacterCustomizationInfoResource>>>;
using StateLock = Core::OffsetPtr<0x70, Red::SharedSpinLock>;
using State = Core::OffsetPtr<0x78, Red::Handle<Red::gameuiCharacterCustomizationState>>;

constexpr auto Initialize = Core::RawFunc<
    /* addr = */ Red::AddressLib::CharacterCustomizationSystem_Initialize,
    /* type = */ void (*)(Red::gameuiICharacterCustomizationSystem* aSystem,
                          Red::Handle<Red::game::Puppet>& aPuppet, bool aIsMale, uintptr_t a4)>();

constexpr auto Uninitialize = Core::RawFunc<
    /* addr = */ Red::AddressLib::CharacterCustomizationSystem_Uninitialize,
    /* type = */ void (*)(Red::gameuiICharacterCustomizationSystem* aSystem)>();

constexpr auto GetResource = Core::RawFunc<
    /* addr = */ Red::AddressLib::CharacterCustomizationSystem_GetResource,
    /* type = */ void (*)(Red::gameuiICharacterCustomizationSystem* aSystem,
                          Red::SharedPtr<Red::ResourceToken<Red::gameuiCharacterCustomizationInfoResource>>& aOut,
                          bool aIsMale)>();

constexpr auto InitializeAppOption = Core::RawFunc<
    /* addr = */ Red::AddressLib::CharacterCustomizationSystem_InitializeAppOption,
    /* type = */ void (*)(Red::gameuiICharacterCustomizationSystem* aSystem,
                          Red::game::ui::CharacterCustomizationPart aPartType,
                          Red::Handle<Red::game::ui::CharacterCustomizationOption>& aOption,
                          Red::SortedUniqueArray<Red::CName>& aStateOptions,
                          Red::Map<Red::CName, Red::Handle<Red::game::ui::CharacterCustomizationOption>>& aUiSlots)>();

constexpr auto InitializeMorphOption = Core::RawFunc<
    /* addr = */ Red::AddressLib::CharacterCustomizationSystem_InitializeMorphOption,
    /* type = */ void (*)(Red::gameuiICharacterCustomizationSystem* aSystem,
                          Red::Handle<Red::game::ui::CharacterCustomizationOption>& aOption,
                          Red::SortedUniqueArray<Red::CName>& aStateOptions,
                          Red::Map<Red::CName, Red::Handle<Red::game::ui::CharacterCustomizationOption>>& aUiSlots)>();

constexpr auto InitializeSwitcherOption = Core::RawFunc<
    /* addr = */ Red::AddressLib::CharacterCustomizationSystem_InitializeSwitcherOption,
    /* type = */ bool (*)(Red::gameuiICharacterCustomizationSystem* aSystem,
                          Red::game::ui::CharacterCustomizationPart aPartType,
                          Red::Handle<Red::game::ui::CharacterCustomizationOption>& aOption,
                          int32_t aCurrentIndex,
                          uint64_t a5,
                          Red::Map<Red::CName, Red::Handle<Red::game::ui::CharacterCustomizationOption>>& aUiSlots)>();
}

namespace Raw::CharacterCustomizationHelper
{
constexpr auto GetHairColor = Core::RawFunc<
    /* addr = */ Red::AddressLib::CharacterCustomizationHelper_GetHairColor,
    /* type = */ void (*)(Red::CName& aOut, Red::WeakHandle<Red::ISerializable>& aSystem, bool aIsMale)>();
}
