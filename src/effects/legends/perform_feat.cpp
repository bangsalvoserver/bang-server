#include "perform_feat.h"

#include "ruleset.h"

#include "effects/base/pick.h"

#include "game/game.h"
#include "game/prompts.h"

namespace banggame {

    int get_count_performed_feats(player_ptr origin) {
        int num_feats = 0;
        origin->m_game->call_event(event_type::count_performed_feats{origin, num_feats});
        return num_feats;
    }
    
    bool is_legend(const_player_ptr origin) {
        return origin->first_character()->deck == card_deck_type::legends;
    }

    std::pair<card_token_type, int> get_card_fame_token_type(const_card_ptr origin_card) {
        for (const auto &[token, count] : origin_card->tokens) {
            if (token != card_token_type::cube && count > 0) {
                return {token, count};
            }
        }
        return {card_token_type::cube, 0};
    }

    static bool can_claim_feat_on(player_ptr origin, const_player_ptr target) {
        if (target != origin && is_legend(target)) {
            if (target->m_hp > 1) {
                return true;
            } else {
                bool can_kill = false;
                origin->m_game->call_event(event_type::check_claim_feat_kill{origin, can_kill});
                return can_kill;
            }
        }
        return false;
    }

    bool effect_perform_feat::can_play(card_ptr origin_card, player_ptr origin) {
        if (origin_card->deck == card_deck_type::feats) {
            if (is_legend(origin)) {
                if (rn::none_of(origin->m_game->m_players, [&](const_player_ptr target) {
                    return can_claim_feat_on(origin, target);
                })) {
                    return false;
                }
            } else {
                auto [token, count] = get_card_fame_token_type(origin->first_character());
                if (count == 0) {
                    return false;
                }
            }
        }

        return get_count_performed_feats(origin) == 0;
    }

    struct request_claim_feat : request_picking_player {
        using request_picking_player::request_picking_player;

        bool can_pick(const_player_ptr target_player) const override {
            return can_claim_feat_on(target, target_player);
        }

        void on_pick(player_ptr target_player) override {
            target->m_game->pop_request();
            target->m_game->add_log("LOG_FEAT_CLAIMED", target, target_player);
            target_player->damage(origin_card, target, 1);
            target->m_game->queue_action([origin_card=origin_card, target=target]{
                origin_card->drop_all_tokens();
                origin_card->move_to(pocket_type::feats_discard);
                draw_next_feat(target);
            });
        }
        
        prompt_string pick_prompt(player_ptr target_player) const override {
            MAYBE_RETURN(prompts::bot_check_target_enemy(target, target_player));
            return {};
        }

        game_string status_text(player_ptr owner) const override {
            if (owner == target) {
                return {"STATUS_CLAIM_FEAT", origin_card};
            } else {
                return {"STATUS_CLAIM_FEAT_OTHER", origin_card, target};
            }
        }
    };

    void effect_perform_feat::on_play(card_ptr origin_card, player_ptr origin) {
        event_card_key key{origin_card, 5};
        origin->m_game->add_listener<event_type::count_performed_feats>(key, [=](player_ptr p, int &num_feats) {
            if (origin == p) {
                ++num_feats;
            }
        });
        origin->m_game->add_listener<event_type::on_turn_end>(key, [=](player_ptr p, bool skipped) {
            if (origin == p) {
                origin->m_game->remove_listeners(key);
            }
        });

        if (origin_card->deck == card_deck_type::feats) {
            if (is_legend(origin)) {
                origin->m_game->queue_request<request_claim_feat>(origin_card, origin, origin);
            } else {
                card_ptr character_card = origin->first_character();
                auto [token, count] = get_card_fame_token_type(character_card);
                origin->m_game->add_log("LOG_FEAT_PERFORMED", origin, origin_card);
                character_card->move_tokens(token, origin_card, std::min<int>(count, 2));
            }
        }
    }
}