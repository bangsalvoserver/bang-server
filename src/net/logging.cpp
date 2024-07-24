#include "logging.h"

#include <chrono>
#include <mutex>

namespace logging {
    std::mutex lock;

    void log_function::do_log(const std::string &message) const {
        std::scoped_lock guard{lock};

        std::format_to(std::ostream_iterator<char>{out},
            "[{:%Y-%m-%d %H:%M:%S}] [{}] {}\n",
            std::chrono::time_point_cast<std::chrono::seconds>(std::chrono::system_clock::now()),
            enums::to_string(local_level), message);
        
        if (enums::indexof(local_level) <= enums::indexof(level::debug)) {
            out.flush();
        }
    }
}