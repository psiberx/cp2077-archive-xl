#pragma once

#include "Core/Memory/Common.hpp"

namespace Core
{
class AddressResolver
{
public:
    virtual uintptr_t ResolveAddress(AddressSegment aSegment, uint32_t aAddressHash) = 0;

    static void SetDefault(AddressResolver& aResolver);
    static AddressResolver& GetDefault();
};
}
