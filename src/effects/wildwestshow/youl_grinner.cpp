#include "youl_grinner.h"

#include "effects/base/gift_card.h"

#include "cards/game_events.h"

#include "game/game_table.h"
#include "game/prompts.h"

namespace banggame {

    bool request_youl_grinner::can_pick(const_card_ptr target_card) const {
        return target_card->pocket == pocket_type::player_hand && target_card->owner == target;
    }

    prompt_string request_youl_grinner::pick_prompt(card_ptr target_card) const {
        return prompts::bot_check_discard_card(target, target_card);
    }

    void request_youl_grinner::on_pick(card_ptr target_card) {
        target->m_game->pop_request();
        effect_gift_card{true}.on_play(origin_card, target, target_card, origin);
    }

    game_string request_youl_grinner::status_text(player_ptr owner) const {
        if (target == owner) {
            return {"STATUS_YOUL_GRINNER", origin_card, origin};
        } else {
            return {"STATUS_YOUL_GRINNER_OTHER", origin_card, origin, target};
        }
    }

    void equip_youl_grinner::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::on_turn_start>({target_card, -2}, [=](player_ptr origin) {
            if (target == origin) {
                for (player_ptr p : target->m_game->range_other_players(target)) {
                    if (p->m_hand.size() > target->m_hand.size()) {
                        target->m_game->queue_request<request_youl_grinner>(target_card, target, p);
                    }
                }
            }
        });
    }
}