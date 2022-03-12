#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

#include <yaml-cpp/yaml.h>

#include <RED4ext/CName.hpp>
#include <RED4ext/Handle.hpp>

#include <RED4ext/Api/EMainReason.hpp>
#include <RED4ext/Api/Sdk.hpp>
#include <RED4ext/Api/Version.hpp>

#include "Core/Stl.hpp"
#include "Engine/Stl.hpp"
#include "Engine/Resources/ResourcePath.hpp"

#ifdef VERBOSE
#include "Engine/Log.hpp"
#endif
