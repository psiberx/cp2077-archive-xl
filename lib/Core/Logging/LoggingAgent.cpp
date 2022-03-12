#include "LoggingAgent.hpp"

#include <cassert>

namespace
{
Core::LoggingDriver* s_driver;
}

void Core::LoggingAgent::SetDriver(Core::LoggingDriver& aDriver)
{
    s_driver = &aDriver;
}

Core::LoggingDriver& Core::LoggingAgent::GetLoggingDriver()
{
    assert(s_driver);
    return *s_driver;
}

void Core::LoggingAgent::LogFlush()
{
    s_driver->LogFlush();
}
