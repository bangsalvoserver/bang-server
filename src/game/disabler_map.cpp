#include "disabler_map.h"

#include "game_table.h"

#include "cards/card_effect.h"

#include "net/logging.h"
#include "utils/type_name.h"

namespace banggame {
    
    static auto disableable_cards(const player_list &players) {
        return players | rv::for_each([](player_ptr p) {
            return rv::concat(
                p->m_table,
                p->m_characters | rv::take(1)
            );
        });
    }

    void disabler_map::add_disabler(event_card_key key, card_disabler_fun &&fun) {
        logging::debug("add_disabler() on {}: {}", key, fun.target_type());

        for (card_ptr target_card : disableable_cards(get_players())) {
            if (fun(target_card) && !is_disabled(target_card)) {
                for (const equip_holder &holder : target_card->equips | rv::reverse) {
                    if (!holder.is_nodisable()) {
                        holder.on_disable(target_card, target_card->owner);
                    }
                }
            }
        }

        m_disablers.emplace(key, std::move(fun));
    }

    void disabler_map::remove_disabler(event_card_key key) {
        auto iterator = m_disablers.find(key);
        if (iterator == m_disablers.end()) return;
        
        logging::debug("remove_disabler() on {}: {}", iterator->first, iterator->second.target_type());

        for (card_ptr target_card : disableable_cards(get_players())) {
            if (iterator->second(target_card) && rn::none_of(m_disablers, [&](const auto &other_disabler) {
                return &other_disabler != &*iterator && other_disabler.second(target_card);
            })) {
                for (const equip_holder &holder : target_card->equips) {
                    if (!holder.is_nodisable()) {
                        holder.on_enable(target_card, target_card->owner);
                    }
                }
            }
        }

        m_disablers.erase(iterator);
    }

    card_ptr disabler_map::get_disabler(const_card_ptr target_card, bool check_disable_use) const {
        for (auto &[card_key, fun] : m_disablers) {
            if ((!check_disable_use || fun.is_disable_use()) && fun(target_card)) {
                return card_key.target_card;
            }
        }
        return nullptr;
    }

}