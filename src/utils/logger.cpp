#include "logger.h"

void configlog(spdlog::level::level_enum level)
{
    std::vector<spdlog::sink_ptr> sinks;

    auto stdout_sink = std::make_shared<spdlog::sinks::ansicolor_stdout_sink_mt>();
    stdout_sink->set_level(spdlog::level::debug);
    sinks.push_back(stdout_sink);

    auto daily_debug_sink = std::make_shared<spdlog::sinks::daily_file_format_sink_mt>("logs/log_debug.log", 2, 30);
    daily_debug_sink->set_level(spdlog::level::debug);
    sinks.push_back(daily_debug_sink);

    auto daily_info_sink = std::make_shared<spdlog::sinks::daily_file_format_sink_mt>("logs/log_info.log", 2, 30);
    daily_info_sink->set_level(spdlog::level::info);
    sinks.push_back(daily_info_sink);

    auto daily_err_sink = std::make_shared<spdlog::sinks::daily_file_format_sink_mt>("logs/log_error.log", 2, 30);
    daily_err_sink->set_level(spdlog::level::err);
    sinks.push_back(daily_err_sink);

    auto logger = std::make_shared<spdlog::logger>("mylogger", begin(sinks), end(sinks));
    spdlog::register_logger(logger);
    spdlog::set_default_logger(logger);
    spdlog::set_level(level);
    spdlog::flush_every(std::chrono::seconds(1));
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [t_%t] [%n] [%^%l%$] > %v");
}
