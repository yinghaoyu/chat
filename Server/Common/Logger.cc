#include "Logger.h"
#include "ConfigMgr.h"

#include <memory>

void Logger::Init()
{
    auto&       cfg     = ConfigMgr::Inst();
    const auto& path    = cfg["Logger"]["Path"];
    const auto& level   = cfg["Logger"]["Level"];
    const auto& pattern = cfg["Logger"]["Pattern"];

    logger_ = spdlog::basic_logger_mt<spdlog::async_factory>("log", path);
    logger_->set_level(spdlog::level::from_str(level));
    logger_->set_pattern(pattern);
    logger_->flush_on(spdlog::level::from_str(level));
}
