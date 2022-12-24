#include "predraw_check.h"

#include "game/game.h"

namespace banggame {

    bool request_predraw::can_pick(card *target_card) const {
        if (target_card->pocket == pocket_type::player_table && target_card->owner == target) {
            int top_priority = std::ranges::max(target->m_predraw_checks
                | ranges::views::values
                | ranges::views::remove_if(&player::predraw_check::resolved)
                | ranges::views::transform(&player::predraw_check::priority));
            auto it = target->m_predraw_checks.find(target_card);
            if (it != target->m_predraw_checks.end()
                && !it->second.resolved
                && it->second.priority == top_priority) {
                return true;
            }
        }
        return false;
    }

    void request_predraw::on_pick(card *target_card) {
        target->m_game->invoke_action([&]{
            target->m_game->pop_request();
            target->m_game->call_event<event_type::on_predraw_check>(target, target_card);
            target->m_game->queue_action([target = target, target_card] {
                auto it = target->m_predraw_checks.find(target_card);
                if (it != target->m_predraw_checks.end()) {
                    it->second.resolved = true;
                }
                target->next_predraw_check();
            });
        });
    }

    game_string request_predraw::status_text(player *owner) const {
        if (owner == target) {
            return "STATUS_PREDRAW";
        } else {
            return {"STATUS_PREDRAW_OTHER", target};
        }
    }

    void equip_predraw_check::on_enable(card *target_card, player *target) {
        target->m_predraw_checks.try_emplace(target_card, priority, false);
    }

    void equip_predraw_check::on_disable(card *target_card, player *target) {
        target->m_predraw_checks.erase(target_card);
    }
}