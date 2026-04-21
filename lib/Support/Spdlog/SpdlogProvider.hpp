#pragma once

#include "Core/Foundation/Feature.hpp"
#include "Core/Logging/LoggingDriver.hpp"

namespace Support
{
class SpdlogProvider
    : public Core::Feature
    , public Core::LoggingDriver
{
public:
    void LogInfo(const std::string_view& aMessage) override;
    void LogWarning(const std::string_view& aMessage) override;
    void LogError(const std::string_view& aMessage) override;
    void LogDebug(const std::string_view& aMessage) override;
    void LogFlush() override;

    auto SetLogPath(const std::filesystem::path& aPath) noexcept
    {
        m_baseLogPath = aPath;
        return Defer(this);
    }

    auto AppendTimestampToLogName() noexcept
    {
        m_appendTimestamp = true;
        return Defer(this);
    }

    auto CreateRecentLogSymlink() noexcept
    {
        m_recentSymlink = true;
        return Defer(this);
    }

    auto SetMaxLogFiles(int32_t aCount) noexcept
    {
        m_maxLogCount = aCount;
        return Defer(this);
    }

    auto SetMaxPartSize(size_t aSize) noexcept
    {
        m_maxPartSize = aSize;
        return Defer(this);
    }

    auto SetMaxPartCount(size_t aCount) noexcept
    {
        m_maxPartCount = aCount;
        return Defer(this);
    }

protected:
    void OnInitialize() override;

    std::filesystem::path m_baseLogPath;
    bool m_appendTimestamp{ false };
    bool m_recentSymlink{ false };
    int32_t m_maxLogCount{ 10 };
    size_t m_maxPartSize{ 100 * (1 << 20) };
    size_t m_maxPartCount{ 2 };
};
}
