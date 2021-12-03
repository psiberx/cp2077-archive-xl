#pragma once

#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include <MinHook.h>
#include <spdlog/spdlog.h>
#include <yaml-cpp/yaml.h>

#include <Windows.h>

#include <RED4ext/Hashing/FNV1a.hpp>
#include <RED4ext/Handle.hpp>
#include <RED4ext/CName.hpp>
#include <RED4ext/CString.hpp>
#include <RED4ext/Relocation.hpp>
