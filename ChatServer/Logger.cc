#include "Logger.h"
#include "ConfigMgr.h"

#include <memory>

void Logger::Init()
{
    auto&       cfg     = ConfigMgr::Inst();
    const auto& path    = cfg["Logger"]["Path"];
    const auto& level   = cfg["Logger"]["Level"];
    const auto& pattern = cfg["Logger"]["Pattern"];

    _logger = spdlog::basic_logger_mt<spdlog::async_factory>("log", path);
    _logger->set_level(spdlog::level::from_str(level));
    _logger->set_pattern(pattern);
    _logger->flush_on(spdlog::level::from_str(level));
}