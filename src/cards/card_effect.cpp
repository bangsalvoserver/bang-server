#include "card_effect.h"

#include "game/game.h"
#include "game/play_verify.h"

namespace banggame {

    void request_base::auto_pick() {
        auto update = target->m_game->make_request_update(target);
        if (update.pick_cards.size() == 1 && update.respond_cards.empty()) {
            on_pick(update.pick_cards.front());
        }
    }

    void request_base::auto_respond() {
        auto update = target->m_game->make_request_update(target);
        if (update.pick_cards.empty() && update.respond_cards.size() == 1) {
            card *origin_card = update.respond_cards.front().card;
            if (origin_card->equips.empty()
                && origin_card->optionals.empty()
                && !origin_card->is_modifier()
                && std::ranges::all_of(origin_card->responses, [](const effect_holder &holder) { return holder.target == target_type::none; })
            ) {
                apply_target_list(target, origin_card, true,
                    target_list{origin_card->responses.size(), play_card_target{enums::enum_tag<target_type::none>}}, {});
            }
        }
    }

    void request_timer::start(ticks total_update_time) {
        lifetime = duration + total_update_time;
    }

    void request_timer::tick() {
        if (request->state == request_state::live) {
            --lifetime;
        }
    }

    bool selection_picker::can_pick(card *target_card) const {
        return target_card->pocket == pocket_type::selection;
    }
    
    void event_equip::on_disable(card *target_card, player *target) {
        target->m_game->remove_listeners(target_card);
    }
}