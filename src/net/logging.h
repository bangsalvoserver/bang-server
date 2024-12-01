#ifndef __LOGGING_H__
#define __LOGGING_H__

#include <format>

#include "utils/enums.h"

namespace logging {

    enum class level {
        all,
        trace,
        debug,
        info,
        status,
        warning,
        error,
        off
    };

    class log_function {
    public:
        constexpr log_function(level local_level): local_level{local_level} {
            if (local_level == level::off || local_level == level::all) {
                throw std::runtime_error("Invalid logging level");
            }
        }

        template<typename ... Ts>
        void operator()(std::format_string<Ts ...> fmt, Ts && ... args) const {
            (*this)(std::format(fmt, std::forward<Ts>(args) ...));
        }

        void operator()(std::string_view message) const;

    private:
        level local_level;
        static level global_level;

        friend void set_logging_level(level global_level);
    };

    void set_logging_level(level global_level);

    constexpr auto trace = log_function(level::trace);
    constexpr auto debug = log_function(level::debug);
    constexpr auto info = log_function(level::info);
    constexpr auto status = log_function(level::status);
    constexpr auto warn = log_function(level::warning);
    constexpr auto error = log_function(level::error);
}

#endif