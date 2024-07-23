#ifndef __LOGGING_H__
#define __LOGGING_H__

#include <format>
#include <iostream>
#include <chrono>

#include "utils/parse_string.h"

namespace logging {

    enum class level {
        all,
        debug,
        info,
        trace,
        warning,
        error
    };

    inline std::istream &operator >> (std::istream &input, logging::level &result) {
        std::string text;
        input >> text;
        if (auto value = enums::from_string<logging::level>(text)) {
            result = *value;
        } else {
            throw std::runtime_error(std::format("Invalid logging level: {}", text));
        }
        return input;
    };

    inline level g_logging_level = level::trace;

    inline std::ostream &get_level_ostream(level l) {
        switch (l) {
        case level::warning:
        case level::error:
            return std::cerr;
        default:
            return std::cout;
        }
    }

    template<typename ... Ts>
    void log(level l, std::format_string<Ts ...> fmt, Ts && ... args) {
        if (enums::indexof(g_logging_level) <= enums::indexof(l)) {
            auto now = std::chrono::time_point_cast<std::chrono::seconds>(std::chrono::system_clock::now());
            get_level_ostream(l)
                << std::format("[{:%Y-%m-%d %H:%M:%S}] [{}] {}", now, l,
                    std::format(fmt, std::forward<Ts>(args) ...))
                << std::endl;
        }
    }

    template<typename ... Ts>
    void debug(std::format_string<Ts ...> fmt, Ts && ... args) {
        log(level::debug, fmt, std::forward<Ts>(args) ...);
    }

    template<typename ... Ts>
    void info(std::format_string<Ts ...> fmt, Ts && ... args) {
        log(level::info, fmt, std::forward<Ts>(args) ...);
    }

    template<typename ... Ts>
    void trace(std::format_string<Ts ...> fmt, Ts && ... args) {
        log(level::trace, fmt, std::forward<Ts>(args) ...);
    }

    template<typename ... Ts>
    void warn(std::format_string<Ts ...> fmt, Ts && ... args) {
        log(level::warning, fmt, std::forward<Ts>(args) ...);
    }

    template<typename ... Ts>
    void error(std::format_string<Ts ...> fmt, Ts && ... args) {
        log(level::error, fmt, std::forward<Ts>(args) ...);
    }
}

#endif