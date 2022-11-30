#pragma once

#include <fmt/format.h>

namespace Red::Log
{
void Channel(CName aChannel, const std::string& aMessage);

template<typename... Args>
constexpr void Channel(CName aChannel, std::string_view aFormat, Args&&... aArgs)
{
    Channel(aChannel, fmt::vformat(aFormat, fmt::make_format_args(std::forward<Args>(aArgs)...)));
}

template<typename... Args>
constexpr void Debug(std::string_view aFormat, Args&&... aArgs)
{
    Channel("DEBUG", fmt::vformat(aFormat, fmt::make_format_args(std::forward<Args>(aArgs)...)));
}
}
