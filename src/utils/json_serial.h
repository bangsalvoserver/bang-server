#ifndef __JSON_SERIAL_H__
#define __JSON_SERIAL_H__

#include <nlohmann/json.hpp>

#include <vector>
#include <chrono>

#include "range_utils.h"

namespace json {

    using json = nlohmann::ordered_json;

    template<typename T>
    concept is_complete = requires(T self) { sizeof(self); };

    template<typename T, typename Context> struct serializer;

    template<typename T, typename Context> struct deserializer;

    using json_error = json::exception;

    struct serialize_error : json_error {
        serialize_error(const char *message): json_error(0, message) {}
        serialize_error(const std::string &message): json_error(0, message.c_str()) {}
    };

    struct deserialize_error : json_error {
        deserialize_error(const char *message): json_error(0, message) {}
        deserialize_error(const std::string &message): json_error(0, message.c_str()) {}
    };

    struct no_context {};

    template<typename T, typename Context>
    json serialize_unchecked(const T &value, const Context &context) {
        serializer<T, Context> obj{};
        if constexpr (requires { obj(value, context); }) {
            return obj(value, context);
        } else {
            return obj(value);
        }
    }

    template<typename T, typename Context>
    json serialize(const T &value, const Context &context) {
        try {
            return serialize_unchecked(value, context);
        } catch (const serialize_error &) {
            throw;
        } catch (const std::exception &e) {
            throw serialize_error(e.what());
        }
    }

    template<typename T>
    json serialize(const T &value) {
        return serialize(value, no_context{});
    }

    template<typename T, typename Context>
    auto deserialize_unchecked(const json &value, const Context &context) {
        deserializer<T, Context> obj{};
        if constexpr (requires { obj(value, context); }) {
            return obj(value, context);
        } else {
            return obj(value);
        }
    }

    template<typename T, typename Context>
    T deserialize(const json &value, const Context &context) {
        try {
            return deserialize_unchecked<T>(value, context);
        } catch (const deserialize_error &) {
            throw;
        } catch (const std::exception &e) {
            throw deserialize_error(e.what());
        }
    }

    template<typename T>
    T deserialize(const json &value) {
        return deserialize<T>(value, no_context{});
    }

    template<typename Context>
    struct serializer<json, Context> {
        json operator()(const json &value) const {
            return value;
        }
    };

    template<std::integral T, typename Context>
    struct serializer<T, Context> {
        json operator()(T value) const {
            return value;
        }
    };

    template<std::floating_point T, typename Context>
    struct serializer<T, Context> {
        json operator()(T value) const {
            return value;
        }
    };

    template<std::convertible_to<std::string_view> T, typename Context>
    struct serializer<T, Context> {
        json operator()(std::string_view value) const {
            return value;
        }
    };

    template<rn::range Range, typename Context> requires (!std::convertible_to<Range, std::string_view>)
    struct serializer<Range, Context> {
        json operator()(const Range &value, const Context &ctx) const {
            auto ret = json::array();
            if constexpr (rn::sized_range<Range>) {
                ret.get_ptr<json::array_t*>()->reserve(value.size());
            }
            for (const auto &obj : value) {
                ret.push_back(serialize_unchecked(obj, ctx));
            }
            return ret;
        }
    };

    template<typename Rep, typename Period, typename Context>
    struct serializer<std::chrono::duration<Rep, Period>, Context> {
        json operator()(const std::chrono::duration<Rep, Period> &value) const {
            return value.count();
        }
    };

    template<typename Clock, typename Duration, typename Context>
    struct serializer<std::chrono::time_point<Clock, Duration>, Context> {
        json operator()(const std::chrono::time_point<Clock, Duration> &value, const Context &ctx) const {
            return serialize_unchecked(value.time_since_epoch(), ctx);
        }
    };

    template<typename T, typename Context>
    struct serializer<std::optional<T>, Context> {
        json operator()(const std::optional<T> &value, const Context &ctx) const {
            if (value) {
                return serialize_unchecked(*value, ctx);
            } else {
                return json{};
            }
        }
    };

