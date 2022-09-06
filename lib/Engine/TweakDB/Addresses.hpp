// This file is generated. DO NOT MODIFY IT!
// Created on 2022-07-16 for Cyberpunk 2077 v.1.52.
// Define patterns in "patterns.py" and run "scan.py" to update.

#pragma once

#include <cstdint>

namespace TweakDB::Addresses
{
constexpr uintptr_t ImageBase = 0x140000000;

constexpr uintptr_t TweakDB_Load = 0x140BC8EF0 - ImageBase; // 48 89 5C 24 18 55 57 41 56 48 8B EC 48 83 EC 70 48 8B D9 45 33 F6 48 8D, expected: 1, index: 0
constexpr uintptr_t TweakDBID_Derive = 0x142B90910 - ImageBase; // 40 53 48 83 EC 30 33 C0 4C 89 44 24 20 48 8B DA, expected: 1, index: 0
}
