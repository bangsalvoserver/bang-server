#include "card_effect.h"

#include "game/game.h"
#include "game/play_verify.h"

namespace banggame {

    bool request_base::auto_pick() {
        auto update = target->m_game->make_request_update(target);
        if (update.pick_cards.size() == 1 && update.respond_cards.empty()) {
            on_pick(update.pick_cards.front());
            return true;
        }
        return false;
    }

    bool request_base::auto_respond() {
        auto update = target->m_game->make_request_update(target);
        if (update.pick_cards.empty() && update.respond_cards.size() == 1) {
            card *origin_card = update.respond_cards.front();
            if (origin_card->equips.empty()
                && origin_card->optionals.empty()
                && origin_card->modifier == card_modifier_type::none
                && std::ranges::all_of(origin_card->responses, [](const effect_holder &holder) { return holder.target == target_type::none; })
            ) {
                auto lock = target->m_game->lock_updates();
                play_card_verify{target, origin_card, true,
                    target_list{origin_card->responses.size(), play_card_target{enums::enum_tag<target_type::none>}}}.do_play_card();
                return true;
            }
        }
        return false;
    }

    void request_timer::start() {
        if (!request->sent) {
            lifetime = duration + request->target->m_game->get_total_update_time();
        }
    }

    void request_timer::tick() {
        if (request->sent && --lifetime <= ticks{0}) {
            auto lock = request->target->m_game->lock_updates(true);
            on_finished();
        }
    }

    bool selection_picker::can_pick(card *target_card) const {
        return target_card->pocket == pocket_type::selection;
    }
    
    void event_equip::on_disable(card *target_card, player *target) {
        target->m_game->remove_listeners(target_card);
    }
}