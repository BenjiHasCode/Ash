#pragma once
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace Dusty {
    class Log {
    public:
        static void init();

        static std::shared_ptr<spdlog::logger> core_logger;
        static std::shared_ptr<spdlog::logger> client_logger;
    };
}

#define DUSTY_CORE_WARN(...) Dusty::Log::core_logger->warn(__VA_ARGS__)
#define DUSTY_CORE_INFO(...) Dusty::Log::core_logger->info(__VA_ARGS__)
#define DUSTY_CORE_ERROR(...) Dusty::Log::core_logger->error(__VA_ARGS__)
#define DUSTY_CORE_FATAL(...) Dusty::Log::core_logger->fatal(__VA_ARGS__)   //hmm???

#define DUSTY_CLIENT_WARN(...) Dusty::Log::client_logger->warn(__VA_ARGS__)
#define DUSTY_CLIENT_INFO(...) Dusty::Log::client_logger->info(__VA_ARGS__)
#define DUSTY_CLIENT_ERROR(...) Dusty::Log::client_logger->error(__VA_ARGS__)
#define DUSTY_CLIENT_FATAL(...) Dusty::Log::client_logger->fatal(__VA_ARGS__)   // hmm???