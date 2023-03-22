#include "pch.h"
#include "log.h"

namespace Dusty {

    std::shared_ptr<spdlog::logger> Log::core_logger;
    std::shared_ptr<spdlog::logger> Log::client_logger;

    void Log::init() {
        // See https://github.com/gabime/spdlog/wiki/3.-Custom-formatting for format specifiers 
        spdlog::set_pattern("%^[%T] %n: %v%$");
        Log::core_logger = spdlog::stdout_color_mt("CORE");
        Log::core_logger->set_level(spdlog::level::trace);

        Log::client_logger = spdlog::stdout_color_mt("CLIENT");
        Log::client_logger->set_level(spdlog::level::trace);
    }
}