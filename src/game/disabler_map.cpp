#include "disabler_map.h"

#include "game_table.h"

#include "cards/card_effect.h"

#include "net/logging.h"
#include "utils/type_name.h"

namespace banggame {

    static auto disableable_cards(game_table *table) {
        struct to_player_card_pair {
            player_ptr p;
            auto operator()(card_ptr c) const { return std::pair{p, c}; }
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
                | rv::for_each([](player_ptr p) {
                    return rv::concat(p->m_table, p->m_characters)
                        | rv::transform(to_player_card_pair{p});
                })
        );
    }

    void disabler_map::add_disabler(event_card_key key, card_disabler_fun &&fun) {
        for (auto [owner, c] : disableable_cards(m_game)) {
            if (std::invoke(fun, c) && !is_disabled(c)) {
                for (const equip_holder &holder : c->equips | rv::reverse) {
                    if (!holder.is_nodisable()) {
                        holder.on_disable(c, owner);
                    }
                }
            }
        }

        logging::debug("add_disabler() on {}: {}", key, utils::demangle(fun.target_type().name()));
        m_disablers.emplace(key, std::move(fun));
    }

    void disabler_map::do_remove_disablers(disabler_map_range range) {
        for (auto [owner, c] : disableable_cards(m_game)) {
            auto disables_c = [c](const auto &pair) { return std::invoke(pair.second, c); };
            auto outside_range = rv::concat(
                rn::subrange(m_disablers.begin(), range.begin()),
                rn::subrange(range.end(), m_disablers.end())
            );
            
            // enables the card only if there are no disablers left after the removal of `range`
            // AND the card is already disabled by at least one disabler in `range`
            if (rn::any_of(range, disables_c) && rn::none_of(outside_range, disables_c)) {
                for (const equip_holder &holder : c->equips) {
                    if (!holder.is_nodisable()) {
                        holder.on_enable(c, owner);
                    }
                }
            }
        }

        for (const auto &[key, fun] : range) {
            logging::debug("remove_disabler() on {}: {}", key, utils::demangle(fun.target_type().name()));
        }

        m_disablers.erase(range.begin(), range.end());
    }

    card_ptr disabler_map::get_disabler(const_card_ptr target_card, bool check_disable_use) const {
        for (auto &[card_key, fun] : m_disablers) {
            if ((!check_disable_use || fun.is_disable_use()) && std::invoke(fun, target_card)) {
                return card_key.target_card;
            }
        }
        return nullptr;
    }

}