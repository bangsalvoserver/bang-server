#include "logging.h"

#include <chrono>
#include <mutex>
#include <print>

namespace logging {
    std::mutex lock;

    level log_function::global_level = level::status;

    void set_logging_level(level global_level) {
        log_function::global_level = global_level;
    }

    void log_function::do_log(std::string_view message) const {
        std::scoped_lock guard{lock};

        FILE *out = enums::indexof(local_level) >= enums::indexof(level::warning) ? stderr : stdout;

        std::println(out,
            "[{:%Y-%m-%d %H:%M:%S}] [{}] {}",
            std::chrono::time_point_cast<std::chrono::seconds>(std::chrono::system_clock::now()),
            enums::to_string(local_level), message);
        
        fflush(out);
    }
}