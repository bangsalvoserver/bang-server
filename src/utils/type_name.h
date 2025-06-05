#ifndef __TYPE_NAME_H__
#define __TYPE_NAME_H__

#include <string>
#include <typeinfo>
#include <typeindex>
#include <format>

#ifdef __GNUG__
#include <cxxabi.h>
#include <memory>

namespace utils {

    inline std::string demangle(const char *name) {
        int status = -4;
        std::unique_ptr<char, void(*)(void*)> res {
            abi::__cxa_demangle(name, nullptr, nullptr, &status),
            std::free
        };
        return status == 0 ? res.get() : name;
    }

}

#else

namespace utils {

    inline std::string demangle(const char *name) {
        return name;
    }

}

#endif

namespace std {
    
    template<std::convertible_to<std::type_index> T>
    struct formatter<T> : formatter<std::string_view> {
        auto format(const std::type_index &type, std::format_context &ctx) const {
            return formatter<std::string_view>::format(utils::demangle(type.name()), ctx);
        }
    };
}

#endif