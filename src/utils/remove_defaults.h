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
    struct serializer<utils::remove_defaults<T>, Context> : context_holder<Context> {
        using context_holder<Context>::context_holder;

        json operator()(const utils::remove_defaults<T> &value) const {
            static constexpr T default_value{};
            json result;
            reflect::for_each<T>([&](auto I) {
                const auto &member_value = reflect::get<I>(value.get());
                if (member_value != reflect::get<I>(default_value)) {
                    if (result.is_null()) {
                        result = json::object();
                    }
                    result.push_back({
                        reflect::member_name<I, T>(),
                        this->template serialize_with_context(member_value)
                    });
                }
            });
            return result;
        }
    };

    template<typename T, typename Context> requires all_fields_deserializable<T, Context>
    struct deserializer<utils::remove_defaults<T>, Context> : context_holder<Context> {
        using context_holder<Context>::context_holder;
        using value_type = utils::remove_defaults<T>;

        value_type operator()(const json &value) const {
            if (value.is_null()) {
                return value_type{};
            } else {
                return value_type{aggregate_deserializer_unchecked<T, Context>{}(value)};
            }
        }
    };
}

#endif