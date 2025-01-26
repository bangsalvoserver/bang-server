#ifndef __STATIC_MAP_H__
#define __STATIC_MAP_H__

#include <cassert>
#include <algorithm>
#include <array>

namespace utils {

    template<typename Key, typename Value, typename Comp = std::less<>>
    class static_map_view;

    template<typename Key, typename Value, size_t Size, typename Comp = std::less<>>
    class static_map {
    public:
        using value_type = std::pair<Key, Value>;

    private:
        std::array<value_type, Size> m_data;

    public:
        constexpr static_map(value_type (&&data)[Size]) {
            std::ranges::move(data, m_data.begin());
            std::ranges::sort(m_data, Comp{}, &value_type::first);
            assert(std::ranges::empty(std::ranges::unique(m_data, {}, &value_type::first)) && "Keys must be unique");
        }

        constexpr auto begin() const { return m_data.begin(); }
        constexpr auto end() const { return m_data.end(); }

        constexpr auto find(const auto &key) const {
            if (auto it = std::ranges::lower_bound(m_data, key, Comp{}, &value_type::first); it != end() && it->first == key) {
                return it;
            } else {
                return end();
            }
        }

        friend class static_map_view<Key, Value, Comp>;
    };

    template<typename Key, typename Value, typename Comp>
    class static_map_view {
    public:
        using value_type = const std::pair<Key, Value>;
    
    private:
        std::span<value_type> m_span;
    
    public:
        constexpr static_map_view() = default;
        
        template<size_t Size>
        constexpr static_map_view(const static_map<Key, Value, Size, Comp> &map)
            : m_span{map.m_data} {}

        constexpr auto begin() const { return m_span.begin(); }
        constexpr auto end() const { return m_span.end(); }

        constexpr auto find(const auto &key) const {
            if (auto it = std::ranges::lower_bound(m_span, key, Comp{}, &value_type::first); it != end() && it->first == key) {
                return it;
            } else {
                return end();
            }
        }
    };

    template<typename Key, typename Value, size_t Size, typename Comp = std::less<>>
    constexpr auto make_static_map(std::pair<Key, Value> (&&data)[Size], Comp comp = {}) {
        return static_map<Key, Value, Size, Comp>(std::move(data));
    }

}

#endif