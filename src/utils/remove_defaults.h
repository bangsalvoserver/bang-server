#ifndef __UTILS_REMOVE_DEFAULTS_H__
#define __UTILS_REMOVE_DEFAULTS_H__

#include "json_aggregate.h"

namespace utils {

    template<typename T> requires (std::is_aggregate_v<T> && std::is_default_constructible_v<T>)
    class remove_defaults {
    private:
        T value;
    
    public:
        remove_defaults() = default;

        remove_defaults(const T &value) : value(value) {}
        remove_defaults(T &&value) : value(std::move(value)) {}

        T &get() {
            return value;
        }

        const T &get() const {
            return value;
        }
    };

}

namespace json {

    template<typename T, typename Context> requires all_fields_serializable<T, Context>
    struct serializer<utils::remove_defaults<T>, Context>  {
        json operator()(const utils::remove_defaults<T> &value, const Context &ctx) const {
            json result;
            reflect::for_each<T>([&](auto I) {
                const auto &member_value = reflect::get<I>(value.get());
                if (member_value != reflect::get<I>(default_value_v<T>)) {
                    if (result.is_null()) {
                        result = json::object();
                    }
                    result.push_back({
                        reflect::member_name<I, T>(),
                        serialize_unchecked(member_value, ctx)
                    });
                }
            });
            return result;
        }
    };

    template<typename T, typename Context> requires all_fields_deserializable<T, Context>
    struct deserializer<utils::remove_defaults<T>, Context> {
        using value_type = utils::remove_defaults<T>;

        value_type operator()(const json &value, const Context &ctx) const {
            if (value.is_null()) {
                return value_type{};
            } else {
                return value_type{aggregate_deserializer_unchecked<T, Context>{*this}(value)};
            }
        }
    };
}

#endif