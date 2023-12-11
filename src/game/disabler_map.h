#ifndef __DISABLER_MAP_H__
#define __DISABLER_MAP_H__

#include <functional>
#include <map>

#include "event_card_key.h"

namespace banggame {

    struct game_table;

    template<typename T>
    concept card_predicate = std::predicate<T, card *>;

    struct card_disabler {
        virtual ~card_disabler() = default;
        virtual bool operator()(card *target_card) const = 0;
    };

    template<card_predicate Function>
    class card_disabler_impl : public card_disabler, private Function {
    public:
        template<std::convertible_to<Function> U>
        card_disabler_impl(U &&function) : Function(std::forward<U>(function)) {}

        bool operator()(card *target_card) const override {
            return std::invoke(static_cast<const Function &>(*this), target_card);
        }
    };

    class disabler_map {
    private:
        std::multimap<event_card_key, std::unique_ptr<card_disabler>, std::less<>> m_disablers;
        game_table *m_game;

        void do_add_disabler(event_card_key key, std::unique_ptr<card_disabler> &&fun);

    public:
        disabler_map(game_table *game): m_game(game) {}

        template<card_predicate Function>
        void add_disabler(event_card_key key, Function &&fun) {
            do_add_disabler(key, std::make_unique<card_disabler_impl<std::decay_t<Function>>>(std::forward<Function>(fun)));
        }

        void remove_disablers(event_card_key key);

        card *get_disabler(card *target_card) const;
        bool is_disabled(card *target_card) const;
    };
}

#endif