#ifndef __STATIC_MAP_H__
#define __STATIC_MAP_H__

#include <cassert>
#include <algorithm>
#include <array>

namespace utils {

    static constexpr size_t linear_search_cap = 16;

    template<typename Key, typename Value, typename Comp = std::less<>>
    class static_map_view;

    template<typename Comp>
    class static_map_base {
    protected:
        [[no_unique_address]] Comp m_comp;

    public:
        constexpr static_map_base(Comp comp = {}) : m_comp(std::move(comp)) {}

        constexpr auto find(this const auto &self, const auto &key) {
            auto begin = self.begin();
            auto end = self.end();
            using value_type = std::iter_value_t<decltype(begin)>;
            if (self.size() <= linear_search_cap) {
                return std::ranges::find(begin, end, key, &value_type::first);
            } else {
                auto it = std::ranges::lower_bound(begin, end, key, self.m_comp, &value_type::first);
                if (it != end && it->first == key) {
                    return it;
                }
                return end;
            }
        }
    };

    template<typename Key, typename Value, size_t Size, typename Comp = std::less<>>
    class static_map : public static_map_base<Comp> {
    public:
        using value_type = std::pair<Key, Value>;

    private:
        std::array<value_type, Size> m_data;

    public:
        constexpr static_map(value_type (&&data)[Size], Comp &&comp)
            : static_map_base<Comp>{std::move(comp)}
        {
            std::ranges::move(data, m_data.begin());
            std::ranges::sort(m_data, this->m_comp, &value_type::first);
            assert(std::ranges::empty(std::ranges::unique(m_data, {}, &value_type::first)) && "Keys must be unique");
        }

        constexpr auto begin() const { return m_data.begin(); }
        constexpr auto end() const { return m_data.end(); }
        constexpr auto size() const { return Size; }

        friend class static_map_view<Key, Value, Comp>;
    };

    template<typename Key, typename Value, typename Comp>
    class static_map_view : public static_map_base<Comp> {
    public:
        using value_type = const std::pair<Key, Value>;
    
    private:
        std::span<value_type> m_span;
    
    public:
        constexpr static_map_view(Comp comp = {})
            : static_map_base<Comp>(std::move(comp)) {}
        
        template<size_t Size>
        constexpr static_map_view(const static_map<Key, Value, Size, Comp> &map)
            : static_map_base<Comp>{map.m_comp}
            , m_span{map.m_data} {}

        constexpr auto begin() const { return m_span.begin(); }
        constexpr auto end() const { return m_span.end(); }
        constexpr auto size() const { return m_span.size(); }
    };

    template<typename Key, typename Value, size_t Size, typename Comp = std::less<>>
    constexpr auto make_static_map(std::pair<Key, Value> (&&data)[Size], Comp comp = {}) {
        return static_map<Key, Value, Size, Comp>(std::move(data), std::move(comp));
    }

}

#endif