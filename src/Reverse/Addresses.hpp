#pragma once

#include <cstdint>

namespace Reverse::Addresses
{
    constexpr uintptr_t ImageBase = 0x140000000;

    constexpr uintptr_t FactoryIndex_LoadFactoryAsync = 0x141042B40 - ImageBase; // 48 89 5C 24 18 55 56 57 41 56 41 57 48 8D 6C 24 C9 48 81 EC A0 00 00 00 45 33 FF 48 89 55 DF 4D 8B F0 44 89 7D FB 48 8B D9 4C 89 7D E7, expected: 1, index: 0
    constexpr uintptr_t Localization_LoadOnScreens = 0x1406E5640 - ImageBase; // 40 55 53 57 48 8B EC 48 81 EC 80 00 00 00 0F 57 C0 48 8B D9 F3 0F 7F 45 A0 48 85 D2 75 17 33 FF 48 8B C1 48 89 39 48 89 79 08 48 81 C4 80 00 00, expected: 2, index: 0
}
