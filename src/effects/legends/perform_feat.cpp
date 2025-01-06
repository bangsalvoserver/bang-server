#include "perform_feat.h"

#include "ruleset.h"

#include "effects/base/pick.h"
#include "effects/base/resolve.h"

#include "game/game.h"
#include "game/prompts.h"

namespace banggame {

    int get_count_performed_feats(player_ptr origin) {
        int num_feats = 0;
        origin->m_game->call_event(event_type::count_performed_feats{origin, num_feats});
        return num_feats;
    }

    std::pair<card_token_type, int> get_card_fame_token_type(const_card_ptr origin_card) {
        for (const auto &[token, count] : origin_card->tokens) {
            if (token != card_token_type::cube && count > 0) {
                return {token, count};
            }
        }
        return {card_token_type::cube, 0};
    }

    bool effect_perform_feat::can_play(card_ptr origin_card, player_ptr origin) {
        if (origin_card->deck == card_deck_type::feats && origin->first_character()->deck != card_deck_type::legends) {
            auto [token, count] = get_card_fame_token_type(origin->first_character());
            if (count == 0) {
                return false;
            }
        }

        return get_count_performed_feats(origin) == 0;
    }

    struct request_damage_legend : request_resolvable {
        request_damage_legend(player_ptr target)
            : request_resolvable{nullptr, nullptr, target} {}

        void on_update() override {
            auto_resolve();
        }

        int resolve_type() const override {
            return 1;
        }

        void on_resolve() override {
            target->m_game->pop_request();
        }

        game_string status_text(player_ptr owner) const override {
            if (owner == target) {
                return "STATUS_DAMAGE_LEGEND";
            } else {
                return {"STATUS_DAMAGE_LEGEND_OTHER", target};
            }
        }
    };

    bool effect_damage_legend::can_play(card_ptr origin_card, player_ptr origin) {
        return origin->m_game->top_request<request_damage_legend>(target_is{origin}) != nullptr;
    }

    game_string effect_damage_legend::get_error(card_ptr origin_card, player_ptr origin, player_ptr target) {
        if (target->m_hp <= 1) {
            bool can_kill = false;
            target->m_game->call_event(event_type::check_damage_legend_kill{target, can_kill});
            if (!can_kill) {
                return "ERROR_DAMAGE_LEGEND_KILL";
            }
        }
        return {};
    }

    game_string effect_damage_legend::on_prompt(card_ptr origin_card, player_ptr origin, player_ptr target) {
        MAYBE_RETURN(prompts::bot_check_target_enemy(origin, target));
        return {};
    }

    void effect_damage_legend::on_play(card_ptr origin_card, player_ptr origin, player_ptr target) {
        origin->m_game->pop_request();
        target->m_game->add_log("LOG_DAMAGED_LEGEND", origin, target);
        target->damage(nullptr, origin, 1);
    }

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
            if (origin->first_character()->deck == card_deck_type::legends) {
                origin->m_game->add_log("LOG_FEAT_CLAIMED", origin, origin_card);
                origin->m_game->queue_request<request_damage_legend>(origin);
                origin->m_game->queue_action([=]{
                    origin_card->drop_all_tokens();
                    origin_card->move_to(pocket_type::feats_discard);
                    draw_next_feat(origin);
                });
            } else {
                card_ptr character_card = origin->first_character();
                auto [token, count] = get_card_fame_token_type(character_card);
                origin->m_game->add_log("LOG_FEAT_PERFORMED", origin, origin_card);
                character_card->move_tokens(token, origin_card, std::min<int>(count, 2));
            }
        }
    }
}