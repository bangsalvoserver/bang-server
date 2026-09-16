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
    
    public:
        template<typename Function>
        card_disabler_fun(Function &&fun)
            : m_fun{std::forward<Function>(fun)}
            , m_type{typeid(Function)} {}

        bool operator()(const_card_ptr target_card) const {
            return m_fun(target_card);
        }
        
        const std::type_index &target_type() const {
            return m_type;
        }
    };

    class disabler_map {
    private:
        std::multimap<event_card_key, card_disabler_fun> m_disablers;
    
    protected:
        virtual const player_list &get_players() const = 0;

    public:
        void add_disabler(event_card_key key, card_disabler_fun &&fun);

        void remove_disabler(event_card_key key);

        card_ptr get_disabler(const_card_ptr target_card) const;

        bool is_disabled(const_card_ptr target_card) const {
            return get_disabler(target_card) != nullptr;
        }
    };
}

#endif