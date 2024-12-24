#ifndef __CARD_DATA_H__
#define __CARD_DATA_H__

#include <vector>
#include <string>

#include "card_effect.h"

namespace banggame {

    struct card_data {
        std::string name;
        std::string image;

        effect_list effects;
        effect_list responses;
        equip_list equips;
        tag_map tags;

        expansion_set expansion;
        card_deck_type deck;

        modifier_holder modifier;
        modifier_holder modifier_response;
        mth_holder mth_effect;
        mth_holder mth_response;
        enums::bitset<target_player_filter> equip_target;
        
        card_color_type color;
        card_sign sign;

        const effect_list &get_effect_list(bool is_response) const {
            return is_response ? responses : effects;
        }

        const mth_holder &get_mth(bool is_response) const {
            return is_response ? mth_response : mth_effect;
        }

        const modifier_holder &get_modifier(bool is_response) const {
            return is_response ? modifier_response : modifier;
        }

        bool has_tag(tag_type tag) const {
            return tags.find(tag) != tags.end();
        }

        std::optional<short> get_tag_value(tag_type tag) const {
            if (auto it = tags.find(tag); it != tags.end()) {
                return it->second;
            } else {
                return std::nullopt;
            }
        }

        bool self_equippable() const {
            return equip_target.empty();
        }

        bool is_brown() const { return color == card_color_type::brown; }
        bool is_blue() const { return color == card_color_type::blue; }
        bool is_green() const { return color == card_color_type::green; }
        bool is_black() const { return color == card_color_type::black; }
        bool is_orange() const { return color == card_color_type::orange; }
        bool is_train() const { return color == card_color_type::train; }
    };

    using card_initializer_list = std::initializer_list<card_data>;

    struct all_cards_t {
        card_initializer_list deck;
        card_initializer_list characters;
        card_initializer_list goldrush;
        card_initializer_list highnoon;
        card_initializer_list fistfulofcards;
        card_initializer_list wildwestshow;
        card_initializer_list button_row;
        card_initializer_list stations;
        card_initializer_list train;
        card_initializer_list locomotive;
        card_initializer_list hidden;

        std::initializer_list<ruleset_ptr> expansions;
    };

    extern const all_cards_t all_cards;

}

#endif