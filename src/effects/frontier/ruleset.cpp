#include "ruleset.h"

#include "cards/game_events.h"

#include "game/game_table.h"

#include "cards/filter_enums.h"

#include "effects/base/requests.h"

namespace banggame {

    
    card_token_type get_card_pardner_token(card_ptr target_card) {
        return get_pardner_token(target_card->get_tag_value(tag_type::pardner).value());
    }

    player_ptr get_tracked_player(card_ptr target_card) {
        if (target_card->is_purple()) {
            card_token_type token = get_card_pardner_token(target_card);
            for (player_ptr target : target_card->m_game->m_players) {
                if (target->alive() && target->tokens[token]) {
                    return target;
                }
            }
        }
        return nullptr;
    }

    void apply_pardner_token(card_ptr origin_card, player_ptr origin, player_ptr target) {
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

    void ruleset_frontier::on_apply(game_ptr game) {
        // TODO add_listener on_discard_pass
        // when discarding "HEAVY_GRUB" -> heal(3)

        // TODO add_listener on_destroy_card
        // when discarding "JACKALOPE" -> target draw(2)

        // TODO add_listener on_discard_pass (new event)
        // when discarding "FEUD" -> return error
        
        game->add_listener<event_type::on_equip_card>(nullptr, [](player_ptr origin, player_ptr target, card_ptr target_card, const effect_context &ctx) {
            if (target_card->is_purple()) {
                player_ptr tracked_player = ctx.get<contexts::tracked_player>();
                apply_pardner_token(target_card, origin, tracked_player ? tracked_player : origin);
            }
        });

        game->add_listener<event_type::on_discard_all>(nullptr, [](player_ptr target) {
            if (!target->alive()) {
                for (int i=1; i<=6; ++i) {
                    card_token_type token = get_pardner_token(i);
                    if (int count = target->tokens[token]) {
                        target->m_game->add_tokens(token, -count, token_positions::player{ target });
                    }
                }
            }
        });
    }

    void effect_track::add_context(card_ptr origin_card, player_ptr origin, player_ptr target, effect_context &ctx) {
        ctx.set<contexts::tracked_player>(target);
    }
}