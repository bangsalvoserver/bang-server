#ifndef __ENUM_MAP_H__
#define __ENUM_MAP_H__

#include "enums.h"

namespace enums {

    template<enumeral E, typename T>
    class enum_map {
    private:
        using array_type = std::array<T, enum_count<E>>;
        array_type m_value{};

        template<bool is_const>
        class enum_map_iterator {
        public:
            using iterator_category = std::random_access_iterator_tag;
            using difference_type = ptrdiff_t;
            using value_type = std::pair<E, std::conditional_t<is_const, const T &, T &>>;
            using reference = value_type;

            using array_pointer = std::conditional_t<is_const, const array_type *, array_type *>;
    
        public:
            constexpr enum_map_iterator() = default;

            constexpr enum_map_iterator(size_t index, array_pointer array)
                : m_index{index}, m_array{array} {}
            
            constexpr bool operator == (const enum_map_iterator &other) const = default;

            constexpr auto operator <=> (const enum_map_iterator &other) const {
                return m_index <=> other.m_index;
            }

            constexpr difference_type operator - (const enum_map_iterator &other) const {
                return m_index - other.m_index;
            }

            constexpr enum_map_iterator &operator ++ () { ++m_index; return *this; }
            constexpr enum_map_iterator operator ++ (int) { auto copy = *this; ++m_index; return copy; }

            constexpr enum_map_iterator &operator --() { --m_index; return *this; }
            constexpr enum_map_iterator operator -- (int) { auto copy = *this; --m_index; return copy; }

            constexpr enum_map_iterator &operator += (difference_type n) { m_index += n; return *this; }
            constexpr enum_map_iterator operator + (difference_type n) const { return { m_index + n, m_array }; }

            friend constexpr enum_map_iterator operator + (difference_type n, const enum_map_iterator &i) { return i + n; }

            constexpr enum_map_iterator &operator -= (difference_type n) { m_index -= n; return *this; }
            constexpr enum_map_iterator operator - (difference_type n) const { return { m_index - n, m_array }; }

            constexpr value_type operator *() const {
                return { enum_values<E>[m_index], (*m_array)[m_index] };
            }

            constexpr value_type operator[] (difference_type n) const {
                return *(*this + n);
            }
        
        private:
            size_t m_index;
            array_pointer m_array;
        };
    
    public:
        using iterator = enum_map_iterator<false>;
        using const_iterator = enum_map_iterator<true>;
        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    
    public:
        constexpr enum_map() = default;

        constexpr enum_map(std::initializer_list<std::pair<E, T>> values) {
            for (const auto &[key, value] : values) {
                m_value[indexof(key)] = value;
            }
        }

        constexpr decltype(auto) operator[] (this auto &&self, E key) {
            return std::forward_like<decltype(self)>(self.m_value[indexof(key)]);
        }

        constexpr iterator begin() { return { 0, &m_value }; }
        constexpr const_iterator cbegin() const { return { 0, &m_value }; }
        constexpr const_iterator begin() const { return cbegin(); }

        constexpr iterator end() { return { m_value.size(), &m_value }; }
        constexpr const_iterator cend() const { return { m_value.size(), &m_value }; }
        constexpr const_iterator end() const { return cend(); }

        constexpr reverse_iterator rbegin() { return reverse_iterator(end()); }
        constexpr const_reverse_iterator crbegin() const { return const_reverse_iterator(cend()); }
        constexpr const_reverse_iterator rbegin() const { return crbegin(); }

        constexpr reverse_iterator rend() { return reverse_iterator(begin()); }
        constexpr const_reverse_iterator crend() const { return const_reverse_iterator(end()); }
        constexpr const_reverse_iterator rend() const { return crend(); }

        constexpr void clear() {
            m_value = {};
        }
    };
}

#endif