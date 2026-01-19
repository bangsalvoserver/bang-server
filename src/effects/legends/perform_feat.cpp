#include "perform_feat.h"

#include "ruleset.h"

#include "effects/base/can_play_card.h"
#include "effects/base/pick.h"
#include "effects/base/resolve.h"

#include "cards/game_events.h"

#include "game/game_table.h"
#include "game/prompts.h"
#include "game/bot_suggestion.h"

namespace banggame {

    int get_count_performed_feats(player_ptr origin) {
        int num_feats = 0;
        origin->m_game->call_event(event_type::count_performed_feats{origin, num_feats});
        return num_feats;
    }

    std::pair<card_token_type, int> get_player_fame_tokens(const_player_ptr origin) {
        for (const auto &[token, count] : origin->tokens) {
            if (is_fame_token(token) && count > 0) {
                return {token, count};
            }
        }
        return {};
    }

    bool effect_perform_feat::can_play(card_ptr origin_card, player_ptr origin) {
        if (origin_card->deck == card_deck_type::feats && !is_legend(origin)) {
            auto [token, count] = get_player_fame_tokens(origin);
            if (count == 0 || origin_card->num_tokens(token) != 0) {
                return false;
            }
        }

        return get_count_performed_feats(origin) == 0;
    }

    static bool can_damage_legend_kill(player_ptr target) {
        return target->m_game->call_event(event_type::check_damage_legend_kill{ target });
    }

    static bool is_valid_damage_legend_target(const_player_ptr origin, const_player_ptr target, bool can_kill) {
        return origin != target && target->alive() && is_legend(target)
            && (target->m_hp > 1 || can_kill);
    }

    struct request_damage_legend : request_resolvable, interface_target_set_players {
        request_damage_legend(card_ptr origin_card, player_ptr target)
            : request_resolvable{origin_card, nullptr, target} {}

        bool can_kill = false;

        void on_update() override {
            if (update_count == 0) {
                can_kill = can_damage_legend_kill(target);
            }
            auto_resolve();
        }

        resolve_type get_resolve_type() const override {
            return resolve_type::dismiss;
        }

        void on_resolve() override {
            target->m_game->pop_request();
        }
        
