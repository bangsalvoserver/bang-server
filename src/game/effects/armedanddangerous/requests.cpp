#include "requests.h"
#include "effects.h"

#include "../../game.h"

namespace banggame {

    bool request_add_cube::can_pick(pocket_type pocket, player *target_player, card *target_card) const {
        if (pocket == pocket_type::player_character) {
            target_card = target->m_characters.front();
        } else if (pocket != pocket_type::player_table || target_card->color != card_color_type::orange) {
            return false;
        }
        return target_player == target && target_card->num_cubes < max_cubes;
    }

    void request_add_cube::on_pick(pocket_type pocket, player *target_player, card *target_card) {
        if (pocket == pocket_type::player_character) {
            target_card = target->m_characters.front();
        }
        if (--ncubes == 0) {
            target->m_game->pop_request();
        }
        target->add_cubes(target_card, 1);
        target->m_game->update_request();
    }

    game_string request_add_cube::status_text(player *owner) const {
        if (owner == target) {
            if (ncubes == 1) {
                if (origin_card) {
                    return {"STATUS_ADD_CUBE_FOR", origin_card};
                } else {
                    return "STATUS_ADD_CUBE";
                }
            } else if (origin_card) {
                return {"STATUS_ADD_CUBE_PLURAL_FOR", origin_card, ncubes};
            } else {
                return {"STATUS_ADD_CUBE_PLURAL", ncubes};
            }
        } else if (ncubes == 1) {
            if (origin_card) {
                return {"STATUS_ADD_CUBE_FOR_OTHER", target, origin_card};
            } else {
                return {"STATUS_ADD_CUBE_OTHER", target};
            }
        } else if (origin_card) {
            return {"STATUS_ADD_CUBE_PLURAL_FOR_OTHER", target, origin_card, ncubes};
        } else {
            return {"STATUS_ADD_CUBE_PLURAL_OTHER", target, ncubes};
        }
    }

    game_string request_move_bomb::status_text(player *owner) const {
        if (target == owner) {
            return {"STATUS_MOVE_BOMB", origin_card};
        } else {
            return {"STATUS_MOVE_BOMB_OTHER", target, origin_card};
        }
    }

    void request_rust::on_finished() {
        effect_rust{}.on_resolve(origin_card, origin, target);
    }

    game_string request_rust::status_text(player *owner) const {
        if (target == owner) {
            return {"STATUS_RUST", origin_card};
        } else {
            return {"STATUS_RUST_OTHER", target, origin_card};
        }
    }

    game_string timer_al_preacher::status_text(player *owner) const {
        if (target == owner) {
            return {"STATUS_CAN_PLAY_CARD", origin_card};
        } else {
            return {"STATUS_CAN_PLAY_CARD_OTHER", target, origin_card};
        }
    }

    game_string timer_tumbleweed::status_text(player *owner) const {
        if (target == owner) {
            return {"STATUS_CAN_PLAY_TUMBLEWEED", origin, origin_card, target_card, drawn_card};
        } else {
            return {"STATUS_CAN_PLAY_TUMBLEWEED_OTHER", origin, origin_card, target_card, drawn_card, target};
        }
    }

}