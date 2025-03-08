#ifndef __SMALL_INT_SET_H__
#define __SMALL_INT_SET_H__

#include <array>
#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <initializer_list>
#include <bit>

#include "json_serial.h"

template<size_t N> struct uint_sized;

template<size_t N> requires (N <= 8) struct uint_sized<N> : std::type_identity<uint8_t> {};
template<size_t N> requires (N > 8 && N <= 16) struct uint_sized<N> : std::type_identity<uint16_t> {};

template<size_t N>
class small_int_set_iterator {
public:
    using iterator_category = std::bidirectional_iterator_tag;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using value_type = int;
    using reference = int;

private:
    using referenced_value = typename uint_sized<N>::type;

    referenced_value value;
    uint8_t index;

public:
    constexpr small_int_set_iterator(referenced_value value, uint8_t index)
        : value{value}, index{index} {}
    
    constexpr int operator *() const {
        return index;
    }

    constexpr small_int_set_iterator &operator++() {
        referenced_value mask = (1 << (index + 1)) - 1;
        index = std::countr_zero<referenced_value>(value & ~mask);
        return *this;
    }

    constexpr small_int_set_iterator operator++(int) {
        auto copy = *this;
        ++*this;
        return copy;
    }

    constexpr small_int_set_iterator &operator--() {
        referenced_value mask = (1 << index) - 1;
        index = 7 - std::countl_zero<referenced_value>(value & mask);
        return *this;
    }

    constexpr small_int_set_iterator operator--(int) {
        auto copy = *this;
        --*this;
        return copy;
    }

    constexpr bool operator == (const small_int_set_iterator &other) const = default;
};

template<size_t N>
class small_int_set {
private:
    using value_type = typename uint_sized<N>::type;

    value_type m_value{};

    constexpr small_int_set(value_type value)
        : m_value{value} {}

public:
    constexpr small_int_set(std::initializer_list<int> values) {
        for (int value : values) {
            set(value);
        }
    }

    constexpr void set(uint8_t value) {
        if (value < 0 || value >= N) {
            throw std::runtime_error(std::format("Values must be in range 0-{}", N - 1));
        }
        m_value |= 1 << value;
    }

    constexpr void clear(uint8_t value) {
        m_value &= ~(1 << value);
    }

    constexpr bool check(uint8_t value) const {
        return bool(m_value & (1 << value));
    }

    constexpr auto begin() const {
        return small_int_set_iterator<N>{m_value, static_cast<uint8_t>(std::countr_zero(m_value))};
    }

    constexpr auto end() const {
        return small_int_set_iterator<N>{m_value, static_cast<uint8_t>(N)};
    }

    constexpr size_t size() const {
        return std::popcount(m_value);
    }

    constexpr int operator[](size_t index) const {
        return *(std::next(begin(), index));
    }
};

#endif