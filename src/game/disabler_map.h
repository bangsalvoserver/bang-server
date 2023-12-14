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
    public:
        using disabler_map_t = std::multimap<event_card_key, std::unique_ptr<card_disabler>, std::less<>>;
        using disabler_map_iterator = disabler_map_t::const_iterator;
        using disabler_map_range = std::ranges::subrange<disabler_map_iterator>;

    private:
        disabler_map_t m_disablers;
        game_table *m_game;

        disabler_map_iterator do_add_disabler(event_card_key key, std::unique_ptr<card_disabler> &&fun);
        void do_remove_disablers(disabler_map_range range);

    public:
        disabler_map(game_table *game): m_game(game) {}

        template<card_predicate Function>
        disabler_map_iterator add_disabler(event_card_key key, Function &&fun) {
            return do_add_disabler(key, std::make_unique<card_disabler_impl<std::decay_t<Function>>>(std::forward<Function>(fun)));
        }

        void remove_disabler(disabler_map_iterator it) {
            do_remove_disablers(disabler_map_range{it, std::next(it)});
        }

        void remove_disablers(event_card_key key) {
            auto [low, high] = m_disablers.equal_range(key);
            do_remove_disablers({low, high});
        }

        void remove_disablers(card *key) {
            auto [low, high] = m_disablers.equal_range(key);
            do_remove_disablers({low, high});
        }

        card *get_disabler(card *target_card) const;
        bool is_disabled(card *target_card) const;
    };
}

#endif