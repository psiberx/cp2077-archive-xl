#include "SpdlogProvider.hpp"
#include "Core/Facades/Runtime.hpp"
#include "Core/Stl.hpp"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>

void Support::SpdlogProvider::OnInitialize()
{
    if (m_logPath.empty())
    {
        m_logPath = Core::Runtime::GetModulePath().replace_extension(L".log");
    }

    auto sink = Core::MakeShared<spdlog::sinks::basic_file_sink_mt>(m_logPath.string(), true);
    auto logger = Core::MakeShared<spdlog::logger>("", spdlog::sinks_init_list{sink});
    logger->flush_on(spdlog::level::trace);

    spdlog::set_default_logger(logger);
    spdlog::set_level(spdlog::level::trace);

    SetDefault(*this);
}

void Support::SpdlogProvider::LogInfo(const std::string& aMessage)
{
    spdlog::default_logger_raw()->info(aMessage);
}

void Support::SpdlogProvider::LogWarning(const std::string& aMessage)
{
    spdlog::default_logger_raw()->warn(aMessage);
}

void Support::SpdlogProvider::LogError(const std::string& aMessage)
{
    spdlog::default_logger_raw()->error(aMessage);
}

void Support::SpdlogProvider::LogDebug(const std::string& aMessage)
{
    spdlog::default_logger_raw()->debug(aMessage);
}

void Support::SpdlogProvider::LogFlush()
{
    spdlog::default_logger_raw()->flush();
}
