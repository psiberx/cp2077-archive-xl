#pragma once

namespace Core
{
class LoggingDriver
{
public:
    virtual void LogInfo(const std::string_view& aMessage) = 0;
    virtual void LogWarning(const std::string_view& aMessage) = 0;
    virtual void LogError(const std::string_view& aMessage) = 0;
    virtual void LogDebug(const std::string_view& aMessage) = 0;
    virtual void LogFlush() = 0;

    template<typename... Args>
    constexpr void LogInfo(std::format_string<Args...> aFormat, Args&&... aArgs)
    {
        LogInfo(std::format(aFormat, std::forward<Args>(aArgs)...));
    }

    template<typename... Args>
    constexpr void LogWarning(std::format_string<Args...> aFormat, Args&&... aArgs)
    {
        LogWarning(std::format(aFormat, std::forward<Args>(aArgs)...));
    }

    template<typename... Args>
    constexpr void LogError(std::format_string<Args...> aFormat, Args&&... aArgs)
    {
        LogError(std::format(aFormat, std::forward<Args>(aArgs)...));
    }

    template<typename... Args>
    constexpr void LogDebug(std::format_string<Args...> aFormat, Args&&... aArgs)
    {
        LogDebug(std::format(aFormat, std::forward<Args>(aArgs)...));
    }

    static void SetDefault(LoggingDriver& aDriver);
    static LoggingDriver& GetDefault();
};
}
