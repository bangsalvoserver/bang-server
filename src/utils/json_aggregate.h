#ifndef __JSON_AGGREGATE_H__
#define __JSON_AGGREGATE_H__

#include "json_serial.h"

#include <reflect>

namespace json {

    template<typename T>
    concept aggregate = std::is_aggregate_v<T>;

    template<typename T>
    concept transparent_aggregate = requires {
        requires aggregate<T>;
        requires !std::is_empty_v<T>;
        typename T::transparent;
    };

    template<typename T>
    concept remove_defaults_aggregate = requires {
        requires aggregate<T>;
        requires std::is_default_constructible_v<T>;
        typename T::remove_defaults;
    };

    template<typename T>
    concept default_aggregate = aggregate<T> && !transparent_aggregate<T> && !remove_defaults_aggregate<T>;

    template<aggregate T, typename Context>
    struct aggregate_serializer_unchecked {
        json operator()(const T &value, const Context &ctx) const {
            json result = json::object();
            reflect::for_each<T>([&](auto I) {
                using member_type = reflect::member_type<I, T>;
                if constexpr (!requires { typename serializer<member_type, Context>::skip_field; }) {
                    result.push_back({
                        reflect::member_name<I, T>(),
                        serialize_unchecked(reflect::get<I>(value), ctx)
                    });
                }
            });
            return result;
        }
    };

    template<default_aggregate T, typename Context>
    struct serializer<T, Context> : aggregate_serializer_unchecked<T, Context> {};

    template<transparent_aggregate T, typename Context> requires (reflect::size<T>() == 1)
    struct serializer<T, Context> {
        json operator()(const T &value, const Context &ctx) const {
            return serialize_unchecked(reflect::get<0>(value), ctx);
        }
    };

    template<transparent_aggregate T, typename Context> requires (reflect::size<T>() > 1)
    struct serializer<T, Context> {
        json operator()(const T &value, const Context &ctx) const {
            return serialize_unchecked(reflect::to<std::tuple>(value), ctx);
        }
    };

    template<aggregate T, typename Context>
    struct aggregate_deserializer_unchecked {
        template<size_t I>
        reflect::member_type<I, T> deserialize_field(const json &value, const Context &ctx) const {
            static constexpr auto name = reflect::member_name<I, T>();
            using value_type = reflect::member_type<I, T>;
            if (auto it = value.find(name); it != value.end()) {
                return deserialize_unchecked<value_type>(*it, ctx);
            } else {
                throw deserialize_error(std::format("Cannot deserialize {}: missing field {}", reflect::type_name<T>(), name));
            }
        }

        T operator()(const json &value, const Context &ctx) const {
            if (!value.is_object()) {
                throw deserialize_error(std::format("Cannot deserialize {}: value is not an object", reflect::type_name<T>()));
            }
            return [&]<size_t ... Is>(std::index_sequence<Is ...>) {
                return T{ deserialize_field<Is>(value, ctx) ... };
            }(std::make_index_sequence<reflect::size<T>()>());
        }
    };
    
    template<default_aggregate T, typename Context>
    struct deserializer<T, Context> : aggregate_deserializer_unchecked<T, Context> {};

    template<remove_defaults_aggregate T, typename Context>
    struct serializer<T, Context>  {
        json operator()(const T &value, const Context &ctx) const {
            json result;
            reflect::for_each<T>([&](auto I) {
                const auto &member_value = reflect::get<I>(value);
                using member_type = reflect::member_type<I, T>;
                if (member_value != member_type{}) {
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

    template<transparent_aggregate T, typename Context> requires (reflect::size<T>() == 1)
    struct deserializer<T, Context> {
        T operator()(const json &value, const Context &ctx) const {
            return { deserialize_unchecked<reflect::member_type<0, T>>(value, ctx) };
        }
    };

    template<transparent_aggregate T, typename Context> requires (reflect::size<T>() > 1)
    struct deserializer<T, Context> {
        T operator()(const json &value, const Context &ctx) const {
            using tuple_type = std::remove_cvref_t<decltype(reflect::to<std::tuple>(std::declval<T>()))>;
            auto tuple = deserialize_unchecked<tuple_type>(value, ctx);
            return [&]<size_t ... Is>(std::index_sequence<Is ...>) {
                return T{ std::move(std::get<Is>(tuple)) ... };
            }(std::make_index_sequence<std::tuple_size_v<tuple_type>>());
        }
    };

    template<remove_defaults_aggregate T, typename Context>
    struct deserializer<T, Context> {
        T operator()(const json &value, const Context &ctx) const {
            if (value.is_null()) {
                return T{};
            } else {
                return T{aggregate_deserializer_unchecked<T, Context>{*this}(value)};
            }
        }
    };

}

#endif