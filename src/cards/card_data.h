#ifndef __CARD_DATA_H__
#define __CARD_DATA_H__

#include <vector>
#include <string>

#include "card_effect.h"

namespace banggame {

    struct card_data {
        std::string_view name;
        std::string_view image;

        effect_list effects;
        effect_list responses;
        equip_list equips;
        tag_map tags;

        expansion_list expansion;
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
            return rn::contains(tags, tag, &tag_value_pair::first);
        }

        std::optional<short> get_tag_value(tag_type tag) const {
            auto it = rn::find(tags, tag, &tag_value_pair::first);
            if (it != tags.end()) return it->second;
            return std::nullopt;
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

    using card_data_list = std::span<card_data>;

    struct all_cards_t {
        card_data_list deck;
        card_data_list characters;
        card_data_list goldrush;
        card_data_list highnoon;
        card_data_list fistfulofcards;
        card_data_list wildwestshow;
        card_data_list button_row;
        card_data_list stations;
        card_data_list train;
        card_data_list locomotive;
        card_data_list legends;
        card_data_list feats;
        card_data_list hidden;

        expansion_list expansions;
    };

    extern const all_cards_t all_cards;

}

#endif