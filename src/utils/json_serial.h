#ifndef __JSON_SERIAL_H__
#define __JSON_SERIAL_H__

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/error/en.h>

#include <vector>
#include <chrono>
#include <stdexcept>

#include "range_utils.h"

namespace json {

    using json = rapidjson::Value;
    using json_document = rapidjson::Document;

    template<typename Encoding = rapidjson::UTF8<>>
    class string_adapter {
    public:
        using Ch = typename Encoding::Ch;
        using string_type = std::basic_string<Ch>;

        string_adapter(string_type &str) : m_str{str} {}

        void Put(Ch c) { PutUnsafe(c); }
        void PutUnsafe(Ch c) { m_str.push_back(c); }
        void Flush() {}
    
    private:
        string_type &m_str;
    };

    using string_writer = rapidjson::Writer<string_adapter<>>;

    template<typename T>
    concept is_complete = requires(T self) { sizeof(self); };

    template<typename T, typename Context> struct serializer;

    template<typename T, typename Context> struct deserializer;

    class raw_string {
    private:
        std::string m_value;

    public:
        raw_string(std::string value): m_value(std::move(value)) {}

        operator std::string_view() const {
            return m_value;
        }
    };

    struct serialize_error : std::runtime_error {
        using std::runtime_error::runtime_error;
    };

    struct deserialize_error : std::runtime_error {
        using std::runtime_error::runtime_error;
    };

    struct no_context {};

    template<typename T, typename Context = no_context>
    void serialize(const T &value, string_writer &writer, const Context &context = {}) {
        using serializer_type = serializer<T, Context>;
        static_assert(is_complete<serializer_type>, "No serializer specified for type T");
        if constexpr (requires { serializer_type::write(value, writer, context); }) {
            serializer_type::write(value, writer, context);
        } else {
            serializer_type::write(value, writer);
        }
    }

    template<typename T, typename Context = no_context>
    std::string to_string(const T &value, const Context &context = {}) {
        std::string result;
        result.reserve(256);
        string_adapter adapter(result);
        string_writer writer(adapter);
        serialize(value, writer, context);
        return result;
    }

    template<typename T, typename Context = no_context>
    auto deserialize(const json &value, const Context &context = {}) {
        using deserializer_type = deserializer<T, Context>;
        static_assert(is_complete<deserializer_type>, "No deserializer specified for type T");
        if constexpr (requires { deserializer_type::read(value, context); }) {
            return deserializer_type::read(value, context);
        } else {
            return deserializer_type::read(value);
        }
    }

    template<typename T, typename Context = no_context>
    T parse_string(std::string_view str, const Context &context = {}) {
        json_document doc;
        doc.Parse(str.data(), str.size());
        if (doc.HasParseError()) {
            throw deserialize_error(std::format("JSON parse error: {}", rapidjson::GetParseError_En(doc.GetParseError())));
        }
        return deserialize<T, Context>(doc, context);
    }
    
    template<typename Context>
    struct serializer<raw_string, Context> {
        static void write(std::string_view value, string_writer &writer) {
            writer.RawValue(value.data(), value.size(), rapidjson::Type::kObjectType);
        }
    };

    template<typename Context>
    struct serializer<std::nullptr_t, Context> {
        static void write(std::nullptr_t, string_writer &writer) {
            writer.Null();
        }
    };

    template<typename Context>
    struct serializer<bool, Context> {
        static void write(bool value, string_writer &writer) {
            writer.Bool(value);
        }
    };

    template<std::integral T, typename Context> requires std::is_signed_v<T>
    struct serializer<T, Context> {
        static void write(T value, string_writer &writer) {
            writer.Int64(static_cast<int64_t>(value));
        }
    };

    template<std::integral T, typename Context> requires std::is_unsigned_v<T>
    struct serializer<T, Context> {
        static void write(T value, string_writer &writer) {
            writer.Uint64(static_cast<uint64_t>(value));
        }
    };

    template<std::floating_point T, typename Context>
    struct serializer<T, Context> {
        static void write(T value, string_writer &writer) {
            writer.Double(static_cast<double>(value));
        }
    };

