#include "tarantula_juice.h"

#include "effects/base/draw_check.h"

#include "game/game_table.h"

namespace banggame {
    
    void effect_tarantula_juice::on_play(card_ptr origin_card, player_ptr origin) {
        origin->m_game->queue_request<request_check>(origin, origin_card, std::not_fn(&card_sign::is_spades),
            [=](card_sign sign) {
                switch (sign.suit) {
                case card_suit::hearts:
                case card_suit::diamonds:
                    origin->heal(2);
                    break;
                case card_suit::clubs:
                    origin->draw_card(1, origin_card);
                    break;
                case card_suit::spades:
                    origin->damage(origin_card, nullptr, 1);
                    break;
                default:
                    // ignore
                    break;
                }
            }
        );
    }

}