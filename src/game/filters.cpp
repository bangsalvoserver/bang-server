#include "filters.h"

#include "game.h"

namespace banggame::filter_impl {
    
    int get_player_hp(player *origin) {
        return origin->m_hp;
    }

    bool check_player_flags(player *origin, player_flags flags) {
        return origin->check_player_flags(flags);
    }

    bool is_player_alive(player *origin) {
        return origin->alive();
    }

    player_role get_player_role(player *origin) {
        return origin->m_role;
    }

    int get_player_range_mod(player *origin) {
        return origin->m_range_mod;
    }

    int get_player_weapon_range(player *origin) {
        return origin->m_weapon_range;
    }

    int get_distance(player *origin, player *target) {
        return origin->m_game->call_event<event_type::apply_distance_modifier>(origin, origin->m_game->calc_distance(origin, target));
    }

    bool is_bangcard(player *origin, card *target) {
        return origin->is_bangcard(target);
    }

    card_sign get_card_sign(player *origin, card *target) {
        return origin->get_card_sign(target);
    }

    card_color_type get_card_color(card *target) {
        return target->color;
    }

    pocket_type get_card_pocket(card *target) {
        return target->pocket;
    }

    card_deck_type get_card_deck(card *target) {
        return target->deck;
    }

    bool is_cube_slot(card *target) {
        return target == target->owner->m_characters.front() || target->color == card_color_type::orange;
    }

    bool card_has_tag(card *target, tag_type type) {
        return target->has_tag(type);
    }
}