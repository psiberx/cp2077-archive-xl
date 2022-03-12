#pragma once

#include <string>
#include <string_view>
#include <utility>
#include <fmt/format.h>
#include <RED4ext/CName.hpp>

namespace Engine::Log
{
void Channel(RED4ext::CName aChannel, const std::string& aMessage);

template<typename... Args>
constexpr void Channel(RED4ext::CName aChannel, std::string_view aFormat, Args&&... aArgs)
{
    Channel(aChannel, fmt::vformat(aFormat, fmt::make_format_args(std::forward<Args>(aArgs)...)));
}

template<typename... Args>
constexpr void Debug(std::string_view aFormat, Args&&... aArgs)
{
    Channel("DEBUG", fmt::vformat(aFormat, fmt::make_format_args(std::forward<Args>(aArgs)...)));
}
}
