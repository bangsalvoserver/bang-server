#ifndef __LOGGING_H__
#define __LOGGING_H__

#include <format>
#include <iostream>

#include "utils/enums.h"

namespace logging {

    enum class level {
        all,
        debug,
        info,
        status,
        warning,
        error,
        off
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

    class log_function {
    public:
        static inline level global_level = level::status;
        
        constexpr log_function(level local_level)
            : local_level{local_level}
            , out{enums::indexof(local_level) >= enums::indexof(level::warning) ? std::cerr : std::cout}
        {
            if (local_level == level::off) {
                throw std::runtime_error("Invalid logging level");
            }
        }

        template<typename ... Ts>
        void operator()(std::format_string<Ts ...> fmt, Ts && ... args) const {
            if (enums::indexof(global_level) <= enums::indexof(local_level)) {
                do_log(std::format(fmt, std::forward<Ts>(args) ...));
            }
        }

    private:
        level local_level;
        std::ostream &out;

        void do_log(const std::string &message) const;
    };

    constexpr auto debug = log_function(level::debug);
    constexpr auto info = log_function(level::info);
    constexpr auto status = log_function(level::status);
    constexpr auto warn = log_function(level::warning);
    constexpr auto error = log_function(level::error);
}

#endif