#include "filters.h"

#include "game/game.h"

namespace banggame::filters::detail {

    bool check_player_flags(const player *origin, player_flags flags) {
        return origin->check_player_flags(flags);
    }

    bool check_game_flags(const player *origin, game_flags flags) {
        return origin->m_game->check_flags(flags);
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

    int count_player_hand_cards(player *origin) {
        return int(origin->m_hand.size());
    }

    int count_player_table_cards(player *origin) {
        return int(std::ranges::count_if(origin->m_table, std::not_fn(&card::is_black)));
    }

    int count_player_cubes(player *origin) {
        return origin->count_cubes();
    }

    int get_distance(player *origin, player *target) {
        return origin->m_game->calc_distance(origin, target);
    }

    card_sign get_card_sign(card *target) {
        return target->sign;
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
        return target == target->owner->first_character() || target->is_orange() && target->pocket == pocket_type::player_table;
    }

    std::optional<short> get_card_tag(card *target, tag_type type) {
        return target->get_tag_value(type);
    }

    card *get_request_origin_card(player *origin) {
        if (auto req = origin->m_game->top_request()) {
            return req->origin_card;
        }
        return nullptr;
    }

    player *get_request_origin(player *origin) {
        if (auto req = origin->m_game->top_request()) {
            return req->origin;
        }
        return nullptr;
    }
}