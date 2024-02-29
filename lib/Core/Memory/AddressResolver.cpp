#include "AddressResolver.hpp"

#include <cassert>

namespace
{
Core::AddressResolver* s_default;
}

void Core::AddressResolver::SetDefault(Core::AddressResolver& aResolver)
{
    s_default = &aResolver;
}

Core::AddressResolver& Core::AddressResolver::GetDefault()
{
    assert(s_default);
    return *s_default;
}
