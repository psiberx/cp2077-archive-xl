#include "LoggingDriver.hpp"
#include "LoggingAgent.hpp"

#include <cassert>

namespace
{
Core::LoggingDriver* s_default;
}

void Core::LoggingDriver::SetDefault(LoggingDriver& aDriver)
{
    s_default = &aDriver;

    LoggingAgent::SetDriver(aDriver);
}

Core::LoggingDriver& Core::LoggingDriver::GetDefault()
{
    assert(s_default);
    return *s_default;
}
