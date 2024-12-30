#pragma once

namespace Red
{
enum class PhotoModeCharacterType : uint32_t
{
    Invalid = 0,
    PlayerFemale = 1,
    PlayerMale = 2,
    NPC = 4,
};

struct PhotoModeCharacter
{
    Handle<gamePuppet> puppet;                         // 00
    Handle<entSimpleColliderComponent> collider;       // 10
    Handle<gameIAttachmentSlotsListener> slotListener; // 20
    uint64_t unk30;                                    // 30
    uint64_t unk38;                                    // 38
    uint64_t unk40;                                    // 40
    uint32_t characterIndex;                           // 48
    PhotoModeCharacterType characterType;              // 4C
    uint64_t unk50;                                    // 50
    uint8_t unk58;                                     // 58
    uint8_t unk59;                                     // 59
    uint8_t unk5A;                                     // 5A
    uint8_t unk5B;                                     // 5B
    uint32_t unk5C;                                    // 5C
    uint8_t unk60;                                     // 60
    uint8_t unk61;                                     // 61
    uint8_t unk62;                                     // 62
    uint8_t unk63;                                     // 63
    uint64_t unk68;                                    // 68
    uint64_t unk70;                                    // 70
    uint64_t unk78;                                    // 78
    uint32_t unk80;                                    // 80
    uint16_t unk84;                                    // 84
    uint8_t unk86;                                     // 86
    uint64_t unk88;                                    // 88
    uint64_t unk90;                                    // 90
    uint32_t unk98;                                    // 98
    uint32_t unk9C;                                    // 9C
    uint8_t unkA0;                                     // A0
    uint32_t unkA4;                                    // A4
    uint32_t unkA8;                                    // A8
    uint32_t unkAC;                                    // AC
    uint32_t unkB0;                                    // B0
    uint32_t unkB4;                                    // B4
    uint32_t unkB8;                                    // B8
    bool unkBC;                                        // BC needUpdate?
    CName lookAtCameraPreset;                          // C0
    uint64_t unkC8;                                    // C8
    uint64_t unkD0;                                    // D0
    uint8_t unkD8;                                     // D8
    float unkDC;                                       // DC spawned?
    uint32_t unkE0;                                    // E0
    uint32_t unkE4;                                    // E4
    uint32_t unkE8;                                    // E8
    uint32_t unkEC;                                    // EC
    uint32_t unkF0;                                    // F0
    uint32_t unkF4;                                    // F4
    uint32_t unkF8;                                    // F8
    uint32_t unkFC;                                    // FC
    uint32_t unk100;                                   // 100
    uint32_t unk104;                                   // 104
    uint32_t unk108;                                   // 108
    uint32_t unk10C;                                   // 10C
    uint32_t unk114;                                   // 114
    uint32_t unk118;                                   // 118
    uint32_t unk11C;                                   // 11C
};
RED4EXT_ASSERT_SIZE(PhotoModeCharacter, 0x120);
RED4EXT_ASSERT_OFFSET(PhotoModeCharacter, characterIndex, 0x48);
RED4EXT_ASSERT_OFFSET(PhotoModeCharacter, lookAtCameraPreset, 0xC0);
}

namespace Raw::PhotoModeSystem
{
using CharacterList = Core::OffsetPtr<0x180, Red::DynArray<Red::PhotoModeCharacter>>;
using SpawnedList = Core::OffsetPtr<0x1D0, Red::DynArray<Red::PhotoModeCharacter*>>;

constexpr auto Activate = Core::RawFunc<
    /* addr = */ Red::AddressLib::PhotoModeSystem_Activate,
    /* type = */ bool (*)(Red::gamePhotoModeSystem* aSystem)>();

constexpr auto RegisterCharacter = Core::RawFunc<
    /* addr = */ Red::AddressLib::PhotoModeSystem_RegisterCharacter,
    /* type = */ bool (*)(Red::gamePhotoModeSystem* aSystem,
                          uint32_t aCharacterIndex,
                          Red::DynArray<Red::PhotoModeCharacter>& aCharacterList,
                          Red::PhotoModeCharacterType aCharacterType,
                          Red::DynArray<Red::gamedataItemType>& aItemTypes,
                          Red::DynArray<Red::Handle<Red::gameItemObject>>& aClothingItems)>();

constexpr auto RegisterPoses = Core::RawFunc<
    /* addr = */ Red::AddressLib::PhotoModeSystem_RegisterPoses,
    /* type = */ void (*)(Red::gamePhotoModeSystem* aSystem, uint32_t aCharacterIndex,
                          Red::PhotoModeCharacterType aCharacterType)>();

constexpr auto RegisterWeaponPoses = Core::RawFunc<
    /* addr = */ Red::AddressLib::PhotoModeSystem_RegisterWeaponPoses,
    /* type = */ void (*)(Red::gamePhotoModeSystem* aSystem, uint32_t aCharacterIndex,
                          Red::DynArray<Red::gamedataItemType>& aItemTypes)>();

constexpr auto RegisterClothingItems = Core::RawFunc<
    /* addr = */ Red::AddressLib::PhotoModeSystem_RegisterClothingItems,
    /* type = */ void (*)(Red::gamePhotoModeSystem* aSystem, uint32_t aCharacterIndex,
                          Red::DynArray<Red::Handle<Red::gameItemObject>>& aClothingItems)>();

constexpr auto PrepareCategories = Core::RawFunc<
    /* addr = */ Red::AddressLib::PhotoModeSystem_PrepareCategories,
    /* type = */ void (*)(Red::gamePhotoModeSystem* aSystem, uint32_t aCharacterIndex, uint64_t a3)>();

constexpr auto PreparePoses = Core::RawFunc<
    /* addr = */ Red::AddressLib::PhotoModeSystem_PreparePoses,
    /* type = */ void (*)(Red::gamePhotoModeSystem* aSystem, uint32_t aCharacterIndex, uint32_t aCategoryIndex,
                          uint64_t a4)>();

constexpr auto PrepareCameras = Core::RawFunc<
    /* addr = */ Red::AddressLib::PhotoModeSystem_PrepareCameras,
    /* type = */ void (*)(Red::gamePhotoModeSystem* aSystem, uint32_t aCharacterIndex, uint32_t* a3, uint32_t* a4)>();

constexpr auto UpdateCategoryDependents = Core::RawFunc<
    /* addr = */ Red::AddressLib::PhotoModeSystem_UpdateCategoryDependents,
    /* type = */ void (*)(Red::gamePhotoModeSystem* aSystem, uint32_t aCategoryIndex,
                          Red::PhotoModeCharacter* aCharacter, uint64_t a4)>();

constexpr auto UpdatePoseDependents = Core::RawFunc<
    /* addr = */ Red::AddressLib::PhotoModeSystem_UpdatePoseDependents,
    /* type = */ void (*)(Red::gamePhotoModeSystem* aSystem, uint32_t aCategoryIndex, uint32_t aPoseIndex,
                          Red::PhotoModeCharacter* aCharacter)>();
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
