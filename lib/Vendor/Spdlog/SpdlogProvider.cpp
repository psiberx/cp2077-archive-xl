#include "SpdlogProvider.hpp"
#include "Core/Stl.hpp"
#include "Core/Facades/Runtime.hpp"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>

Vendor::SpdlogProvider::SpdlogProvider()
    : SpdlogProvider(Options{})
{
}

Vendor::SpdlogProvider::SpdlogProvider(Vendor::SpdlogProvider::Options&& aOptions)
{
    if (aOptions.logPath.empty())
        aOptions.logPath = Core::Runtime::GetModulePath().replace_extension(L".log");

    auto sink = Core::MakeShared<spdlog::sinks::basic_file_sink_mt>(aOptions.logPath.string(), true);
    auto logger = Core::MakeShared<spdlog::logger>("", spdlog::sinks_init_list{sink});
    logger->flush_on(spdlog::level::trace);

    spdlog::set_default_logger(logger);
    spdlog::set_level(spdlog::level::trace);

    SetDefault(*this);
}

void Vendor::SpdlogProvider::LogInfo(const std::string& aMessage)
{
    spdlog::default_logger_raw()->info(aMessage);
}

void Vendor::SpdlogProvider::LogWarning(const std::string& aMessage)
{
    spdlog::default_logger_raw()->warn(aMessage);
}

void Vendor::SpdlogProvider::LogError(const std::string& aMessage)
{
    spdlog::default_logger_raw()->error(aMessage);
}

void Vendor::SpdlogProvider::LogFlush()
{
    spdlog::default_logger_raw()->flush();
}
