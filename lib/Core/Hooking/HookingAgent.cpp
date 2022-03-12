#include "HookingAgent.hpp"

#include <cassert>

namespace
{
Core::HookingDriver* s_driver;
}

void Core::HookingAgent::SetHookingDriver(Core::HookingDriver& aDriver)
{
    s_driver = &aDriver;
}

Core::HookingDriver& Core::HookingAgent::GetHookingDriver()
{
    assert(s_driver);
    return *s_driver;
}
