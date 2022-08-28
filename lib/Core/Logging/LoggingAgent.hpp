#pragma once

#include "LoggingDriver.hpp"

namespace Core
{
class LoggingAgent
{
protected:
    template<typename... Args>
    inline static void LogInfo(std::string_view aFormat, Args&&... aArgs)
    {
        GetLoggingDriver().LogInfo(fmt::vformat(aFormat, fmt::make_format_args(std::forward<Args>(aArgs)...)));
    }

    template<typename... Args>
    inline static void LogWarning(std::string_view aFormat, Args&&... aArgs)
    {
        GetLoggingDriver().LogWarning(fmt::vformat(aFormat, fmt::make_format_args(std::forward<Args>(aArgs)...)));
    }

    template<typename... Args>
    inline static void LogError(std::string_view aFormat, Args&&... aArgs)
    {
        GetLoggingDriver().LogError(fmt::vformat(aFormat, fmt::make_format_args(std::forward<Args>(aArgs)...)));
    }

    template<typename... Args>
    inline static void LogDebug(std::string_view aFormat, Args&&... aArgs)
    {
        GetLoggingDriver().LogDebug(fmt::vformat(aFormat, fmt::make_format_args(std::forward<Args>(aArgs)...)));
    }

    static void LogFlush();

    static LoggingDriver& GetLoggingDriver();

private:
    friend LoggingDriver;

    static void SetDriver(LoggingDriver& aDriver);
};
}
