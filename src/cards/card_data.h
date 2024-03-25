#ifndef __CARD_DATA_H__
#define __CARD_DATA_H__

#include <vector>
#include <string>

#include "card_enums.h"
#include "holders.h"

namespace banggame {

    DEFINE_STRUCT(modifier_holder,
        (serial::modifier_type, type)
    )

    DEFINE_STRUCT(mth_holder,
        (serial::mth_type, type)
        (serial::int_list, args)
    )

    DEFINE_STRUCT(tag_holder,
        (short, tag_value)
        (tag_type, type)
    )

    using effect_list = std::vector<effect_holder>;
    using equip_list = std::vector<equip_holder>;
    using tag_list = std::vector<tag_holder>;

    DEFINE_STRUCT(card_data,
        (std::string, name)
        (std::string, image)

        (effect_list, effects)
        (effect_list, responses)
        (effect_list, optionals)
        (equip_list, equips)
        (tag_list, tags)

        (expansion_type, expansion)
        (card_deck_type, deck)

        (modifier_holder, modifier)
        (mth_holder, mth_effect)
        (mth_holder, mth_response)
        (target_player_filter, equip_target)
        
        (card_color_type, color)
        (card_sign, sign),

        const effect_list &get_effect_list(bool is_response) const {
            return is_response ? responses : effects;
        }

        const mth_holder &get_mth(bool is_response) const {
            return is_response ? mth_response : mth_effect;
        }

        bool has_tag(tag_type tag) const {
            return rn::contains(tags, tag, &tag_holder::type);
        }

        bool is_modifier() const {
            return modifier.type != nullptr;
        }

        std::optional<short> get_tag_value(tag_type tag) const {
            if (auto it = rn::find(tags, tag, &tag_holder::type); it != tags.end()) {
                return it->tag_value;
            } else {
                return std::nullopt;
            }
        }

        bool self_equippable() const {
            return equip_target == target_player_filter{};
        }

        bool is_brown() const { return color == card_color_type::brown; }
        bool is_blue() const { return color == card_color_type::blue; }
        bool is_green() const { return color == card_color_type::green; }
        bool is_black() const { return color == card_color_type::black; }
        bool is_orange() const { return color == card_color_type::orange; }
        bool is_train() const { return color == card_color_type::train; }
    )

    struct all_cards_t {
        std::vector<card_data> deck;
        std::vector<card_data> characters;
        std::vector<card_data> goldrush;
        std::vector<card_data> highnoon;
        std::vector<card_data> fistfulofcards;
        std::vector<card_data> wildwestshow;
        std::vector<card_data> button_row;
        std::vector<card_data> stations;
        std::vector<card_data> train;
        std::vector<card_data> locomotive;
        std::vector<card_data> hidden;
    };

    extern const all_cards_t all_cards;

}

#endif