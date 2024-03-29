#include "request_base.h"

#include "cards/filter_enums.h"

#include "game.h"
#include "play_verify.h"

namespace banggame {

    void request_picking::auto_pick() {
        if (card *only_card = get_single_element(get_all_playable_cards(target, true))) {
            if (only_card->has_tag(tag_type::pick)) {
                if (card *target_card = get_single_element(get_pick_cards(target))) {
                    on_pick(target_card);
                }
            }
        }
    }

    void request_base::auto_respond() {
        if (card *only_card = get_single_element(get_all_playable_cards(target, true))) {
            if (only_card->equips.empty()
                && only_card->optionals.empty()
                && !only_card->is_modifier()
                && rn::all_of(only_card->responses, [](const effect_holder &holder) { return holder.target == target_type::none; })
            ) {
                apply_target_list(target, only_card, true,
                    target_list{only_card->responses.size(), play_card_target{enums::enum_tag<target_type::none>}}, {});
            }
        }
    }

    bool selection_picker::can_pick(card *target_card) const {
        return target_card->pocket == pocket_type::selection;
    }
    
}