#ifndef __DISABLER_MAP_H__
#define __DISABLER_MAP_H__

#include <functional>
#include <typeindex>
#include <map>

#include "event_card_key.h"

namespace banggame {

    struct game_table;

    class card_disabler_fun {
    private:
        std::move_only_function<bool(const_card_ptr) const> m_fun;
        std::type_index m_type;
        bool m_disable_use;
    
    public:
        template<typename Function>
        card_disabler_fun(Function &&fun, bool disable_use = false)
            : m_fun{std::forward<Function>(fun)}
            , m_type{typeid(Function)}
            , m_disable_use(disable_use) {}

        bool operator()(const_card_ptr target_card) const {
            return m_fun(target_card);
        }
        
        const std::type_index &target_type() const {
            return m_type;
        }

        bool is_disable_use() const {
            return m_disable_use;
        }
    };

    class disabler_map {
    private:
        using disabler_map_t = std::multimap<event_card_key, card_disabler_fun, std::less<>>;
        using disabler_map_iterator = disabler_map_t::const_iterator;
        using disabler_map_range = rn::subrange<disabler_map_iterator>;

    private:
        disabler_map_t m_disablers;
        game_ptr m_game;

        void do_remove_disablers(disabler_map_range range);

    public:
        disabler_map(game_ptr m_game): m_game(m_game) {}

        void add_disabler(event_card_key key, card_disabler_fun &&fun);

        void remove_disablers(event_card_key key) {
            auto [low, high] = m_disablers.equal_range(key);
            do_remove_disablers({low, high});
        }

        void remove_disablers(card_ptr key) {
            auto [low, high] = m_disablers.equal_range(key);
            do_remove_disablers({low, high});
        }

        card_ptr get_disabler(const_card_ptr target_card, bool check_disable_use = false) const;

        bool is_disabled(const_card_ptr target_card, bool check_disable_use = false) const {
            return get_disabler(target_card, check_disable_use) != nullptr;
        }

        card_ptr get_usage_disabler(const_card_ptr target_card) const {
            return get_disabler(target_card, true);
        }

        bool is_usage_disabled(const_card_ptr target_card) const {
            return is_disabled(target_card, true);
        }
    };
}

#endif