#pragma once

#include <cstdint>

namespace Engine::Addresses
{
constexpr uintptr_t ImageBase = 0x140000000;

constexpr uintptr_t Engine_Main = 0x140191C50 - ImageBase; // 40 53 48 81 EC ? ? ? ? FF 15 ? ? ? ? E8 ? ? ? ? E8 ? ? ? ?, expected: 1, index: 0
constexpr uintptr_t ResourceReference_Load = 0x140247CF0 - ImageBase; // 48 89 5C 24 10 57 48 83 EC 30 48 8D 59 08 48 8B F9 48 8B 0B 48 85 C9, expected: 1, index: 0
constexpr uintptr_t ResourceReference_Fetch = 0x140247DA0 - ImageBase; // 40 53 48 83 EC 20 48 8B D9 E8 ? ? ? ? 48 8B 4B 08 48 85 C9 74 0A, expected: 1, index: 0
constexpr uintptr_t ResourceReference_Reset = 0x140247C80 - ImageBase; // 48 83 EC 38 45 33 C0 4C 89 01 48 8B 51 10 48 8B 41 08 48 89 54 24 28 4C 89 41 10 48 89 44 24 20, expected: 1, index: 0
}
