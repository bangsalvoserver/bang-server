#ifndef __ENUM_MAP_H__
#define __ENUM_MAP_H__

#include "enums.h"

namespace enums {

    template<enumeral E, typename T>
    class enum_map {
    private:
        using array_type = std::array<T, enum_values<E>().size()>;
        array_type m_value{};

        template<bool is_const>
        class enum_map_iterator {
        public:
            using iterator_category = std::bidirectional_iterator_tag;
            using difference_type = ptrdiff_t;
            using value_type = std::pair<E, std::conditional_t<is_const, const T &, T &>>;
            using pointer = value_type *;
            using reference = value_type;

            using array_pointer = std::conditional_t<is_const, const array_type *, array_type *>;
    
        public:
            enum_map_iterator() = default;

            enum_map_iterator(size_t index, array_pointer array)
                : m_index{index}, m_array{array} {}
            
            bool operator == (const enum_map_iterator &other) const = default;

            enum_map_iterator &operator ++ () {
                ++m_index;
                return *this;
            }

            enum_map_iterator operator ++ (int) {
                auto copy = *this;
                ++m_index;
                return copy;
            }

            enum_map_iterator &operator --() {
                --m_index;
                return *this;
            }

            enum_map_iterator operator -- (int) {
                auto copy = *this;
                --m_index;
                return copy;
            }

            value_type operator *() const {
                return { enum_values<E>()[m_index], (*m_array)[m_index] };
            }
        
        private:
            size_t m_index;
            array_pointer m_array;
        };
        
        using value_type = std::pair<E, T &>;
        using iterator = enum_map_iterator<false>;
        using const_iterator = enum_map_iterator<true>;
        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    
    public:
        decltype(auto) operator[] (this auto &&self, E key) {
            return std::forward_like<decltype(self)>(self.m_value[indexof(key)]);
        }

        iterator begin() { return { 0, &m_value }; }
        const_iterator cbegin() const { return { 0, &m_value }; }
        const_iterator begin() const { return cbegin(); }

        iterator end() { return { m_value.size(), &m_value }; }
        const_iterator cend() const { return { m_value.size(), &m_value }; }
        const_iterator end() const { return cend(); }

        reverse_iterator rbegin() { return reverse_iterator(end()); }
        const_reverse_iterator crbegin() const { return const_reverse_iterator(cend()); }
        const_reverse_iterator rbegin() const { return crbegin(); }

        reverse_iterator rend() { return reverse_iterator(begin()); }
        const_reverse_iterator crend() const { return const_reverse_iterator(end()); }
        const_reverse_iterator rend() const { return crend(); }

        void clear() {
            m_value = {};
        }
    };
}

#endif