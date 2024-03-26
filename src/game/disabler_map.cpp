#include "disabler_map.h"

#include "game_table.h"

#include "cards/card_effect.h"

namespace banggame {

    static auto disableable_cards(game_table *table) {
        struct to_player_card_pair {
            player *p;
            auto operator()(card *c) const { return std::pair{p, c}; }
        };

        return rv::concat(
            table->m_scenario_cards
                | rv::take_last(1)
                | rv::transform(to_player_card_pair{table->m_first_player}),
            table->m_wws_scenario_cards
                | rv::take_last(1)
                | rv::transform(to_player_card_pair{table->m_first_player}),
            table->m_players
                | rv::filter(&player::alive)
                | rv::for_each([](player *p) {
                    return rv::concat(p->m_table, p->m_characters)
                        | rv::transform(to_player_card_pair{p});
                })
        );
    }

    disabler_map::disabler_map_iterator disabler_map::add_disabler(event_card_key key, card_disabler_fun &&fun) {
        for (auto [owner, c] : disableable_cards(m_game)) {
            if (!is_disabled(c) && std::invoke(fun, c)) {
                for (const equip_holder &holder : c->equips) {
                    if (!holder.type->is_nodisable) {
                        holder.type->on_disable(holder.effect_value, c, owner);
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
                if (rn::none_of(range, [&](const auto &pair) { return pair.first == t; })) {
                    a = a || std::invoke(fun, c);
                } else {
                    b = b || std::invoke(fun, c);
                }
            }
            if (!a && b) {
                for (const equip_holder &holder : c->equips) {
                    if (!holder.type->is_nodisable) {
                        holder.type->on_enable(holder.effect_value, c, owner);
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