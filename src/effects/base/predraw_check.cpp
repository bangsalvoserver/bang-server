#include "predraw_check.h"

#include "game/game.h"
#include "game/game_options.h"

namespace banggame {

    void request_predraw::on_update() {
        if (target->alive() && target->m_game->m_playing == target) {
            if (!live) {
                target->m_game->call_event(event_type::get_predraw_checks{ target, checks });
                rn::sort(checks, std::greater{}, &event_card_key::priority);
            }
            if (checks.empty()) {
                target->m_game->pop_request();
            } else if (target->m_game->m_options.auto_pick_predraw) {
                auto_pick();
            }
        } else {
            target->m_game->pop_request();
        }
    }

    bool request_predraw::can_pick(const_card_ptr target_card) const {
        if (target_card->owner == target) {
            int top_priority = checks[0].priority;
            return rn::contains(checks | rv::take_while([=](const event_card_key &key) {
                return key.priority == top_priority;
            }), target_card, &event_card_key::target_card);
        }
        return false;
    }

    void request_predraw::on_pick(card_ptr target_card) {
        std::erase_if(checks, [&](const event_card_key &key) {
            return key.target_card == target_card;
        });
        target->m_game->call_event(event_type::on_predraw_check{ target, target_card });
    }

    game_string request_predraw::status_text(player_ptr owner) const {
        if (owner == target) {
            return "STATUS_PREDRAW";
        } else {
            return {"STATUS_PREDRAW_OTHER", target};
        }
    }

    void equip_predraw_check::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::get_predraw_checks>({target_card, 10},
            [=, priority=priority](player_ptr origin, std::vector<event_card_key> &ret) {
                if (origin == target) {
                    ret.emplace_back(target_card, priority);
                };
            });
    }

    void equip_predraw_check::on_disable(card_ptr target_card, player_ptr target) {
        target->m_game->remove_listeners(event_card_key{target_card, 10});
    }
}