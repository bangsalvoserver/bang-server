#include "steal_destroy.h"

#include "game/game.h"
#include "game/filters.h"
#include "game/prompts.h"
#include "game/game_options.h"

#include "cards/filter_enums.h"

namespace banggame {

    request_targeting::timer_targeting::timer_targeting(request_targeting *request)
        : request_timer(request, request->target->m_game->m_options.escape_timer) {}

    void request_targeting::on_update() {
        if (target->immune_to(origin_card, origin, flags)) {
            target->m_game->pop_request();
        } else {
            switch (target->can_escape(origin, origin_card, flags)) {
            case 1:
                if (origin != target_card->owner) {
                    break;
                }
                [[fallthrough]];
            case 0:
                auto_resolve();
                break;
            case 2:
                m_timer.reset();
            }
        }
    }

    void request_targeting::on_resolve() {
        target->m_game->pop_request();
        on_resolve_target();
    }
    
    card_list request_targeting::get_highlights() const {
        if (target_card->pocket == pocket_type::player_hand) {
            return target->m_hand;
        } else {
            return {target_card};
        }
    }

    game_string effect_steal::get_error(card_ptr origin_card, player_ptr origin, card_ptr target_card) {
        if (target_card->pocket == pocket_type::player_table && target_card->is_train()) {
            MAYBE_RETURN(check_player_filter(target_card, origin, target_card->equip_target, origin));
        }
        return {};
    }

    game_string effect_steal::on_prompt(card_ptr origin_card, player_ptr origin, card_ptr target_card) {
        MAYBE_RETURN(prompts::bot_check_target_card(origin, target_card));
        if (origin == target_card->owner) {
            if (target_card->is_train() || target_card->pocket == pocket_type::player_hand) {
                return {"PROMPT_CARD_NO_EFFECT", origin_card};
            } else {
                return {"PROMPT_TARGET_SELF", origin_card};
            }
        }
        return {};
    }

    void effect_steal::on_resolve(card_ptr origin_card, player_ptr origin, card_ptr target_card) {
        bool handled = false;
        origin->m_game->call_event(event_type::on_destroy_card{ origin, target_card, handled });
        origin->m_game->queue_action([=]{
            player_ptr target_player = target_card->owner;
            if ((!handled || origin->alive()) && target_player) {
                if (origin != target_player && target_card->visibility != card_visibility::shown) {
                    origin->m_game->add_log(update_target::includes(origin, target_player), "LOG_STOLEN_CARD", origin, target_player, target_card);
                }
                origin->steal_card(target_card);
            }
        }, 42);
    }

    struct request_steal : request_targeting {
        using request_targeting::request_targeting;

        void on_update() override {
            if (!live) {
                if (origin != target) {
                    if (target_card->pocket == pocket_type::player_hand) {
                        origin->m_game->add_log("LOG_PLAYED_CARD_STEAL_HAND", origin_card, origin, target);
                    } else {
                        origin->m_game->add_log("LOG_PLAYED_CARD_STEAL", origin_card, origin, target, target_card);
                    }
                } else {
                    if (target_card->visibility != card_visibility::shown) {
                        origin->m_game->add_log(update_target::includes(origin), "LOG_PLAYED_CARD_STEAL_OWN", origin_card, origin, target_card);
                        origin->m_game->add_log(update_target::excludes(origin), "LOG_PLAYED_CARD_STEAL_OWN_HAND", origin_card, origin);
                    } else {
                        origin->m_game->add_log("LOG_PLAYED_CARD_STEAL_OWN", origin_card, origin, target_card);
                    }
                }
            }
            request_targeting::on_update();
        }

        void on_resolve_target() override {
            effect_steal{}.on_resolve(origin_card, origin, target_card);
        }

        game_string status_text(player_ptr owner) const override {
            if (target == owner) {
                if (target_card->pocket == pocket_type::player_hand) {
                    return {"STATUS_STEAL_FROM_HAND", origin_card};
                } else {
                    return {"STATUS_STEAL", origin_card, target_card};
                }
            } else {
                if (target_card->pocket == pocket_type::player_hand) {
                    return {"STATUS_STEAL_OTHER_FROM_HAND", target, origin_card};
                } else {
                    return {"STATUS_STEAL_OTHER", target, origin_card, target_card};
                }
            }
        }
    };

