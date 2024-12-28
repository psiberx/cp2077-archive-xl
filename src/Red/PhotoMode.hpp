#pragma once

namespace Red
{
struct PhotoModeCharacter
{
    uint8_t unk00[0x47];         // 00
    uint32_t characterIndex;     // 48
    uint8_t unk4C[0x120 - 0x4C]; // 4C
};
RED4EXT_ASSERT_SIZE(PhotoModeCharacter, 0x120);
RED4EXT_ASSERT_OFFSET(PhotoModeCharacter, characterIndex, 0x48);
}

namespace Raw::PhotoModeSystem
{
using CharacterList = Core::OffsetPtr<0x180, Red::DynArray<Red::PhotoModeCharacter>>;

constexpr auto Activate = Core::RawFunc<
    /* addr = */ Red::AddressLib::PhotoModeSystem_Activate,
    /* type = */ bool (*)(Red::gamePhotoModeSystem* aSystem)>();

constexpr auto AddCharacter = Core::RawFunc<
    /* addr = */ Red::AddressLib::PhotoModeSystem_RegisterCharacter,
    /* type = */ bool (*)(Red::gamePhotoModeSystem* aSystem,
                          uint32_t aCharacterIndex,
                          const Red::DynArray<Red::PhotoModeCharacter>& aCharacterList,
                          uint32_t aPoseType,
                          const Red::DynArray<void*>& a5,
                          const Red::DynArray<void*>& a6)>();

constexpr auto PrepareAttributes = Core::RawFunc<
    /* addr = */ Red::AddressLib::PhotoModeSystem_PrepareAttributes,
    /* type = */ void (*)(Red::gamePhotoModeSystem* aSystem, uint32_t a2,
                          Red::PhotoModeCharacter* aCharacter, uint64_t a4)>();

constexpr auto CollectPoses = Core::RawFunc<
    /* addr = */ Red::AddressLib::PhotoModeSystem_CollectPoses,
    /* type = */ void (*)(Red::gamePhotoModeSystem* aSystem, uint32_t aCharacterIndex, uint32_t a3, uint64_t a4)>();

constexpr auto PreparePoses = Core::RawFunc<
    /* addr = */ Red::AddressLib::PhotoModeSystem_PreparePoses,
    /* type = */ void (*)(Red::gamePhotoModeSystem* aSystem, uint32_t aCharacterIndex, uint32_t* a3, uint32_t* a4)>();
}

namespace Raw::PhotoModeMenuController
{
constexpr auto SetupGridSelector = Core::RawFunc<
    /* addr = */ Red::AddressLib::PhotoModeMenuController_SetupGridSelector,
    /* type = */ bool (*)(Red::gameuiPhotoModeMenuController* aController, Red::CName aEventName, uint8_t& a3,
                          uint32_t& aAttribute, Red::DynArray<Red::gameuiPhotoModeOptionGridButtonData>& aGridData,
                          uint32_t& aElementsCount, uint32_t& aElementsInRow)>();

constexpr auto SetNpcImageCallback = Core::RawFunc<
    /* addr = */ Red::AddressLib::PhotoModeMenuController_SetNpcImageCallback,
    /* type = */ void (*)(void* aCallback, uint32_t aCharacterIndex, Red::ResourcePath aAtlasPath,
                          Red::CName aImagePart, int32_t aImageIndex)>();
}
