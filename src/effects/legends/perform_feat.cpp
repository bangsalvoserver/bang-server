#include "perform_feat.h"

#include "ruleset.h"

#include "effects/base/can_play_card.h"
#include "effects/base/pick.h"
#include "effects/base/resolve.h"

#include "cards/game_events.h"

#include "game/game_table.h"
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
        if (origin_card->deck == card_deck_type::feats && !is_legend(origin)) {
            auto [token, count] = get_card_fame_token_type(origin->get_character());
            if (count == 0 || origin_card->num_tokens(token) != 0) {
                return false;
            }
        }

        return get_count_performed_feats(origin) == 0;
    }

    struct request_damage_legend : request_resolvable, interface_target_set_players {
        request_damage_legend(card_ptr origin_card, player_ptr target)
            : request_resolvable{origin_card, nullptr, target} {}

        bool can_kill = false;

        void on_update() override {
            if (!live) {
                target->m_game->call_event(event_type::check_damage_legend_kill{ target, can_kill });
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
            return target_player != target && target_player->alive() && is_legend(target_player)
                && (target_player->m_hp > 1 || can_kill);
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
                if (!live) {
                    target->m_game->call_event(event_type::get_performable_feats{ target, target_cards });   
                    if (target_cards.empty() || !effect_perform_feat{}.can_play(target_cards.front(), target)) {
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

        void on_resolve() override {
            target->m_game->pop_request();
        }

        prompt_string resolve_prompt() const override {
            if (target->is_bot()) {
                return "BOT_BAD_PLAY";
            }
            return {};
        }

        bool can_pick(const_card_ptr target_card) const override {
            return rn::contains(target_cards, target_card);
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
                target_cards.push_back(origin_card);
            }
        });
        target->m_game->add_listener<event_type::on_turn_end>(key, [=](player_ptr origin, bool skipped) {
            if (origin == target) {
                target->m_game->remove_listeners(key);
            }
        });

        target->m_game->queue_request<request_perform_feat>(target);
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
            if (is_legend(origin)) {
                origin->m_game->add_log("LOG_FEAT_CLAIMED", origin, origin_card);
                origin->m_game->queue_request<request_damage_legend>(origin_card, origin);
                origin->m_game->queue_action([=]{
                    origin_card->drop_all_fame();
                    origin_card->m_game->m_first_player->disable_equip(origin_card);
                    origin_card->move_to(pocket_type::feats_discard);
                    draw_next_feat(origin);
                });
            } else {
                origin->m_game->add_log("LOG_FEAT_PERFORMED", origin, origin_card);
                origin->m_game->queue_action([=]{
                    card_ptr character_card = origin->get_character();
                    auto [token, count] = get_card_fame_token_type(character_card);
                    character_card->move_tokens(token, origin_card, std::min<int>(count, 2));
                });
            }
        }
    }

    void feat_equip::on_disable(card_ptr origin_card, player_ptr origin) {
        origin->m_game->remove_listeners(event_card_key{origin_card, 0});
    }
}