    void effect_steal::on_play(card_ptr origin_card, player_ptr origin, card_ptr target_card, effect_flags flags) {
        origin->m_game->queue_request<request_steal>(origin_card, origin, target_card->owner, target_card, flags);
    }

    game_string effect_discard::get_error(card_ptr origin_card, player_ptr origin, card_ptr target_card) {
        if (card_ptr disabler = origin->m_game->get_usage_disabler(target_card)) {
            return {"ERROR_CARD_DISABLED_BY", target_card, disabler};
        }
        return {};
    }

    game_string effect_discard::on_prompt(card_ptr origin_card, player_ptr origin, card_ptr target_card) {
        if (target_card->pocket == pocket_type::player_table
            && target_card->owner == origin
            && target_card->has_tag(tag_type::ghost_card))
        {
            return "PROMPT_TARGET_SELF_GHOST_CARD";
        }
        return {};
    }

    void effect_discard::on_play(card_ptr origin_card, player_ptr origin) {
        origin->discard_used_card(origin_card);
    }

    void effect_discard::on_play(card_ptr origin_card, player_ptr origin, card_ptr target_card) {
        player_ptr target_player = target_card->owner;
        if (origin != target_player) {
            origin->m_game->add_log("LOG_DISCARDED_CARD", origin, target_player, target_card);
        } else {
            origin->m_game->add_log("LOG_DISCARDED_SELF_CARD", target_player, target_card);
        }
        target_player->discard_card(target_card, used);
    }

    game_string effect_destroy::on_prompt(card_ptr origin_card, player_ptr origin, card_ptr target_card) {
        MAYBE_RETURN(prompts::bot_check_target_card(origin, target_card));
        MAYBE_RETURN(prompts::prompt_target_self(origin_card, origin, target_card->owner));
        return {};
    }

    void effect_destroy::on_resolve(card_ptr origin_card, player_ptr origin, card_ptr target_card) {
        bool handled = false;
        origin->m_game->call_event(event_type::on_destroy_card{ origin, target_card, handled });
        origin->m_game->queue_action([=]{
            player_ptr target_player = target_card->owner;
            if ((!handled || origin->alive()) && target_player) {
                if (origin != target_player && target_card->visibility != card_visibility::shown) {
                    origin->m_game->add_log("LOG_DISCARDED_CARD", origin, target_player, target_card);
                }
                target_player->discard_card(target_card);
            }
        }, 42);
    }
    
    struct request_destroy : request_targeting {
        using request_targeting::request_targeting;

        void on_update() override {
            if (!live) {
                if (origin != target) {
                    if (target_card->pocket == pocket_type::player_hand) {
                        origin->m_game->add_log("LOG_PLAYED_CARD_DESTROY_HAND", origin_card, origin, target);
                    } else {
                        origin->m_game->add_log("LOG_PLAYED_CARD_DESTROY", origin_card, origin, target, target_card);
                    }
                } else {
                    origin->m_game->add_log("LOG_PLAYED_CARD_DESTROY_OWN", origin_card, origin, target_card);
                }
            }
            request_targeting::on_update();
        }

        void on_resolve_target() override {
            effect_destroy{}.on_resolve(origin_card, origin, target_card);
        }

        game_string status_text(player_ptr owner) const override {
            if (target == owner) {
                if (target_card->pocket == pocket_type::player_hand) {
                    return {"STATUS_DESTROY_FROM_HAND", origin_card};
                } else {
                    return {"STATUS_DESTROY", origin_card, target_card};
                }
            } else {
                if (target_card->pocket == pocket_type::player_hand) {
                    return {"STATUS_DESTROY_OTHER_FROM_HAND", target, origin_card};
                } else {
                    return {"STATUS_DESTROY_OTHER", target, origin_card, target_card};
                }
            }
        }
    };
    
    void effect_destroy::on_play(card_ptr origin_card, player_ptr origin, card_ptr target_card, effect_flags flags) {
        origin->m_game->queue_request<request_destroy>(origin_card, origin, target_card->owner, target_card, flags);
    }
}