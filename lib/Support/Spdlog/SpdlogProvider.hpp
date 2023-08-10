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
        m_logPath = aPath;
        return Defer(this);
    }

protected:
    void OnInitialize() override;

    std::filesystem::path m_logPath;
};
}
