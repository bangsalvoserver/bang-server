#include "handcuffs.h"

#include "effects/base/draw.h"
#include "cards/filter_enums.h"

#include "game/game.h"

namespace banggame {
    
    struct request_handcuffs : request_base {
        request_handcuffs(card_ptr origin_card, player_ptr target)
            : request_base(origin_card, nullptr, target, {}, -8) {}

        game_string status_text(player_ptr owner) const override {
            if (target == owner) {
                return {"STATUS_HANDCUFFS", origin_card};
            } else {
                return {"STATUS_HANDCUFFS_OTHER", target, origin_card};
            }
        }
    };

    void equip_handcuffs::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::on_turn_start>(target_card, [=](player_ptr origin) {
            origin->m_game->queue_request<request_handcuffs>(target_card, origin);
        });
        target->m_game->add_listener<event_type::on_turn_end>(target_card, [=](player_ptr origin, bool skipped) {
            origin->m_game->remove_listeners(event_card_key{target_card, 1});
        });
    }

    bool effect_handcuffs::can_play(card_ptr target_card, player_ptr target) {
        return target->m_game->top_request<request_handcuffs>(target) != nullptr;
    }
    
    void effect_handcuffs::on_play(card_ptr target_card, player_ptr target) {
        card_ptr origin_card = target->m_game->top_request<request_handcuffs>()->origin_card;
        target->m_game->pop_request();

        target->m_game->add_listener<event_type::check_play_card>({origin_card, 1},
            [origin_card, target, suit=suit](player_ptr origin, card_ptr c, const effect_context &ctx, game_string &out_error) {
                if (c->pocket == pocket_type::player_hand && c->owner == target && c->sign.suit != suit) {
                    out_error = {"ERROR_INVALID_SUIT", origin_card, c};
                }
            });
    }
}