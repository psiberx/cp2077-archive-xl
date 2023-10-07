#pragma once

#include "LoggingDriver.hpp"

namespace Core
{
class LoggingAgent
{
protected:
    inline static void LogInfo(const char* aMessage)
    {
        GetLoggingDriver().LogInfo(aMessage);
    }

    inline static void LogWarning(const char* aMessage)
    {
        GetLoggingDriver().LogWarning(aMessage);
    }

    inline static void LogError(const char* aMessage)
    {
        GetLoggingDriver().LogError(aMessage);
    }

    inline static void LogDebug(const char* aMessage)
    {
        GetLoggingDriver().LogDebug(aMessage);
    }

    template<typename... Args>
    inline static constexpr void LogInfo(std::format_string<Args...> aFormat, Args&&... aArgs)
    {
        GetLoggingDriver().LogInfo(aFormat, std::forward<Args>(aArgs)...);
    }

    template<typename... Args>
    inline static constexpr void LogWarning(std::format_string<Args...> aFormat, Args&&... aArgs)
    {
        GetLoggingDriver().LogWarning(std::format(aFormat, std::forward<Args>(aArgs)...));
    }

    template<typename... Args>
    inline static constexpr void LogError(std::format_string<Args...> aFormat, Args&&... aArgs)
    {
        GetLoggingDriver().LogError(std::format(aFormat, std::forward<Args>(aArgs)...));
    }

    template<typename... Args>
    inline static constexpr void LogDebug(std::format_string<Args...> aFormat, Args&&... aArgs)
    {
        GetLoggingDriver().LogDebug(std::format(aFormat, std::forward<Args>(aArgs)...));
    }

    static void LogFlush();

    static LoggingDriver& GetLoggingDriver();

private:
    friend LoggingDriver;

    static void SetDriver(LoggingDriver& aDriver);
};
}
