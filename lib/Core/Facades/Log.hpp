#pragma once

#include "Core/Logging/LoggingDriver.hpp"

namespace Core::Log
{
inline void Info(const std::string_view& aMessage)
{
    LoggingDriver::GetDefault().LogInfo(aMessage);
}

inline void Warning(const std::string_view& aMessage)
{
    LoggingDriver::GetDefault().LogWarning(aMessage);
}

inline void Error(const std::string_view& aMessage)
{
    LoggingDriver::GetDefault().LogError(aMessage);
}

inline void Debug(const std::string_view& aMessage)
{
    LoggingDriver::GetDefault().LogDebug(aMessage);
}

template<typename... Args>
constexpr void Info(std::format_string<Args...> aFormat, Args&&... aArgs)
{
    LoggingDriver::GetDefault().LogInfo(std::format(aFormat, std::forward<Args>(aArgs)...));
}

template<typename... Args>
constexpr void Warning(std::format_string<Args...> aFormat, Args&&... aArgs)
{
    LoggingDriver::GetDefault().LogWarning(std::format(aFormat, std::forward<Args>(aArgs)...));
}

template<typename... Args>
constexpr void Error(std::format_string<Args...> aFormat, Args&&... aArgs)
{
    LoggingDriver::GetDefault().LogError(std::format(aFormat, std::forward<Args>(aArgs)...));
}

template<typename... Args>
constexpr void Debug(std::format_string<Args...> aFormat, Args&&... aArgs)
{
    LoggingDriver::GetDefault().LogDebug(std::format(aFormat, std::forward<Args>(aArgs)...));
}
}
