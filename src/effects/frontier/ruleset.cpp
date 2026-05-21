#include "ruleset.h"

#include "cards/game_events.h"
#include "cards/filter_enums.h"
#include "cards/game_enums.h"

#include "effects/base/draw.h"
#include "effects/base/requests.h"
#include "effects/base/steal_destroy.h"

#include "effects/dodgecity/ruleset.h"
#include "effects/wildwestshow/ruleset.h"

#include "game/game_table.h"
#include "game/game_options.h"

namespace banggame {

    namespace event_type {
        struct get_tracked_player {
            using result_type = player_ptr;
            card_ptr target_card;
        };
    }

    static card_token_type get_card_pardner_token(card_ptr target_card) {
        auto tag_value = target_card->get_tag_value(tag_type::pardner);
        assert(tag_value && *tag_value >= 1 && *tag_value <= pardner_tokens.size());
        return pardner_tokens[*tag_value - 1];
    }

    player_ptr get_tracked_player(card_ptr target_card) {
        return target_card->m_game->call_event(event_type::get_tracked_player{ target_card });
    }

    void apply_pardner_token(card_ptr origin_card, player_ptr origin, player_ptr target) {
        origin->m_game->add_log("LOG_TRACKED_PLAYER", origin, origin_card, target);
        card_token_type token = get_card_pardner_token(origin_card);
        origin->m_game->add_tokens(token, 1, token_positions::card{ origin_card });
        origin->m_game->move_tokens(token, token_positions::card{ origin_card }, token_positions::player{ target }, 1);

        event_card_key key{ origin_card, -4 };
        origin->m_game->add_listener<event_type::get_tracked_player>(key, [=](card_ptr target_card) {
            return origin_card == target_card ? target : nullptr;
        });

        origin->m_game->add_listener<event_type::on_discard_all>(key, [=](player_ptr e_target) {
            if (target == e_target && !target->alive()) {
                target->m_game->move_tokens(token, token_positions::player{ target }, token_positions::card{ origin_card }, 1);
                target->m_game->add_tokens(token, -1, token_positions::card{ origin_card });
                target->m_game->remove_listeners(key);
            }
        });
    }

    void remove_pardner_token(card_ptr origin_card, player_ptr origin) {
        if (player_ptr target = get_tracked_player(origin_card)) {
            card_token_type token = get_card_pardner_token(origin_card);
            origin->m_game->move_tokens(token, token_positions::player{ target }, token_positions::card{ origin_card }, 1);
            origin->m_game->add_tokens(token, -1, token_positions::card{ origin_card });

            origin->m_game->remove_listeners({ origin_card, -4 });
        }
    }

    namespace contexts {
        struct tracked_player {
            player_ptr value;
        };
    }

    void ruleset_frontier::on_apply(game_ptr game) {
        if (!game->m_options.expansions.contains(GET_RULESET(wildwestshow))) {
            track_played_cards(game);
        }
        
        if (!game->m_options.expansions.contains(GET_RULESET(dodgecity))) {
            ruleset_dodgecity{}.on_apply(game);
        }

        game->add_listener<event_type::on_discard_pass>({ nullptr, 1 }, [](player_ptr origin, card_ptr target_card) {
            if (!origin->is_ghost() && target_card->name == "HEAVY_GRUB") {
                origin->m_game->add_log("LOG_CARD_HAS_EFFECT", target_card);
                if (origin->m_game->check_flags(game_flag::phase_one_draw_discard)) {
                    target_card->set_visibility(card_visibility::shown);
                    target_card->add_short_pause();
                    target_card->set_visibility(card_visibility::hidden);
                }
                origin->heal(target_card, origin, 3);
            }
        });

        game->add_listener<event_type::on_destroy_card>({ nullptr, 1 }, [](player_ptr origin, card_ptr origin_card, card_ptr target_card, bool is_destroy, destroy_flags &flags) {
            player_ptr target = target_card->owner;
            if (origin != target && target_card->name == "JACKALOPE") {
                origin->m_game->queue_action([=]{
                    if (target->alive()) {
                        target->m_game->queue_request<request_can_draw>(target_card, origin, target, 2);
                    }
                }, 41);
            }
        });
        
        game->add_listener<event_type::on_equip_card>(nullptr, [](player_ptr origin, player_ptr target, card_ptr target_card, const effect_context &ctx) {
            if (target_card->is_purple()) {
                player_ptr tracked_player = ctx.get<contexts::tracked_player>();
                apply_pardner_token(target_card, origin, tracked_player ? tracked_player : origin);
            }
        });
    }

    game_string effect_track::on_prompt(card_ptr origin_card, player_ptr origin, player_ptr target) {
        if (origin == target) {
            return {"PROMPT_TRACK_SELF", origin_card};
        }
        return {};
    }

    void effect_track::add_context(card_ptr origin_card, player_ptr origin, player_ptr target, effect_context &ctx) {
        ctx.set<contexts::tracked_player>(target);
    }
}