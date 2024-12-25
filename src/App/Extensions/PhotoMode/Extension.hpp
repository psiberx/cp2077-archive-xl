#pragma once

#include "App/Extensions/ExtensionBase.hpp"
#include "App/Extensions/PhotoMode/Config.hpp"

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
    void OnActivatePhotoMode(Red::gamePhotoModeSystem* aSystem);
    static void OnSetupGridSelector(Red::gameuiPhotoModeMenuController* aController,
                                    Red::CName aEventName, uint8_t& a3, uint32_t& aAttribute,
                                    Red::DynArray<Red::gameuiPhotoModeOptionGridButtonData>& aGridData,
                                    uint32_t& aElementsCount, uint32_t& aElementsInRow);

    static inline Core::SortedMap<uint32_t, uint32_t> s_extraCharacters;
    static inline uint32_t s_dummyCharacterIndex{0};
};
}
