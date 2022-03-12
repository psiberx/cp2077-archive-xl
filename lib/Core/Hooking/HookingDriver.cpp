#include "HookingDriver.hpp"
#include "HookingAgent.hpp"

#include <cassert>

namespace
{
Core::HookingDriver* s_default;
}

void Core::HookingDriver::SetDefault(Core::HookingDriver& aDriver)
{
    s_default = &aDriver;

    HookingAgent::SetHookingDriver(aDriver);
}

Core::HookingDriver& Core::HookingDriver::GetDefault()
{
    assert(s_default);
    return *s_default;
}
