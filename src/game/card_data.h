#ifndef __CARD_DATA_H__
#define __CARD_DATA_H__

#include <vector>
#include <string>

#include "holders.h"
#include "card_enums.h"

namespace banggame {

    using effect_list = std::vector<effect_holder>;
    using equip_list = std::vector<equip_holder>;
    using tag_list = std::vector<tag_holder>;

    struct card_data {REFLECTABLE(
        (int) id,

        (std::string) name,
        (std::string) image,

        (effect_list) effects,
        (effect_list) responses,
        (effect_list) optionals,
        (equip_list) equips,
        (tag_list) tags,

        (card_expansion_type) expansion,
        (card_deck_type) deck,

        (card_modifier_type) modifier,
        (mth_holder) multi_target_handler,
        (target_player_filter) equip_target,
        
        (card_sign) sign,
        (card_color_type) color
    )

        bool has_tag(tag_type tag) const {
            return std::ranges::find(tags, tag, &tag_holder::type) != tags.end();
        }

        std::optional<short> get_tag_value(tag_type tag) const {
            if (auto it = std::ranges::find(tags, tag, &tag_holder::type); it != tags.end()) {
                return it->tag_value;
            } else {
                return std::nullopt;
            }
        }

        bool self_equippable() const {
            return equip_target == target_player_filter{};
        }
    
        short buy_cost() const {
            return get_tag_value(tag_type::buy_cost).value_or(0);
        }
    };

}

#endif