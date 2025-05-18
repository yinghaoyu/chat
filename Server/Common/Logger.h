#pragma once

#include "Singleton.h"

#include <spdlog/async.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>

class Logger : public Singleton<Logger>
{
    friend Singleton<Logger>;

  public:
    ~Logger() {}

    void Init();

    std::shared_ptr<spdlog::logger> GetLogger() { return logger_; }

  private:
    Logger() { Init(); }

    std::shared_ptr<spdlog::logger> logger_;
};

#ifndef LOG_TRACE
#define LOG_TRACE(...)                                           \
    Logger::GetInstance()->GetLogger()->log(                     \
        spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION}, \
        spdlog::level::trace,                                    \
        __VA_ARGS__)
#endif

#ifndef LOG_DEBUG
#define LOG_DEBUG(...)                                           \
    Logger::GetInstance()->GetLogger()->log(                     \
        spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION}, \
        spdlog::level::debug,                                    \
        __VA_ARGS__)
#endif

#ifndef LOG_INFO
#define LOG_INFO(...)                                            \
    Logger::GetInstance()->GetLogger()->log(                     \
        spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION}, \
        spdlog::level::info,                                     \
        __VA_ARGS__)
#endif

#ifndef LOG_WARN
#define LOG_WARN(...)                                            \
    Logger::GetInstance()->GetLogger()->log(                     \
        spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION}, \
        spdlog::level::warn,                                     \
        __VA_ARGS__)
#endif

#ifndef LOG_ERROR
#define LOG_ERROR(...)                                           \
    Logger::GetInstance()->GetLogger()->log(                     \
        spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION}, \
        spdlog::level::err,                                      \
        __VA_ARGS__)
#endif

#ifndef LOG_CRITICAL
#define LOG_CRITICAL(...)                                        \
    Logger::GetInstance()->GetLogger()->log(                     \
        spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION}, \
        spdlog::level::critical,                                 \
        __VA_ARGS__)
#endif
