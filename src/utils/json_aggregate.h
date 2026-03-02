#ifndef __JSON_AGGREGATE_H__
#define __JSON_AGGREGATE_H__

#include "json_serial.h"

#include <meta>

namespace json {

    template<typename T>
    concept aggregate = std::is_aggregate_v<T>;

    template<aggregate T>
    static constexpr auto fields_of = std::define_static_array(
        std::meta::nonstatic_data_members_of(^^T, std::meta::access_context::current())
        | rv::filter([](std::meta::info field) { return !has_annotation(field, ^^ignore_t); })
    );

    template<aggregate T, typename Context>
    struct aggregate_serializer {
        static constexpr auto fields = fields_of<T>;

        static void write_fields(const T &value, string_writer &writer, const Context &ctx) {
            template for (constexpr auto field : fields) {
                std::string_view key = std::meta::identifier_of(field);
                writer.Key(key.data(), key.size());
                serialize(value.[:field:], writer, ctx);
            }
        }

        static void write(const T &value, string_writer &writer, const Context &ctx) {
            if constexpr (requires { typename T::transparent; }) {
                static constexpr bool one_field = fields.size() == 1;
                if constexpr (!one_field) {
                    writer.StartArray();
                }
                template for (constexpr auto field : fields) {
                    serialize(value.[:field:], writer, ctx);
                }
                if constexpr (!one_field) {
                    writer.EndArray();
                }
            } else {
                writer.StartObject();
                write_fields(value, writer, ctx);
                writer.EndObject();
            }
        }
    };

    template<aggregate T, typename Context>
    struct serializer<T, Context> : aggregate_serializer<T, Context> {};

    template<aggregate T, typename Context>
    struct aggregate_deserializer {
        static constexpr auto fields = fields_of<T>;

        template<std::meta::info field>
        static void deserialize_field(T &result, const json &value, const Context &ctx) {
            static constexpr auto name = std::meta::identifier_of(field);
            auto &field_value = result.[:field:];
            using field_type = std::remove_reference_t<decltype(field_value)>;
            json key(rapidjson::StringRef(name.data(), name.size()));
            if (auto it = value.FindMember(key); it != value.MemberEnd()) {
                field_value = deserialize<field_type>(it->value, ctx);
            } else {
                throw deserialize_error(std::format("Cannot deserialize {}: missing field {}", type_name<T>, name));
            }
        }

        template<std::meta::info field>
        static void deserialize_transparent_field(T &result, const json &value, const Context &ctx) {
            auto &field_value = result.[:field:];
            using field_type = std::remove_reference_t<decltype(field_value)>;
            field_value = deserialize<field_type>(value, ctx);
        }

        static T read(const json &value, const Context &ctx) {
            if constexpr (has_annotation(^^T, ^^transparent_t)) {
                if constexpr (fields.size() == 1) {
                    T result{};
                    auto &field_value = result.[:fields[0]:];
                    using field_type = std::remove_reference_t<decltype(field_value)>;
                    field_value = deserialize<field_type>(value, ctx);
                    return result;
                } else {
                    if (!value.IsArray()) {
                        throw deserialize_error(std::format("Cannot deserialize {}: value is not an array", type_name<T>));
                    }
                    const auto &value_array = value.GetArray();
                    if (value_array.Size() != fields.size()) {
                        throw deserialize_error(std::format("Cannot deserialize {}: invalid size", type_name<T>));
                    }
                    T result{};
                    size_t i = 0;
                    template for (constexpr auto field : fields) {
                        deserialize_transparent_field<field>(result, value_array[i], ctx);
                        ++i;
                    }
                    return result;
                }
            } else {
                if (!value.IsObject()) {
                    throw deserialize_error(std::format("Cannot deserialize {}: value is not an object", type_name<T>));
                }
                T result{};
                template for (constexpr auto field : fields) {
                    deserialize_field<field>(result, value, ctx);
                }
                return result;
            }
        }
    };
    
    template<aggregate T, typename Context>
    struct deserializer<T, Context> : aggregate_deserializer<T, Context> {};

}

#endif