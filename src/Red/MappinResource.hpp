#pragma once

#include "Core/Raw.hpp"
#include "Red/Addresses.hpp"

namespace Raw::MappinResource
{
constexpr auto GetMappinData = Core::RawFunc<
    /* addr = */ Red::Addresses::MappinResource_GetMappinData,
    /* type = */ void* (*)(Red::CResource* aResource, uint32_t aHash)>();
}
