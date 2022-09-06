#pragma once

#include "Core/Logging/LoggingDriver.hpp"

namespace Core::Log
{
template<typename... Args>
constexpr void Info(std::string_view aFormat, Args&&... aArgs)
{
    LoggingDriver::GetDefault().LogInfo(fmt::vformat(aFormat, fmt::make_format_args(std::forward<Args>(aArgs)...)));
}

template<typename... Args>
constexpr void Warning(std::string_view aFormat, Args&&... aArgs)
{
    LoggingDriver::GetDefault().LogWarning(fmt::vformat(aFormat, fmt::make_format_args(std::forward<Args>(aArgs)...)));
}

template<typename... Args>
constexpr void Error(std::string_view aFormat, Args&&... aArgs)
{
    LoggingDriver::GetDefault().LogError(fmt::vformat(aFormat, fmt::make_format_args(std::forward<Args>(aArgs)...)));
}

template<typename... Args>
constexpr void Debug(std::string_view aFormat, Args&&... aArgs)
{
    LoggingDriver::GetDefault().LogDebug(fmt::vformat(aFormat, fmt::make_format_args(std::forward<Args>(aArgs)...)));
}
}
