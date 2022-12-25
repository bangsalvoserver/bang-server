#include "steal_destroy.h"

#include "game/game.h"

namespace banggame {

    game_string prompt_target_self::on_prompt(card *origin_card, player *origin, card *target_card) {
        if (origin == target_card->owner) {
            return {"PROMPT_TARGET_SELF", origin_card};
        }
        return {};
    }

    request_targeting::timer_targeting::timer_targeting(request_targeting *request)
        : request_timer(request, request->target->m_game->m_options.escape_timer_ms) {}

    void request_targeting::on_update() {
        if (origin == target_card->owner || target->empty_hand() || !target->can_escape(origin, origin_card, flags)) {
            auto_respond();
        }
    }

    void request_targeting::on_resolve() {
        target->m_game->invoke_action([&]{
            target->m_game->pop_request();
            on_resolve_target();
        });
    }
    
    std::vector<card *> request_targeting::get_highlights() const {
        if (target_card->pocket == pocket_type::player_hand) {
            return target->m_hand;
        } else {
            return {target_card};
        }
    }

    void effect_steal::on_resolve(card *origin_card, player *origin, card *target_card) {
        origin->m_game->call_event<event_type::on_destroy_card>(origin, target_card->owner, target_card);
        origin->m_game->queue_action([=]{
            if (origin->alive() && target_card->owner) {
                if (origin != target_card->owner && target_card->visibility != card_visibility::shown) {
                    origin->m_game->add_log(update_target::includes(origin, target_card->owner), "LOG_STOLEN_CARD", origin, target_card->owner, target_card);
                }
                origin->steal_card(target_card);
            }
        }, 1);
    }

    struct request_steal : request_targeting {
        using request_targeting::request_targeting;

        timer_targeting m_timer{this};
        request_timer *timer() override { return &m_timer; }

        void on_resolve_target() override {
            effect_steal{}.on_resolve(origin_card, origin, target_card);
        }

        game_string status_text(player *owner) const override {
            if (target == owner) {
                if (target_card->visibility != card_visibility::shown) {
                    return {"STATUS_STEAL_FROM_HAND", origin_card};
                } else {
                    return {"STATUS_STEAL", origin_card, target_card};
                }
            } else {
                if (target_card->visibility != card_visibility::shown) {
                    return {"STATUS_STEAL_OTHER_FROM_HAND", target, origin_card};
                } else {
                    return {"STATUS_STEAL_OTHER", target, origin_card, target_card};
                }
            }
        }
    };

    void effect_steal::on_play(card *origin_card, player *origin, card *target_card, effect_flags flags) {
        origin->m_game->queue_action([=]{
            if (origin != target_card->owner) {
                if (target_card->visibility != card_visibility::shown) {
                    origin->m_game->add_log("LOG_PLAYED_CARD_STEAL_HAND", origin_card, origin, target_card->owner);
                } else {
                    origin->m_game->add_log("LOG_PLAYED_CARD_STEAL", origin_card, origin, target_card->owner, target_card);
                }
            } else {
                if (target_card->visibility != card_visibility::shown) {
                    origin->m_game->add_log(update_target::includes(origin), "LOG_PLAYED_CARD_STEAL_OWN", origin_card, origin, target_card);
                    origin->m_game->add_log(update_target::excludes(origin), "LOG_PLAYED_CARD_STEAL_OWN_HAND", origin_card, origin);
                } else {
                    origin->m_game->add_log("LOG_PLAYED_CARD_STEAL_OWN", origin_card, origin, target_card);
                }
            }
            origin->m_game->queue_request<request_steal>(origin_card, origin, target_card->owner, target_card, flags);
        });
    }

    void effect_discard::on_play(card *origin_card, player *origin, card *target_card) {
        if (origin != target_card->owner) {
            origin->m_game->add_log("LOG_DISCARDED_CARD", origin, target_card->owner, target_card);
        } else {
            origin->m_game->add_log("LOG_DISCARDED_SELF_CARD", target_card->owner, target_card);
        }
        target_card->owner->discard_card(target_card);
    }

    void effect_destroy::on_resolve(card *origin_card, player *origin, card *target_card) {
        origin->m_game->call_event<event_type::on_destroy_card>(origin, target_card->owner, target_card);
        origin->m_game->queue_action([=]{
            if (origin->alive() && target_card->owner) {
                if (origin != target_card->owner && target_card->visibility != card_visibility::shown) {
                    origin->m_game->add_log("LOG_DISCARDED_CARD", origin, target_card->owner, target_card);
                }
                target_card->owner->discard_card(target_card);
            }
        }, 1);
    }
    
    struct request_destroy : request_targeting {
        using request_targeting::request_targeting;

        timer_targeting m_timer{this};
        request_timer *timer() override { return &m_timer; }

        void on_resolve_target() override {
            effect_destroy{}.on_resolve(origin_card, origin, target_card);
        }

        game_string status_text(player *owner) const override {
            if (target == owner) {
                if (target_card->visibility != card_visibility::shown) {
                    return {"STATUS_DESTROY_FROM_HAND", origin_card};
                } else {
                    return {"STATUS_DESTROY", origin_card, target_card};
                }
            } else {
                if (target_card->visibility != card_visibility::shown) {
                    return {"STATUS_DESTROY_OTHER_FROM_HAND", target, origin_card};
                } else {
                    return {"STATUS_DESTROY_OTHER", target, origin_card, target_card};
                }
            }
        }
    };
    
    void effect_destroy::on_play(card *origin_card, player *origin, card *target_card, effect_flags flags) {
        origin->m_game->queue_action([=]{
            if (origin != target_card->owner) {
                if (target_card->visibility != card_visibility::shown) {
                    origin->m_game->add_log("LOG_PLAYED_CARD_DESTROY_HAND", origin_card, origin, target_card->owner);
                } else {
                    origin->m_game->add_log("LOG_PLAYED_CARD_DESTROY", origin_card, origin, target_card->owner, target_card);
                }
            } else {
                origin->m_game->add_log("LOG_PLAYED_CARD_DESTROY_OWN", origin_card, origin, target_card);
            }
            origin->m_game->queue_request<request_destroy>(origin_card, origin, target_card->owner, target_card, flags);
        });
    }
}