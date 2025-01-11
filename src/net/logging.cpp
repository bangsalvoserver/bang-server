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

    using log_context = std::vector<std::string>;

    static log_context &get_log_context() {
        thread_local log_context ctx{};
        return ctx;
    }

    void push_context(std::string context) {
        get_log_context().push_back(std::move(context));
    }

    void pop_context() {
        get_log_context().pop_back();
    }

    void log_function::do_log(std::string_view message) const {
        std::scoped_lock guard{lock};

        FILE *out = enums::indexof(local_level) >= enums::indexof(level::warning) ? stderr : stdout;

        std::print(out,
            "[{:%Y-%m-%d %H:%M:%S}] [{}]",
            std::chrono::time_point_cast<std::chrono::seconds>(std::chrono::system_clock::now()),
            enums::to_string(local_level)
        );
        for (std::string_view ctx : get_log_context()) {
            std::print(out, "[{}] ", ctx);
        }
        std::println(out, "{}", message);
        
        fflush(out);
    }
}