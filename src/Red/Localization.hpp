#pragma once

namespace Red
{
constexpr auto LocKeyPrefix = "LocKey#";
constexpr auto LocKeyPrefixLength = std::char_traits<char>::length(LocKeyPrefix);
}

namespace Raw::Localization
{
using VoiceOverTokens = Core::OffsetPtr<0x8, Red::Map<uint32_t, Red::SharedPtr<Red::ResourceToken<Red::JsonResource>>>>;
using LipMapToken = Core::OffsetPtr<0x188, Red::SharedPtr<Red::ResourceToken<Red::animLipsyncMapping>>>;

constexpr auto LoadTexts = Core::RawFunc<
    /* addr = */ Red::AddressLib::Localization_LoadOnScreens,
    /* type = */ uint64_t (*)(Red::Handle<Red::localization::PersistenceOnScreenEntries>& aOut,
                              Red::ResourcePath aPath)>();

constexpr auto LoadSubtitles = Core::RawFunc<
    /* addr = */ Red::AddressLib::Localization_LoadSubtitles,
    /* type = */ uint64_t (*)(Red::Handle<Red::localization::PersistenceSubtitleMap>& aOut,
                              Red::ResourcePath aPath)>();

constexpr auto LoadVoiceOvers = Core::RawFunc<
    /* addr = */ Red::AddressLib::Localization_LoadVoiceOvers,
    /* type = */ void (*)(void* aContext, uint64_t a2)>();

constexpr auto LoadLipsyncs = Core::RawFunc<
    /* addr = */ Red::AddressLib::Localization_LoadLipsyncs,
    /* type = */ void (*)(void* aContext, uint8_t a2)>();
}
