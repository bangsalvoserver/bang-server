#include "ruleset.h"

#include "cards/game_events.h"
#include "cards/filter_enums.h"
#include "cards/game_enums.h"

#include "effects/base/requests.h"
#include "effects/base/steal_destroy.h"
#include "effects/wildwestshow/ruleset.h"

#include "game/game_table.h"
#include "game/game_options.h"

namespace banggame {

    static card_token_type get_card_pardner_token(card_ptr target_card) {
        auto tag_value = target_card->get_tag_value(tag_type::pardner);
        assert(tag_value && *tag_value >= 1 && *tag_value <= pardner_tokens.size());
        return pardner_tokens[*tag_value - 1];
    }

    player_ptr get_tracked_player(card_ptr target_card) {
        card_token_type token = get_card_pardner_token(target_card);
        for (player_ptr target : target_card->m_game->m_players) {
            if (target->alive() && target->tokens[token]) {
                return target;
            }
        }
        return nullptr;
    }

    bool is_tracked_player(card_ptr target_card, player_ptr target) {
        return target->alive() && target->tokens[get_card_pardner_token(target_card)];
    }

    void apply_pardner_token(card_ptr origin_card, player_ptr origin, player_ptr target) {
        origin->m_game->add_log("LOG_TRACKED_PLAYER", origin, origin_card, target);
        card_token_type token = get_card_pardner_token(origin_card);
        origin->m_game->add_tokens(token, 1, token_positions::card{ origin_card });
        origin->m_game->move_tokens(token, token_positions::card{ origin_card }, token_positions::player{ target }, 1);
    }

    void remove_pardner_token(card_ptr origin_card, player_ptr origin) {
        if (player_ptr target = get_tracked_player(origin_card)) {
            card_token_type token = get_card_pardner_token(origin_card);
            origin->m_game->move_tokens(token, token_positions::player{ target }, token_positions::card{ origin_card }, 1);
            origin->m_game->add_tokens(token, -1, token_positions::card{ origin_card });
        }
    }

    namespace contexts {
        struct tracked_player {
            player_ptr value;
        };
    }

    struct request_jackalope : request_resolvable, interface_picking {
        using request_resolvable::request_resolvable;
        
        bool can_pick(card_ptr target_card) const override {
            return target_card->pocket == pocket_type::main_deck
                || (target_card->pocket == pocket_type::discard_pile && target->m_game->m_deck.empty());
        }

        void on_pick(card_ptr target_card) override {
            pop_request();
            target->draw_card(2, origin_card);
        }

        void on_resolve() override {
            pop_request();
        }

        resolve_type get_resolve_type() const override {
            return resolve_type::dismiss;
        }

        prompt_string resolve_prompt() const override {
            return {"PROMPT_CANCEL_DRAW", origin_card};
        }

        game_string status_text(player_ptr owner) const override {
            if (target == owner) {
                return {"STATUS_JACKALOPE", origin_card};
            } else {
                return {"STATUS_JACKALOPE_OTHER", target, origin_card};
            }
        }
    };

    void ruleset_frontier::on_apply(game_ptr game) {
        if (!game->m_options.expansions.contains(GET_RULESET(wildwestshow))) {
            track_played_cards(game);
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

        game->add_listener<event_type::on_destroy_card>({ nullptr, 1 }, [](player_ptr origin, card_ptr target_card, bool is_destroy, destroy_flags &flags) {
            player_ptr target = target_card->owner;
            if (origin != target && target_card->name == "JACKALOPE") {
                origin->m_game->queue_action([=]{
                    if (target->alive()) {
                        target->m_game->queue_request<request_jackalope>(target_card, origin, target);
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

        game->add_listener<event_type::on_discard_all>(nullptr, [](player_ptr target) {
            if (!target->alive()) {
                for (card_token_type token : pardner_tokens) {
                    if (int count = target->tokens[token]) {
                        target->m_game->add_tokens(token, -count, token_positions::player{ target });
                    }
                }
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