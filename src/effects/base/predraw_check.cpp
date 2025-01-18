#include "predraw_check.h"

#include "game/game_table.h"
#include "game/game_options.h"

namespace banggame {

    void request_predraw::on_update() {
        if (target->alive() && target->m_game->m_playing == target) {
            if (!live) {
                target->m_game->call_event(event_type::get_predraw_checks{ target, checks });
            }
            auto cards = get_checking_cards();
            if (cards.empty()) {
                target->m_game->pop_request();
            } else if (cards.size() == 1) {
                bool is_auto_pick = target->m_game->m_options.auto_pick_predraw;
                card_ptr checking_card = cards.front().target_card;
                target->m_game->call_event(event_type::check_predraw_auto_pick{ target, checking_card, is_auto_pick });
                if (is_auto_pick) {
                    on_pick(checking_card);
                }
            }
        } else {
            target->m_game->pop_request();
        }
    }

    std::span<const event_card_key> request_predraw::get_checking_cards() const {
        if (checks.empty()) return {};

        auto top_priority = checks.front().priority;
        auto end = rn::find_if_not(checks, [&](const event_card_key &key) {
            return key.priority == top_priority;
        });

        return {checks.begin(), end};
    }

    void request_predraw::remove_check(card_ptr target_card) {
        std::erase_if(checks, [&](const event_card_key &key) {
            return key.target_card == target_card;
        });
    }

    bool request_predraw::can_pick(const_card_ptr target_card) const {
        return rn::contains(get_checking_cards(), target_card, &event_card_key::target_card);
    }

    void request_predraw::on_pick(card_ptr target_card) {
        remove_check(target_card);
        target->m_game->call_event(event_type::on_predraw_check{ target, target_card });
    }

    game_string request_predraw::status_text(player_ptr owner) const {
        auto cards = get_checking_cards();
        if (cards.size() == 1) {
            card_ptr checking_card = cards.front().target_card;
            if (owner == target) {
                return {"STATUS_PREDRAW_FOR", checking_card};
            } else {
                return {"STATUS_PREDRAW_FOR_OTHER", target, checking_card};
            }
        } else {
            if (owner == target) {
                return "STATUS_PREDRAW";
            } else {
                return {"STATUS_PREDRAW_OTHER", target};
            }
        }
    }

    void equip_predraw_check::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::get_predraw_checks>({target_card, 10 + priority},
            [=, priority=priority](player_ptr origin, std::vector<event_card_key> &ret) {
                if (origin == target) {
                    ret.emplace_back(target_card, priority);
                };
            });
    }

    void equip_predraw_check::on_disable(card_ptr target_card, player_ptr target) {
        target->m_game->remove_listeners(event_card_key{target_card, 10 + priority});
    }
}