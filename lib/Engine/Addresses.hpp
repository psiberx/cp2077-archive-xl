#pragma once

#include <cstdint>

namespace Engine::Addresses
{
constexpr uintptr_t ImageBase = 0x140000000;

constexpr uintptr_t Engine_Main = 0x140191BD0 - ImageBase; // 40 53 48 81 EC ? ? ? ? FF 15 ? ? ? ? E8 ? ? ? ? E8 ? ? ? ?, expected: 1, index: 0
}
