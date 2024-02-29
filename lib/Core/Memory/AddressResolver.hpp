#pragma once

namespace Core
{
class AddressResolver
{
public:
    virtual uintptr_t ResolveAddress(uint32_t aAddressID) = 0;

    static void SetDefault(AddressResolver& aResolver);
    static AddressResolver& GetDefault();
};
}
