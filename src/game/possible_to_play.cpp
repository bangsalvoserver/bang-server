#include "player.h"

#include "game.h"

#include "play_verify.h"

namespace banggame {
    using namespace enums::flag_operators;

    std::vector<player *> player::make_equip_set(card *card) {
        std::vector<player *> ret;
        if (card->self_equippable()) {
            if (!find_equipped_card(card)) {
                ret.push_back(this);
            }
        } else {
            for (player &p : m_game->m_players) {
                if (!check_player_filter(this, card->equip_target, &p) && !p.find_equipped_card(card)) {
                    ret.push_back(&p);
                }
            }
        }
        return ret;
    }

    std::vector<player *> player::make_player_target_set(card *origin_card, const effect_holder &holder) {
        std::vector<player *> ret;
        for (player &target : m_game->m_players) {
            if (!check_player_filter(this, holder.player_filter, &target)
                && !holder.verify(origin_card, this, &target)) {
                ret.push_back(&target);
            }
        }
        return ret;
    }

    std::vector<card *> player::make_card_target_set(card *origin_card, const effect_holder &holder) {
        std::vector<card *> ret;
        auto add_if_valid = [&](card *target_card) {
            if (!check_card_filter(origin_card, this, holder.card_filter, target_card)
                && !holder.verify(origin_card, this, target_card)) {
                ret.push_back(target_card);
            }
        };
        for (player *target : make_player_target_set(origin_card, holder)) {
            std::ranges::for_each(target->m_hand, add_if_valid);
            std::ranges::for_each(target->m_table, add_if_valid);
        }
        return ret;
    }

    bool player::is_possible_to_play(card *target_card, bool is_response) {
        auto &effects = is_response ? target_card->responses : target_card->effects;
        if (effects.empty()) return false;

        return std::ranges::all_of(effects, [&](const effect_holder &holder) {
            switch (holder.target) {
            case target_type::none:
                return !holder.verify(target_card, this);
            case target_type::player:
                return !make_player_target_set(target_card, holder).empty();
            case target_type::card:
            case target_type::extra_card:
                return !make_card_target_set(target_card, holder).empty();
            case target_type::cards:
                return make_card_target_set(target_card, holder).size() >= std::max<size_t>(1, holder.target_value);
            case target_type::select_cubes:
                return count_cubes() >= holder.target_value;
            case target_type::self_cubes:
                return target_card->num_cubes >= holder.target_value;
            default:
                return true;
            }
        });
    }
}