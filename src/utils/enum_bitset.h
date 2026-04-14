#ifndef __ENUM_BITSET_H__
#define __ENUM_BITSET_H__

#include "enums.h"
#include "json_serial.h"

#include <cstdint>
#include <type_traits>
#include <initializer_list>
#include <limits>
#include <bit>

namespace enums {
    using bitset_int = uint64_t;

    template<enumeral T>
    class bitset_iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type = ptrdiff_t;
        using value_type = T;
        using pointer = value_type *;
        using reference = value_type;
    
    private:
        static constexpr int end_index = std::numeric_limits<bitset_int>::digits;

        bitset_int m_value = 0;
        int m_index = 0;

        constexpr bitset_iterator(bitset_int value, int index)
            : m_value{value}, m_index{index} {}
    
    public:
        constexpr bitset_iterator() = default;

        static constexpr bitset_iterator begin(bitset_int value) {
            return bitset_iterator{value, value == 0 ? end_index : std::countr_zero(value)};
        }

        static constexpr bitset_iterator end(bitset_int value) {
            return bitset_iterator{value, end_index};
        }
        
        constexpr bool operator == (const bitset_iterator &other) const = default;

        constexpr bitset_iterator &operator ++ () {
            auto shifted = m_value >> (m_index + 1);
            if (shifted == 0) {
                m_index = end_index;
            } else {
                m_index += 1 + std::countr_zero(shifted);
            }
            return *this;
        }

        constexpr bitset_iterator operator ++ (int) {
            auto copy = *this; ++(*this); return copy;
        }

        constexpr value_type operator *() const {
            return enum_values<T>[m_index];
        }
    };

    template<enumeral T>
    class bitset {
    private:
        bitset_int m_value = 0;
    
    public:
        using value_type = T;
        using iterator = bitset_iterator<T>;
        using const_iterator = bitset_iterator<T>;

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

        constexpr bool check_any(bitset value) const {
            return (m_value & value.m_value) != 0;
        }

        constexpr bool check_all(bitset value) const {
            return (m_value & value.m_value) == value.m_value;
        }

        constexpr auto begin() const {
            return iterator::begin(m_value);
        }

        constexpr auto end() const {
            return iterator::end(m_value);
        }
    };

}

namespace json {

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

namespace std {

    template<enums::enumeral E>
    struct formatter<enums::bitset<E>> : formatter<std::string_view> {
        static constexpr std::string bitset_to_string(enums::bitset<E> value) {
            std::string ret;
            for (E v : value) {
                if (!ret.empty()) {
                    ret += ' ';
                }
                ret.append(enums::to_string(v));
            }
            return ret;
        }

        auto format(enums::bitset<E> value, format_context &ctx) const {
            return formatter<std::string_view>::format(bitset_to_string(value), ctx);
        }
    };

}

#endif