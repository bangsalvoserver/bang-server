#include "jail.h"

#include "cards/filter_enums.h"

#include "predraw_check.h"
#include "draw_check.h"

#include "game/game_table.h"
#include "game/prompts.h"

namespace banggame {

    game_string equip_jail::on_prompt(card_ptr origin_card, player_ptr origin, player_ptr target) {
        MAYBE_RETURN(prompts::bot_check_target_enemy(origin, target, true));
        MAYBE_RETURN(prompts::prompt_target_self(origin_card, origin, target));
        return {};
    }

    void equip_jail::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::on_predraw_check>(target_card, [=](player_ptr p, card_ptr e_card) {
            if (p == target && e_card == target_card) {
                target->m_game->queue_request<request_check>(target, target_card, &card_sign::is_hearts, [=](bool result) {
                    target->discard_card(target_card);
                    if (result) {
                        target->m_game->add_log("LOG_JAIL_BREAK", target);
                    } else {
                        target->skip_turn();
                    }
                });
            }
        });
    }

    static constexpr auto checking_card_is_jail = [](const event_card_key &key) {
        return key.target_card->has_tag(tag_type::jail);
    };

    bool effect_escape_jail::can_play(card_ptr origin_card, player_ptr origin) {
        if (auto req = origin->m_game->top_request<request_predraw>(target_is{origin})) {
            return rn::any_of(req->get_checking_cards(), checking_card_is_jail);
        }
        return false;
    }

    void effect_escape_jail::on_play(card_ptr origin_card, player_ptr origin, card_ptr target_card) {
        auto req = origin->m_game->top_request<request_predraw>();
        card_ptr jail_card = rn::find_if(req->get_checking_cards(), checking_card_is_jail)->target_card;
        req->remove_check(jail_card);

        effect_discard::on_play(origin_card, origin, target_card);

        origin->discard_card(jail_card);
        origin->m_game->add_log("LOG_JAIL_BREAK", origin);
    }
}