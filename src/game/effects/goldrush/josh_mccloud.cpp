#include "josh_mccloud.h"

#include "../../game.h"
#include "../base/requests.h"

namespace banggame {

    void effect_josh_mccloud::on_play(card *origin_card, player *target) {
        auto *card = target->m_game->draw_shop_card();
        auto discard_drawn_card = [&]{
            target->m_game->move_card(card, pocket_type::shop_discard, nullptr, show_card_flags::pause_before_move);
        };
        if (card->color == card_color_type::black) {
            if (!target->m_game->check_flags(game_flags::disable_equipping)) {
                auto equip_set = target->make_equip_set(card);
                if (equip_set.empty()) {
                    discard_drawn_card();
                } else if (equip_set.size() == 1) {
                    equip_set.front()->equip_card(card);
                } else {
                    target->m_game->queue_request<request_force_play_card>(origin_card, target, card);
                }
            } else {
                discard_drawn_card();
            }
        } else if (card->has_tag(tag_type::shopchoice) || target->is_possible_to_play(card)) {
            target->m_game->queue_request<request_force_play_card>(origin_card, target, card);
        } else {
            discard_drawn_card();
        }
    }
}