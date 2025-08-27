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
    struct aggregate_serializer {
        static void write_fields(const T &value, string_writer &writer, const Context &ctx) {
            reflect::for_each<T>([&](auto I) {
                using member_type = reflect::member_type<I, T>;
                if constexpr (!requires { typename serializer<member_type, Context>::skip_field; }) {
                    std::string_view key = reflect::member_name<I, T>();
                    writer.Key(key.data(), key.size());
                    serialize(reflect::get<I>(value), writer, ctx);
                }
            });
        }

        static void write(const T &value, string_writer &writer, const Context &ctx) {
            writer.StartObject();
            write_fields(value, writer, ctx);
            writer.EndObject();
        }
    };

    template<default_aggregate T, typename Context>
    struct serializer<T, Context> : aggregate_serializer<T, Context> {};

    template<transparent_aggregate T, typename Context> requires (reflect::size<T>() == 1)
    struct serializer<T, Context> {
        static void write(const T &value, string_writer &writer, const Context &ctx) {
            serialize(reflect::get<0>(value), writer, ctx);
        }
    };

    template<transparent_aggregate T, typename Context> requires (reflect::size<T>() > 1)
    struct serializer<T, Context> {
        static void write(const T &value, string_writer &writer, const Context &ctx) {
            serialize(reflect::to<std::tuple>(value), writer, ctx);
        }
    };

    template<aggregate T, typename Context>
    struct aggregate_deserializer {
        template<size_t I>
        static reflect::member_type<I, T> deserialize_field(const json &value, const Context &ctx) {
            static constexpr auto name = reflect::member_name<I, T>();
            using value_type = reflect::member_type<I, T>;
            json key(rapidjson::StringRef(name.data(), name.size()));
            if (auto it = value.FindMember(key); it != value.MemberEnd()) {
                return deserialize<value_type>(it->value, ctx);
            } else {
                throw deserialize_error(std::format("Cannot deserialize {}: missing field {}", reflect::type_name<T>(), name));
            }
        }

        static T read(const json &value, const Context &ctx) {
            if (!value.IsObject()) {
                throw deserialize_error(std::format("Cannot deserialize {}: value is not an object", reflect::type_name<T>()));
            }
            return [&]<size_t ... Is>(std::index_sequence<Is ...>) {
                return T{ deserialize_field<Is>(value, ctx) ... };
            }(std::make_index_sequence<reflect::size<T>()>());
        }
    };
    
    template<default_aggregate T, typename Context>
    struct deserializer<T, Context> : aggregate_deserializer<T, Context> {};

    template<remove_defaults_aggregate T, typename Context>
    struct serializer<T, Context>  {
        static void write(const T &value, string_writer &writer, const Context &ctx) {
            bool empty = true;
            reflect::for_each<T>([&](auto I) {
                const auto &member_value = reflect::get<I>(value);
                using member_type = reflect::member_type<I, T>;
                if (member_value != member_type{}) {
                    if (empty) {
                        empty = false;
                        writer.StartObject();
                    }
                    std::string_view key = reflect::member_name<I, T>();
                    writer.Key(key.data(), key.size());
                    serialize(member_value, writer, ctx);
                }
            });
            if (empty) {
                writer.Null();
            } else {
                writer.EndObject();
            }
        }
    };

    template<transparent_aggregate T, typename Context> requires (reflect::size<T>() == 1)
    struct deserializer<T, Context> {
        static T read(const json &value, const Context &ctx) {
            return { deserialize<reflect::member_type<0, T>>(value, ctx) };
        }
    };

    template<transparent_aggregate T, typename Context> requires (reflect::size<T>() > 1)
    struct deserializer<T, Context> {
        static T read(const json &value, const Context &ctx) {
            using tuple_type = std::remove_cvref_t<decltype(reflect::to<std::tuple>(std::declval<T>()))>;
            auto tuple = deserialize<tuple_type>(value, ctx);
            return [&]<size_t ... Is>(std::index_sequence<Is ...>) {
                return T{ std::move(std::get<Is>(tuple)) ... };
            }(std::make_index_sequence<std::tuple_size_v<tuple_type>>());
        }
    };

    template<remove_defaults_aggregate T, typename Context>
    struct deserializer<T, Context> {
        static T read(const json &value, const Context &ctx) {
            if (value.IsNull()) {
                return T{};
            } else {
                return T{aggregate_deserializer<T, Context>::read(value, ctx)};
            }
        }
    };

}

#endif