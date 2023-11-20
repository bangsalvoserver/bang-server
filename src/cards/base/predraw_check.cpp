#include "predraw_check.h"

#include "game/game.h"

namespace banggame {

    void request_predraw::on_update() {
        if (target->alive() && target->m_game->m_playing == target) {
            if (!live) {
                checks = target->m_game->call_event<event_type::get_predraw_checks>(target, std::vector<card_priority_pair>{});
                std::ranges::sort(checks, std::greater{}, &card_priority_pair::second);
            }
            if (checks.empty()) {
                target->m_game->pop_request();
            } else {
                auto_pick();
            }
        } else {
            target->m_game->pop_request();
        }
    }

    bool request_predraw::can_pick(card *target_card) const {
        if (target_card->owner == target) {
            int top_priority = checks[0].second;
            return ranges::contains(checks | ranges::views::take_while([=](const card_priority_pair &pair) {
                return pair.second == top_priority;
            }), target_card, &card_priority_pair::first);
        }
        return false;
    }

    void request_predraw::on_pick(card *target_card) {
        std::erase_if(checks, [&](const card_priority_pair &key) {
            return key.first == target_card;
        });
        target->m_game->call_event<event_type::on_predraw_check>(target, target_card);
    }

    game_string request_predraw::status_text(player *owner) const {
        if (owner == target) {
            return "STATUS_PREDRAW";
        } else {
            return {"STATUS_PREDRAW_OTHER", target};
        }
    }

    void equip_predraw_check::on_enable(card *target_card, player *target) {
        target->m_game->add_listener<event_type::get_predraw_checks>({target_card, 10},
            [=, priority=priority](player *origin, std::vector<card_priority_pair> &ret) {
                if (origin == target) {
                    ret.emplace_back(target_card, priority);
                };
            });
    }

    void equip_predraw_check::on_disable(card *target_card, player *target) {
        target->m_game->remove_listeners(event_card_key{target_card, 10});
    }
}