    template<typename First, typename Second, typename Context>
    struct serializer<std::pair<First, Second>, Context> {
        json operator()(const std::pair<First, Second> &value, const Context &ctx) const {
            return json::array({
                serialize_unchecked(value.first, ctx),
                serialize_unchecked(value.second, ctx)
            });
        }
    };

    template<typename Context, typename ... Ts>
    struct serializer<std::tuple<Ts ...>, Context> {
        json operator()(const std::tuple<Ts ...> &value, const Context &ctx) const {
            return [&]<size_t ... Is>(std::index_sequence<Is ...>) {
                return json::array({
                    serialize_unchecked(std::get<Is>(value), ctx) ...
                });
            }(std::index_sequence_for<Ts ...>());
        }
    };
    
    template<typename Context>
    struct deserializer<json, Context> {
        json operator()(const json &value) const {
            return value;
        }
    };

    template<typename T, typename Context> requires std::is_arithmetic_v<T>
    struct deserializer<T, Context> {
        T operator()(const json &value) const {
            if constexpr (std::is_same_v<T, bool>) {
                if (!value.is_boolean()) {
                    throw deserialize_error("Cannot deserialize boolean");
                }
            } else if constexpr (std::is_integral_v<T>) {
                if (!value.is_number_integer()) {
                    throw deserialize_error("Cannot deserialize integer");
                }
            } else {
                if (!value.is_number()) {
                    throw deserialize_error("Cannot deserialize number");
                }
            }
            return value.get<T>();
        }
    };

    template<typename Context>
    struct deserializer<std::string, Context> {
        std::string operator()(const json &value) const {
            if (!value.is_string()) {
                throw deserialize_error("Cannot deserialize string");
            }
            return value.get<std::string>();
        }
    };
    
    template<rn::range Range, typename Context>
    struct deserializer<Range, Context> {
        Range operator()(const json &value, const Context &ctx) const {
            if (!value.is_array()) {
                throw deserialize_error("Cannot deserialize range");
            }
            return value
                | rv::transform([&](const json &obj) {
                    return deserialize_unchecked<rn::range_value_t<Range>>(obj, ctx);
                })
                | rn::to<Range>;
        }
    };

    template<typename Rep, typename Period, typename Context>
    struct deserializer<std::chrono::duration<Rep, Period>, Context> {
        std::chrono::duration<Rep, Period> operator()(const json &value) const {
            if (!value.is_number()) {
                throw deserialize_error("Cannot deserialize duration: value is not a number");
            }
            return std::chrono::duration<Rep, Period>{value.get<Rep>()};
        }
    };
    
    template<typename Clock, typename Duration, typename Context>
    struct deserializer<std::chrono::time_point<Clock, Duration>, Context> {
        std::chrono::time_point<Clock, Duration> operator()(const json &value, const Context &ctx) const {
            return std::chrono::time_point<Clock, Duration>{ deserialize_unchecked<Duration>(value, ctx) };
        }
    };

    template<typename T, typename Context>
    struct deserializer<std::optional<T>, Context> {
        std::optional<T> operator()(const json &value, const Context &ctx) const {
            if (value.is_null()) {
                return std::nullopt;
            } else {
                return deserialize_unchecked<T>(value, ctx);
            }
        }
    };

    template<typename First, typename Second, typename Context>
    struct deserializer<std::pair<First, Second>, Context> {
        std::pair<First, Second> operator()(const json &value, const Context &ctx) const {
            if (!value.is_array() || value.size() != 2) {
                throw deserialize_error("Cannot deserialize pair: value is not an array of two elements");
            }
            return {
                deserialize_unchecked<First>(value[0], ctx),
                deserialize_unchecked<Second>(value[1], ctx)
            };
        }
    };

    template<typename Context, typename ...Ts>
    struct deserializer<std::tuple<Ts ...>, Context> {
        std::tuple<Ts ...> operator()(const json &value, const Context &ctx) const {
            if (!value.is_array() || value.size() != sizeof...(Ts)) {
                throw deserialize_error("Cannot deserialize tuple: invalid size of array");
            }
            return [&]<size_t ... Is>(std::index_sequence<Is ...>) {
                return std::make_tuple(deserialize_unchecked<Ts>(value[Is], ctx) ...);
            }(std::index_sequence_for<Ts ...>());
        }
    };
}

#endif