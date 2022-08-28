// This file is generated. DO NOT MODIFY IT!
// Created on 2022-08-28 for Cyberpunk 2077 v.1.52.
// Define patterns in "patterns.py" and run "scan.py" to update.

#pragma once

#include <cstdint>

namespace Engine::Addresses
{
constexpr uintptr_t ImageBase = 0x140000000;

constexpr uintptr_t Engine_Main = 0x140191C50 - ImageBase; // 40 53 48 81 EC ? ? ? ? FF 15 ? ? ? ? E8 ? ? ? ? E8 ? ? ? ?, expected: 1, index: 0
constexpr uintptr_t Engine_Init = 0x140A6FC60 - ImageBase; // 48 89 5C 24 10 55 56 41 56 48 8D 6C 24 B9 48 81 EC F0 00 00 00 4C 8B F2 48 8B F1 E8, expected: 1, index: 0
}
