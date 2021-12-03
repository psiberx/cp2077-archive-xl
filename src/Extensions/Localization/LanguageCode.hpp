#pragma once

#include "Reverse/ResourcePath.hpp"

namespace AXL
{
using LanguageHash = uint64_t;
using LanguageCode = RED4ext::CName;

LanguageCode ResolveLanguageCode(ResourceHash aResource);
}
