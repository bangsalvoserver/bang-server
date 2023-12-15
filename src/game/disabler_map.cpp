#include "disabler_map.h"

#include "game_table.h"

namespace banggame {

    static auto disableable_cards(game_table *table) {
        struct to_player_card_pair {
            player *p;
            auto operator()(card *c) const { return std::pair{p, c}; }
        };

        return ranges::views::concat(
            table->m_scenario_cards
                | ranges::views::take_last(1)
                | ranges::views::transform(to_player_card_pair{table->m_first_player}),
            table->m_wws_scenario_cards
                | ranges::views::take_last(1)
                | ranges::views::transform(to_player_card_pair{table->m_first_player}),
            table->m_players
                | ranges::views::filter(&player::alive)
                | ranges::views::for_each([](player *p) {
                    return ranges::views::concat(p->m_table, p->m_characters)
                        | ranges::views::transform(to_player_card_pair{p});
                })
        );
    }

    disabler_map::disabler_map_iterator disabler_map::add_disabler(event_card_key key, card_disabler_fun &&fun) {
        for (auto [owner, c] : disableable_cards(m_game)) {
            if (!is_disabled(c) && std::invoke(fun, c)) {
                for (const equip_holder &e : c->equips) {
                    if (!e.is_nodisable()) {
                        e.on_disable(c, owner);
                    }
                }
            }
        }

        return m_disablers.emplace(key, std::move(fun));
    }

    void disabler_map::do_remove_disablers(disabler_map_range range) {
        for (auto [owner, c] : disableable_cards(m_game)) {
            bool a = false;
            bool b = false;
            for (const auto &[t, fun] : m_disablers) {
                if (std::ranges::none_of(range, [&](const auto &pair) { return pair.first == t; })) {
                    a = a || std::invoke(fun, c);
                } else {
                    b = b || std::invoke(fun, c);
                }
            }
            if (!a && b) {
                for (const equip_holder &e : c->equips) {
                    if (!e.is_nodisable()) {
                        e.on_enable(c, owner);
                    }
                }
            }
        }

        m_disablers.erase(range.begin(), range.end());
    }

    card *disabler_map::get_disabler(card *target_card) const {
        for (auto &[card_key, fun] : m_disablers) {
            if (std::invoke(fun, target_card)) return card_key.target_card;
        }
        return nullptr;
    }

    bool disabler_map::is_disabled(card *target_card) const {
        return get_disabler(target_card) != nullptr;
    }

}