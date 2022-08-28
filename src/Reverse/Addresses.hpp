// This file is generated. DO NOT MODIFY IT!
// Created on 2022-08-28 for Cyberpunk 2077 v.1.52.
// Define patterns in "patterns.py" and run "scan.py" to update.

#pragma once

#include <cstdint>

namespace Reverse::Addresses
{
constexpr uintptr_t ImageBase = 0x140000000;

constexpr uintptr_t GarmentAssembler_AddItem = 0x141C00FC0 - ImageBase; // 48 89 5C 24 08 57 48 83 EC 60 49 8B F8 4C 8B C2 48 8D 54 24 40 E8 ? ? ? ? 48 8B 07, expected: 2, index: 0
constexpr uintptr_t GarmentAssembler_OverrideItem = 0x141C01090 - ImageBase; // 48 89 5C 24 08 57 48 83 EC 60 49 8B F8 4C 8B C2 48 8D 54 24 40 E8 ? ? ? ? 48 8B 07, expected: 2, index: 1
constexpr uintptr_t GarmentAssembler_RemoveItem = 0x141C02FC0 - ImageBase; // 48 89 5C 24 08 57 48 83 EC 40 49 8B F8 4C 8B C2 48 8D 54 24 20 E8 ? ? ? ? 44 8B 47, expected: 1, index: 0
constexpr uintptr_t GarmentAssembler_OnGameDetach = 0x141C037C0 - ImageBase; // 40 53 48 83 EC 20 8B 51 14 48 8B D9 48 8B 49 08 E8 ? ? ? ? C7 43 14 00 00 00 00, expected: 2, index: 1

constexpr uintptr_t AppearanceChanger_ComputePlayerGarment = 0x141C1F000 - ImageBase; // 48 89 5C 24 08 48 89 74 24 10 48 89 7C 24 18 4C 89 64 24 20 55 41 56 41 57 48 8D 6C 24 C0, expected: 1, index: 0

constexpr uintptr_t AppearanceNameVisualTagsPreset_GetVisualTags = 0x14178ECC0 - ImageBase; // 48 89 74 24 18 48 89 54 24 10 57 48 83 EC 40 49 8B F8 48 8D 54 24 20 4C 8D 44 24 58 48 83 C1 40 49 8B F1, expected: 1, index: 0

constexpr uintptr_t AppearanceResource_FindAppearanceDefinition = 0x140FEAC70 - ImageBase; // 48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 57 41 56 41 57 48 83 EC 20 48 8B F1 45 8B F9 48 81 C1 F0 00 00 00 49 8B D8 4C 8B F2, expected: 1, index: 0

constexpr uintptr_t Entity_GetComponents = 0x141035300 - ImageBase; // 48 83 C1 70 E9, expected: 9, index: 1
constexpr uintptr_t Entity_ReassembleAppearance = 0x141030DA0 - ImageBase; // 40 55 53 56 57 41 54 41 55 41 56 41 57 48 8D 6C 24 88 48 81 EC 78 01 00 00 4C 8B EA 49 8B D9, expected: 1, index: 0

constexpr uintptr_t EntityTemplate_FindAppearance = 0x1410513B0 - ImageBase; // 40 53 48 83 EC 20 48 8B  D9 48 85 D2 74 ? 48 3B 15 ? ? ? ? 75 ? 48  8B 51 60 48 3B 15 ? ? ? ? 48 89 7C 24 30 75, expected: 1, index: 0

constexpr uintptr_t FactoryIndex_LoadFactoryAsync = 0x141042D40 - ImageBase; // 48 89 5C 24 18 55 56 57 41 56 41 57 48 8D 6C 24 C9 48 81 EC A0 00 00 00 45 33 FF 48 89 55 DF 4D 8B F0 44 89 7D FB 48 8B D9 4C 89 7D E7, expected: 1, index: 0

constexpr uintptr_t Localization_LoadOnScreens = 0x1406E5BB0 - ImageBase; // 40 55 53 57 48 8B EC 48 81 EC 80 00 00 00 0F 57 C0 48 8B D9 F3 0F 7F 45 A0 48 85 D2 75 17 33 FF 48 8B C1 48 89 39 48 89 79 08 48 81 C4 80 00 00, expected: 2, index: 0

constexpr uintptr_t TagList_MergeWith = 0x1402A9AA0 - ImageBase; // 48 89 5C 24 08 48 89 74 24 10 57 48 83 EC 20 48 8B 1A 48 8B F1 8B 42 0C 48 8D 3C C3 48 3B DF 74, expected: 1, index: 0
}
