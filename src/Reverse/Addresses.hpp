#pragma once

#include "stdafx.hpp"

namespace AXL
{
namespace Addresses
{
    constexpr uintptr_t ImageBase = RED4ext::Addresses::ImageBase;

    // Loads and processes factory csv file in async context.
    // Pattern: 48 89 5C 24 10 48 89 74 24 18 48 89 7C 24 20 55 41 54 41 55 41 56 41 57 48 8D 6C 24 C9 48 81 EC E0 00 00 00 48 8B 02 45 33 ED 4D 8B E0 44 89 6D
    // Signature: void (*)(uintptr_t aThis, uint64_t* aResourceHash, uintptr_t aContext)
    constexpr uintptr_t FactoryIndex_LoadFactoryAsync = 0x1410FD790 - ImageBase;

    // Loads onscreens json file and returns unserialized data.
    // Pattern: 40 55 56 57 48 8B EC 48 81 EC 80 00 00 00 48 83 3A 00 0F 57 C0 F3 0F 7F 45 A0 48 8B F9 75 17 33 F6 48 8B C1 48 89 31 48 89 71 08 48 81 C4 80 00
    // Signature: uint64_t (*)(Handle<PersistenceOnScreenEntries>* aResult, uint64_t* aResourceHash)
    constexpr uintptr_t Localization_LoadOnscreens = 0x140744360 - ImageBase;

    // Sanitizes string input and calculates FNV1a64 hash.
    // I'll reverse the sanitation logic later and move it to RED4ext.SDK.
    // Pattern: 41 56 48 81 EC F0 00 00 00 44 8B 42 08 4C 8B F1 45 85 C0 75 14 48 C7 01 00 00 00 00 48 8B C1 48
    // Signature: uint64_t (*)(uint64_t* aResourceHash, std::string_view* aResourcePath)
    constexpr uintptr_t Resource_MakeHash = 0x14022D370 - ImageBase;
}
}
