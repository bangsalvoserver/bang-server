#ifndef __ENUM_BITSET_H__
#define __ENUM_BITSET_H__

#include "enums.h"
#include "json_serial.h"

#include <cstdint>
#include <type_traits>
#include <initializer_list>

namespace enums {
    using bitset_int = uint64_t;

    template<enumeral T>
    class bitset {
    private:
        bitset_int m_value = 0;
    
    public:
        constexpr bitset() = default;

        constexpr bitset(T value) {
            add(value);
        }

        constexpr bitset(std::initializer_list<T> values) {
            for (T value : values) {
                add(value);
            }
        }

    public:
        static constexpr bitset_int to_bit(T value) {
            return static_cast<bitset_int>(1) << indexof(value);
        }

        constexpr void merge(bitset value) {
            m_value |= value.m_value;
        }

        constexpr void add(T value) {
            m_value |= to_bit(value);
        }

        constexpr void remove(T value) {
            m_value &= ~to_bit(value);
        }

        constexpr void clear() {
            m_value = 0;
        }

        constexpr bool empty() const {
            return m_value == 0;
        }

        constexpr bool check(T value) const {
            return (m_value & to_bit(value)) != 0;
        }

        constexpr bool check(bitset value) const {
            return (m_value & value.m_value) == value.m_value;
        }
    };

}

namespace json {

    template<enums::enumeral T, typename Context>
    struct serializer<enums::bitset<T>, Context> {
        static void write(const enums::bitset<T> &value, string_writer &writer) {
            writer.StartArray();
            for (T v : enums::enum_values<T>()) {
                if (value.check(v)) {
                    serialize(enums::to_string(v), writer);
                }
            }
            writer.EndArray();
        }
    };

    template<enums::enumeral T, typename Context>
    struct deserializer<enums::bitset<T>, Context> {
        static enums::bitset<T> read(const json &value, const Context &ctx) {
            if (!value.IsArray()) {
                throw deserialize_error(std::format("Cannot deserialize {} bitset: value is not an array", reflect::type_name<T>()));
            }
            enums::bitset<T> ret;
            for (const auto &elem : value.GetArray()) {
                ret.add(deserialize<T>(elem, ctx));
            }
            return ret;
        }
    };

}

#endif