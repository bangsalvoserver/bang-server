#include "logging.h"

#include <chrono>
#include <mutex>
#include <print>

namespace logging {
    std::mutex lock;

    void log_function::operator()(std::string_view message) const {
        if (enums::indexof(global_level) <= enums::indexof(local_level)) {
            std::scoped_lock guard{lock};

            FILE *out = enums::indexof(local_level) >= enums::indexof(level::warning) ? stderr : stdout;

            std::println(out,
                "[{:%Y-%m-%d %H:%M:%S}] [{}] {}",
                std::chrono::time_point_cast<std::chrono::seconds>(std::chrono::system_clock::now()),
                enums::to_string(local_level), message);
            
            fflush(out);
        }
    }
}