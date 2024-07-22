#include "handcuffs.h"

#include "effects/base/draw.h"
#include "cards/filter_enums.h"

#include "game/game.h"

namespace banggame {
    
    struct request_handcuffs : request_base {
        request_handcuffs(card *origin_card, player *target)
            : request_base(origin_card, nullptr, target, {}, -8) {}

        game_string status_text(player *owner) const override {
            if (target == owner) {
                return {"STATUS_HANDCUFFS", origin_card};
            } else {
                return {"STATUS_HANDCUFFS_OTHER", target, origin_card};
            }
        }
    };

    void equip_handcuffs::on_enable(card *target_card, player *target) {
        target->m_game->add_listener<event_type::on_turn_start>(target_card, [=](player *origin) {
            origin->m_game->queue_request<request_handcuffs>(target_card, origin);
        });
        target->m_game->add_listener<event_type::on_turn_end>(target_card, [=](player *origin, bool skipped) {
            origin->m_game->remove_listeners(event_card_key{target_card, 1});
        });
    }

    bool effect_handcuffs::can_play(card *target_card, player *target) {
        return target->m_game->top_request<request_handcuffs>(target) != nullptr;
    }
    
    void effect_handcuffs::on_play(card *target_card, player *target) {
        card *origin_card = target->m_game->top_request<request_handcuffs>()->origin_card;
        target->m_game->pop_request();

        switch (suit) {
        case card_suit::clubs:
            target->m_game->add_log("LOG_DECLARED_CLUBS", target, origin_card);
            break;
        case card_suit::diamonds:
            target->m_game->add_log("LOG_DECLARED_DIAMONDS", target, origin_card);
            break;
        case card_suit::hearts:
            target->m_game->add_log("LOG_DECLARED_HEARTS", target, origin_card);
            break;
        case card_suit::spades:
            target->m_game->add_log("LOG_DECLARED_SPADES", target, origin_card);
            break;
        }

        target->m_game->add_listener<event_type::check_play_card>({origin_card, 1},
            [origin_card, target, suit=suit](player *origin, card *c, const effect_context &ctx, game_string &out_error) {
                if (c->pocket == pocket_type::player_hand && c->owner == target && c->sign.suit != suit) {
                    out_error = {"ERROR_INVALID_SUIT", origin_card, c};
                }
            });
    }
}