    template<std::convertible_to<std::string_view> T, typename Context>
    struct serializer<T, Context> {
        static void write(std::string_view value, string_writer &writer) {
            if (!value.empty()) {
                writer.String(value.data(), value.size());
            } else {
                writer.String("", 0);
            }
        }
    };

    template<typename Context>
    struct serializer<const char *, Context> {
        static void write(const char *value, string_writer &writer) {
            if (value) {
                writer.String(value);
            } else {
                writer.String("", 0);
            }
        }
    };

    template<rn::range Range, typename Context> requires (!std::convertible_to<Range, std::string_view>)
    struct serializer<Range, Context> {
        static void write(const Range &value, string_writer &writer, const Context &ctx) {
            writer.StartArray();
            for (const auto &obj : value) {
                serialize(obj, writer, ctx);
            }
            writer.EndArray();
        }
    };

    template<typename Rep, typename Period, typename Context>
    struct serializer<std::chrono::duration<Rep, Period>, Context> {
        static void write(const std::chrono::duration<Rep, Period> &value, string_writer &writer, const Context &ctx) {
            serialize(value.count(), writer, ctx);
        }
    };

    template<typename Clock, typename Duration, typename Context>
    struct serializer<std::chrono::time_point<Clock, Duration>, Context> {
        static void write(const std::chrono::time_point<Clock, Duration> &value, string_writer &writer, const Context &ctx) {
            serialize(value.time_since_epoch(), writer, ctx);
        }
    };

    template<typename T, typename Context>
    struct serializer<std::optional<T>, Context> {
        static void write(const std::optional<T> &value, string_writer &writer, const Context &ctx) {
            if (value) serialize(*value, writer, ctx);
            else writer.Null();
        }
    };

    template<typename First, typename Second, typename Context>
    struct serializer<std::pair<First, Second>, Context> {
        static void write(const std::pair<First, Second> &value, string_writer &writer, const Context &ctx) {
            writer.StartArray();
            serialize(value.first, writer, ctx);
            serialize(value.second, writer, ctx);
            writer.EndArray();
        }
    };

    template<typename Context, typename ... Ts>
    struct serializer<std::tuple<Ts ...>, Context> {
        static void write(const std::tuple<Ts ...> &value, string_writer &writer, const Context &ctx) {
            writer.StartArray();
            [&]<size_t ... Is>(std::index_sequence<Is ...>) {
                (serialize(std::get<Is>(value), writer, ctx), ...);
            }(std::index_sequence_for<Ts ...>());
            writer.EndArray();
        }
    };

    template<typename Context>
    struct deserializer<json_document, Context> {
        static json_document read(const json &value) {
            json_document doc;
            doc.CopyFrom(value, doc.GetAllocator());
            return doc;
        }
    };

    template<typename Context>
    struct deserializer<std::nullptr_t, Context> {
        static std::nullptr_t read(const json &value) {
            if (!value.IsNull()) {
                throw deserialize_error("Cannot deserialize null");
            }
            return nullptr;
        }
    };

    template<typename Context>
    struct deserializer<bool, Context> {
        static bool read(const json &value) {
            if (!value.IsBool()) {
                throw deserialize_error("Cannot deserialize boolean");
            }
            return value.GetBool();
        }
    };

    template<std::integral T, typename Context> requires std::is_signed_v<T>
    struct deserializer<T, Context> {
        static T read(const json &value) {
            if (value.IsInt()) {
                return static_cast<T>(value.GetInt());
            } else if (value.IsInt64()) {
                return static_cast<T>(value.GetInt64());
            }
            throw deserialize_error("Cannot deserialize integer");
        }
    };

    template<std::integral T, typename Context> requires std::is_unsigned_v<T>
    struct deserializer<T, Context> {
        static T read(const json &value) {
            if (value.IsUint()) {
                return static_cast<T>(value.GetUint());
            } else if (value.IsUint64()) {
                return static_cast<T>(value.GetUint64());
            }
            throw deserialize_error("Cannot deserialize unsigned integer");
        }
    };

