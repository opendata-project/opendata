
#include "log.h"



std::shared_ptr<spdlog::details::thread_pool> g_log_tp;

void InitLogUtil2() {
	const char* pFormat = "[%C/%m/%d %H:%M:%S.%e][%l][PID:%P][tid:%t][%@] %v";
	auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>("/opt/cfs_meta_data/test.log", 1024*1024*10, 10, false);
	file_sink->set_level(spdlog::level::trace);
	file_sink->set_pattern(pFormat);

	auto tp = std::make_shared<spdlog::details::thread_pool>(10240, 1);
	std::shared_ptr<spdlog::logger> pLogger = std::make_shared<spdlog::async_logger>("multi_sink", file_sink, tp, spdlog::async_overflow_policy::block);
	pLogger->set_level((spdlog::level::level_enum)SPDLOG_LEVEL_TRACE);
	pLogger->flush_on(spdlog::level::trace);
	// spdlog::register_logger(pLogger);
	set_default_logger(pLogger);
    g_log_tp = tp;
}


std::shared_ptr<spdlog::logger> g_log_tp2;

void InitLogUtil() {
	const char* pFormat = "[%C/%m/%d %H:%M:%S.%e][%l][PID:%P][tid:%t][%@] %v";

	auto logger = spdlog::rotating_logger_mt("multi_sink", "/opt/cfs_meta_data/test.log", 1024*1024*10, 10);
	logger->set_pattern(pFormat);
	logger->set_level((spdlog::level::level_enum)SPDLOG_LEVEL_TRACE);
	// spdlog::register_logger(logger);
	logger->flush_on(spdlog::level::trace);
	set_default_logger(logger);
    g_log_tp2 = logger;
}
