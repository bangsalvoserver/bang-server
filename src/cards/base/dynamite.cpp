#include "dynamite.h"

#include "game/game.h"

namespace banggame {
    void equip_dynamite::on_enable(card *target_card, player *target) {
        target->m_game->add_listener<event_type::on_predraw_check>(target_card, [=](player *e_player, card *e_card) {
            if (e_player == target && e_card == target_card) {
                target->m_game->draw_check_then(target, target_card, [](card_sign sign) {
                    return sign.suit != card_suit::spades
                        || enums::indexof(sign.rank) < enums::indexof(card_rank::rank_2)
                        || enums::indexof(sign.rank) > enums::indexof(card_rank::rank_9);
                }, [=](bool result) {
                    if (!result) {
                        target->m_game->add_log("LOG_CARD_EXPLODES", target_card);
                        target->m_game->play_sound(nullptr, "dynamite");
                        target->discard_card(target_card);
                        target->damage(target_card, nullptr, 3);
                    } else if (auto dest = ranges::find_if(range_other_players(target), [target_card](player *p) {
                        return !p->find_equipped_card(target_card);
                    }); *dest != target) {
                        target_card->on_disable(target);
                        (*dest)->equip_card(target_card);
                    }
                });
            }
        });
    }
}