    template<std::floating_point T, typename Context>
    struct deserializer<T, Context> {
        static T read(const json &value) {
            if (value.IsNumber()) {
                return static_cast<T>(value.GetDouble());
            }
            throw deserialize_error("Cannot deserialize number");
        }
    };

    template<typename Context>
    struct deserializer<std::string, Context> {
        static std::string read(const json &value) {
            if (!value.IsString()) {
                throw deserialize_error("Cannot deserialize string");
            }
            return std::string{value.GetString(), value.GetStringLength()};
        }
    };
    
    template<rn::range Range, typename Context>
    struct deserializer<Range, Context> {
        static Range read(const json &value, const Context &ctx) {
            if (!value.IsArray()) {
                throw deserialize_error("Cannot deserialize range");
            }
            return value.GetArray()
                | rv::transform([&](const json &obj) {
                    return deserialize<rn::range_value_t<Range>>(obj, ctx);
                })
                | rn::to<Range>();
        }
    };

    template<typename T, typename Context, size_t N>
    struct deserializer<std::array<T, N>, Context> {
        static std::array<T, N> read(const json &value, const Context &context) {
            if (!value.IsArray()) {
                throw deserialize_error("Cannot deserialize array");
            }
            const auto &value_array = value.GetArray();
            if (value_array.Size() != N) {
                throw deserialize_error("Cannot deserialize array: invalid size");
            }
            return [&]<size_t ... Is>(std::index_sequence<Is ...>) {
                return std::array<T, N>{
                    deserialize<T, Context>(value_array[Is], context) ...
                };
            }(std::make_index_sequence<N>());
        }
    };

    template<typename Rep, typename Period, typename Context>
    struct deserializer<std::chrono::duration<Rep, Period>, Context> {
        static std::chrono::duration<Rep, Period> read(const json &value, const Context &ctx) {
            return std::chrono::duration<Rep, Period>{deserialize<Rep, Context>(value, ctx)};
        }
    };
    
    template<typename Clock, typename Duration, typename Context>
    struct deserializer<std::chrono::time_point<Clock, Duration>, Context> {
        static std::chrono::time_point<Clock, Duration> read(const json &value, const Context &ctx) {
            return std::chrono::time_point<Clock, Duration>{ deserialize<Duration>(value, ctx) };
        }
    };

    template<typename T, typename Context>
    struct deserializer<std::optional<T>, Context> {
        static std::optional<T> read(const json &value, const Context &ctx) {
            if (value.IsNull()) {
                return std::nullopt;
            } else {
                return deserialize<T>(value, ctx);
            }
        }
    };

    template<typename First, typename Second, typename Context>
    struct deserializer<std::pair<First, Second>, Context> {
        static std::pair<First, Second> read(const json &value, const Context &ctx) {
            if (!value.IsArray()) {
                throw deserialize_error("Cannot deserialize pair: value is not an array");
            }
            const auto &value_array = value.GetArray();
            if (value_array.Size() != 2) {
                throw deserialize_error("Cannot deserialize pair: size is not 2");
            }
            return {
                deserialize<First>(value_array[0], ctx),
                deserialize<Second>(value_array[1], ctx)
            };
        }
    };

    template<typename Context, typename ...Ts>
    struct deserializer<std::tuple<Ts ...>, Context> {
        static std::tuple<Ts ...> read(const json &value, const Context &ctx) {
            if (!value.IsArray()) {
                throw deserialize_error("Cannot deserialize tuple: value is not an array");
            }
            const auto &value_array = value.GetArray();
            if (value_array.Size() != sizeof...(Ts)) {
                throw deserialize_error("Cannot deserialize tuple: invalid size of array");
            }
            return [&]<size_t ... Is>(std::index_sequence<Is ...>) {
                return std::make_tuple(deserialize<Ts>(value_array[Is], ctx) ...);
            }(std::index_sequence_for<Ts ...>());
        }
    };
}

#endif