#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

#include <yaml-cpp/yaml.h>

#include <RED4ext/CName.hpp>
#include <RED4ext/DynArray.hpp>
#include <RED4ext/Handle.hpp>
#include <RED4ext/HashMap.hpp>
#include <RED4ext/ResourcePath.hpp>

#include <RED4ext/Api/EMainReason.hpp>
#include <RED4ext/Api/Sdk.hpp>
#include <RED4ext/Api/Runtime.hpp>
#include <RED4ext/Api/SemVer.hpp>
#include <RED4ext/Api/Version.hpp>

#include "Core/Stl.hpp"
#include "Engine/Stl.hpp"

#ifdef VERBOSE
#include "Engine/Log.hpp"
#endif
