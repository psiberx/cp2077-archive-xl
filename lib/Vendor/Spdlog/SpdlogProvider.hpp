#pragma once

#include "Core/Foundation/Feature.hpp"
#include "Core/Logging/LoggingDriver.hpp"

#include <filesystem>
#include <string>

namespace Vendor
{
class SpdlogProvider : public Core::Feature, public Core::LoggingDriver
{
public:
    struct Options
    {
        std::filesystem::path logPath;
    };

    SpdlogProvider();
    explicit SpdlogProvider(Options&& aOptions);

private:
    void LogInfo(const std::string& aMessage) override;
    void LogWarning(const std::string& aMessage) override;
    void LogError(const std::string& aMessage) override;
    void LogFlush() override;
};
}
