#ifndef __JSON_SERIAL_H__
#define __JSON_SERIAL_H__

#include <nlohmann/json.hpp>
#include <fmt/format.h>

#include <vector>
#include <string>
#include <chrono>
#include <map>

namespace json {

    using json = nlohmann::ordered_json;

    template<typename T>
    concept is_complete = requires(T self) { sizeof(self); };

    template<typename T, typename Context = void> struct serializer;

    template<typename T, typename Context = void>
    struct is_serializable : std::bool_constant<is_complete<serializer<T, Context>>> {};

    template<typename T, typename Context = void>
    concept serializable = is_serializable<T, Context>::value;

    template<typename T, typename Context = void> struct deserializer;

    template<typename T, typename Context = void>
    struct is_deserializable : std::bool_constant<is_complete<deserializer<T, Context>>> {};

    template<typename T, typename Context = void>
    concept deserializable = is_deserializable<T, Context>::value;

    using json_error = json::exception;

    struct deserialize_error : json_error {
        deserialize_error(const char *message): json_error(0, message) {}
    };

    template<typename Context>
    struct context_holder {
        const Context &context;

        context_holder(const Context &context) : context(context) {}

        template<serializable<Context> T>
        auto serialize_with_context(const T &value) const {
            if constexpr (requires { serializer<T, Context>{context}; }) {
                return serializer<T, Context>{context}(value);
            } else {
                return serializer<T, void>{}(value);
            }
        }

        template<deserializable<Context> T>
        auto deserialize_with_context(const json &value) const {
            if constexpr (requires { deserializer<T, Context>{context}; }) {
                return deserializer<T, Context>{context}(value);
            } else {
                return deserializer<T, void>{}(value);
            }
        }
    };

    template<> struct context_holder<void> {
        template<serializable T>
        auto serialize_with_context(const T &value) const {
            return serializer<T, void>{}(value);
        }

        template<deserializable T>
        auto deserialize_with_context(const json &value) const {
            return deserializer<T, void>{}(value);
        }
    };

    template<typename T> requires serializable<T>
    json serialize(const T &value) {
        return serializer<T, void>{}(value);
    }

    template<typename T, typename Context> requires serializable<T, Context>
    json serialize(const T &value, const Context &context) {
        return context_holder<Context>{context}.serialize_with_context(value);
    }

    template<typename T> requires deserializable<T>
    T deserialize(const json &value) {
        try {
            return deserializer<T, void>{}(value);
        } catch (const std::exception &e) {
            throw deserialize_error(e.what());
        }
    }

    template<typename T, typename Context> requires deserializable<T, Context>
    T deserialize(const json &value, const Context &context) {
        try {
            return context_holder<Context>{context}.template deserialize_with_context<T>(value);
        } catch (const std::exception &e) {
            throw deserialize_error(e.what());
        }
    }

    template<typename Context>
    struct serializer<json, Context> {
        json operator()(const json &value) const {
            return value;
        }
    };

    template<typename T, typename Context> requires std::is_arithmetic_v<T>
    struct serializer<T, Context> {
        json operator()(const T &value) const {
            return value;
        }
    };

    template<typename Context>
    struct serializer<std::string, Context> {
        json operator()(const std::string &value) const {
            return value;
        }
    };

    template<typename T, typename Context> requires serializable<T, Context>
    struct serializer<std::vector<T>, Context> : context_holder<Context> {
        using context_holder<Context>::context_holder;
        
        json operator()(const std::vector<T> &value) const {
            auto ret = json::array();
            ret.get_ptr<json::array_t*>()->reserve(value.size());
            for (const T &obj : value) {
                ret.push_back(this->serialize_with_context(obj));
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

    template<typename T, typename Context> requires serializable<T, Context>
    struct serializer<std::optional<T>, Context> : context_holder<Context> {
        using context_holder<Context>::context_holder;

        json operator()(const std::optional<T> &value) const {
            if (value) {
                return this->serialize_with_context(*value);
            } else {
                return json{};
            }
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
                    throw std::runtime_error("Cannot deserialize boolean");
                }
            } else if constexpr (std::is_integral_v<T>) {
                if (!value.is_number_integer()) {
                    throw std::runtime_error("Cannot deserialize integer");
                }
            } else {
                if (!value.is_number()) {
                    throw std::runtime_error("Cannot deserialize number");
                }
            }
            return value.get<T>();
        }
    };

    template<typename Context>
    struct deserializer<std::string, Context> {
        std::string operator()(const json &value) const {
            if (!value.is_string()) {
                throw std::runtime_error("Cannot deserialize string");
            }
            return value.get<std::string>();
        }
    };
    
    template<typename T, typename Context> requires deserializable<T, Context>
    struct deserializer<std::vector<T>, Context> : context_holder<Context> {
        using context_holder<Context>::context_holder;
        
        std::vector<T> operator()(const json &value) const {
            if (!value.is_array()) {
                throw std::runtime_error("Cannot deserialize vector");
            }
            std::vector<T> ret;
            ret.reserve(value.size());
            for (const auto &obj : value) {
                ret.push_back(this->template deserialize_with_context<T>(obj));
            }
            return ret;
        }
    };

    template<typename Rep, typename Period, typename Context>
    struct deserializer<std::chrono::duration<Rep, Period>, Context> {
        std::chrono::duration<Rep, Period> operator()(const json &value) const {
            if (!value.is_number()) {
                throw std::runtime_error("Cannot deserialize duration: value is not a number");
            }
            return std::chrono::duration<Rep, Period>{value.get<Rep>()};
        }
    };

    template<typename T, typename Context> requires deserializable<T, Context>
    struct deserializer<std::optional<T>, Context> : context_holder<Context> {
        using context_holder<Context>::context_holder;

        std::optional<T> operator()(const json &value) const {
            if (value.is_null()) {
                return std::nullopt;
            } else {
                return this->template deserialize_with_context<T>(value);
            }
        }
    };
}

#endif