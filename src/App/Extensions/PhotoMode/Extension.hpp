#pragma once

#include "App/Extensions/ExtensionBase.hpp"
#include "App/Extensions/PhotoMode/Config.hpp"
#include "Red/PhotoMode.hpp"

namespace App
{
class PhotoModeExtension : public ConfigurableExtensionImpl<PhotoModeConfig>
{
public:
    std::string_view GetName() override;
    bool Load() override;
    bool Unload() override;
    void Configure() override;
    void ApplyTweaks() override;

private:
    struct CharacterDataSource
    {
        uint32_t index;
        uint32_t type;
    };

    static void OnActivatePhotoMode(Red::gamePhotoModeSystem* aSystem);
    static void OnRegisterPoses(Red::gamePhotoModeSystem* aSystem, uint32_t aCharacterIndex, uint32_t aPoseType);
    static void OnRegisterPropPoses1(Red::gamePhotoModeSystem* aSystem, uint32_t aCharacterIndex, uint64_t a3);
    static void OnPreparePoses(Red::gamePhotoModeSystem* aSystem, uint32_t aCharacterIndex, uint32_t a3, uint64_t a4);
    static void OnSetupGridSelector(Red::gameuiPhotoModeMenuController* aController,
                                    Red::CName aEventName, uint8_t& a3, uint32_t& aAttribute,
                                    Red::DynArray<Red::gameuiPhotoModeOptionGridButtonData>& aGridData,
                                    uint32_t& aElementsCount, uint32_t& aElementsInRow);
    static void OnSetNpcImage(void* aCallback, uint32_t aCharacterIndex, Red::ResourcePath aAtlasPath,
                              Red::CName aImagePart, int32_t aImageIndex);

    static inline Core::SortedMap<uint32_t, CharacterDataSource> s_extraCharacters;
    static inline Core::SortedMap<Red::TweakDBID, uint32_t> s_characterIndexMap;
    static inline uint32_t s_dummyCharacterIndex{0};
};
}