        bool in_target_set(const_player_ptr target_player) const override {
            return is_valid_damage_legend_target(target, target_player, can_kill);
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

    game_string effect_damage_legend::on_prompt(card_ptr origin_card, player_ptr origin, player_ptr target) {
        MAYBE_RETURN(prompts::bot_check_target_enemy(origin, target));
        return {};
    }

    void effect_damage_legend::on_play(card_ptr origin_card, player_ptr origin, player_ptr target) {
        origin->m_game->pop_request();
        target->m_game->add_log("LOG_DAMAGED_LEGEND", origin, target);
        target->damage(origin_card, origin, 1);
    }

    struct request_perform_feat : request_resolvable, interface_picking {
        request_perform_feat(player_ptr target)
            : request_resolvable{nullptr, nullptr, target, {}, 30} {}
        
        card_list target_cards;
        
        void on_update() override {
            if (target->alive() && target == target->m_game->m_playing) {
                if (update_count == 0) {
                    target->m_game->call_event(event_type::get_performable_feats{ target, target_cards });   

                    if (target_cards.empty()) {
                        target->m_game->pop_request();
                    }
                }
            } else {
                target->m_game->pop_request();
            }
        }

        resolve_type get_resolve_type() const override {
            return resolve_type::dismiss;
        }

        card_list get_highlights(player_ptr owner) const override {
            if (owner != target) {
                return target_cards;
            }
            return {};
        }

        void on_resolve() override {
            target->m_game->pop_request();
        }

        prompt_string resolve_prompt() const override {
            if (!is_legend(target)) {
                return "PROMPT_CANCEL_PERFORM_FEAT";
            }
            return {};
        }

        bool can_pick(const_card_ptr target_card) const override {
            return rn::contains(target_cards, target_card);
        }

        prompt_string pick_prompt(card_ptr target_card) const override {
            return effect_perform_feat{}.on_prompt(target_card, target);
        }

        void on_pick(card_ptr target_card) override {
            target->m_game->pop_request();
            effect_perform_feat{}.on_play(target_card, target);
        }

        game_string status_text(player_ptr owner) const override {
            if (target_cards.size() == 1) {
                card_ptr target_card = target_cards.front();
                if (is_legend(target)) {
                    if (owner == target) {
                        return {"STATUS_CLAIM_FEAT", target_card};
                    } else {
                        return {"STATUS_CLAIM_FEAT_OTHER", target_card, target};
                    }
                } else {
                    if (owner == target) {
                        return {"STATUS_PERFORM_FEAT", target_card};
                    } else {
                        return {"STATUS_PERFORM_FEAT_OTHER", target_card, target};
                    }
                }
            } else {
                if (is_legend(target)) {
                    if (owner == target) {
                        return "STATUS_CLAIM_A_FEAT";
                    } else {
                        return {"STATUS_CLAIM_A_FEAT_OTHER", target};
                    }
                } else {
                    if (owner == target) {
                        return "STATUS_PERFORM_A_FEAT";
                    } else {
                        return {"STATUS_PERFORM_A_FEAT_OTHER", target};
                    }
                }
            }
        }
    };

    void queue_request_perform_feat(card_ptr origin_card, player_ptr target) {
        event_card_key key{ origin_card, 6 };
        target->m_game->add_listener<event_type::get_performable_feats>(key, [=](player_ptr origin, card_list &target_cards) {
            if (origin == target) {
                target->m_game->remove_listeners(key);

                if (effect_perform_feat{}.can_play(origin_card, target)) {
                    target_cards.push_back(origin_card);
                }
            }
        });
        target->m_game->add_listener<event_type::on_turn_end>(key, [=](player_ptr origin, bool skipped) {
            if (origin == target) {
                target->m_game->remove_listeners(key);
            }
        });

        target->m_game->queue_request<request_perform_feat>(target);
    }

    prompt_string effect_perform_feat::on_prompt(card_ptr origin_card, player_ptr origin) {
        if (origin_card->deck == card_deck_type::feats && is_legend(origin) && rn::none_of(origin->m_game->range_other_players(origin),
            [&, can_kill = can_damage_legend_kill(origin)](player_ptr target) {
                return is_valid_damage_legend_target(origin, target, can_kill)
                    && (!origin->is_bot() || bot_suggestion::is_target_enemy(origin, target));
            })
        ) {
            return {2, "PROMPT_CLAIM_NO_TARGET"};
        }
        return {};
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
                origin->m_game->queue_action([=]{
                    origin->m_game->remove_listeners(key);
                }, 20);
            }
        });

        if (origin_card->deck == card_deck_type::feats) {
            if (is_legend(origin)) {
                origin->m_game->add_log("LOG_FEAT_CLAIMED", origin, origin_card);
                origin->m_game->queue_request<request_damage_legend>(origin_card, origin);
                origin->m_game->queue_action([=]{
                    origin_card->drop_all_fame();
                    origin_card->m_game->m_first_player->disable_equip(origin_card);
                    origin_card->move_to(pocket_type::feats_discard);
                    draw_next_feat(origin);
                }, 10);
            } else {
                origin->m_game->add_log("LOG_FEAT_PERFORMED", origin, origin_card);
                origin->m_game->queue_action([=]{
                    auto [token, count] = get_player_fame_tokens(origin);
                    origin->m_game->move_tokens(token, token_positions::player{origin}, token_positions::card{origin_card}, 2);
                }, 10);
            }
        }
    }

    void feat_equip::on_disable(card_ptr origin_card, player_ptr origin) {
        origin->m_game->remove_listeners(event_card_key{origin_card, 0});
    }
}