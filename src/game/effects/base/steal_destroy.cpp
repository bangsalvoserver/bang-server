#include "steal_destroy.h"

#include "../../game.h"

namespace banggame {

    game_string prompt_target_self_hand::on_prompt(card *origin_card, player *origin, card *target_card) {
        if (origin == target_card->owner && target_card->pocket == pocket_type::player_hand) {
            return {"PROMPT_TARGET_OWN_HAND", origin_card};
        }
        return {};
    }
    
    std::vector<card *> request_targeting::get_highlights() const {
        if (target_card->pocket == pocket_type::player_hand) {
            return target->m_hand;
        } else {
            return {target_card};
        }
    }

    static void finalize_steal(card *origin_card, player *origin, card *target_card) {
        if (origin->alive() && target_card->owner) {
            auto priv_target = update_target::includes(origin, target_card->owner);
            auto inv_target = update_target::excludes(origin, target_card->owner);
            if (origin != target_card->owner) {
                origin->m_game->add_log(priv_target, "LOG_STOLEN_CARD", origin, target_card->owner, target_card);
                if (target_card->pocket == pocket_type::player_hand) {
                    origin->m_game->add_log(inv_target, "LOG_STOLEN_CARD_FROM_HAND", origin, target_card->owner);
                } else {
                    origin->m_game->add_log(inv_target, "LOG_STOLEN_CARD", origin, target_card->owner, target_card);
                }
            } else {
                origin->m_game->add_log(priv_target, "LOG_STOLEN_SELF_CARD", origin, target_card);
                if (target_card->pocket == pocket_type::player_hand) {
                    origin->m_game->add_log(inv_target, "LOG_STOLEN_SELF_CARD_FROM_HAND", origin);
                } else {
                    origin->m_game->add_log(inv_target, "LOG_STOLEN_SELF_CARD", origin, target_card);
                }
            }
            origin->steal_card(target_card);
        }
    }

    void effect_steal::on_resolve(card *origin_card, player *origin, card *target_card) {
        if (origin->m_game->num_queued_requests([&]{
            origin->m_game->call_event<event_type::on_discard_card>(origin, target_card->owner, target_card);
        })) {
            origin->m_game->queue_action_front([=]{ finalize_steal(origin_card, origin, target_card); });
        } else {
            finalize_steal(origin_card, origin, target_card);
        }
    }

    struct request_steal : request_targeting, resolvable_request {
        using request_targeting::request_targeting;

        void on_resolve() override {
            origin->m_game->pop_request();
            effect_steal{}.on_resolve(origin_card, origin, target_card);
            origin->m_game->update_request();
        }

        game_string status_text(player *owner) const override {
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

    void effect_steal::on_play(card *origin_card, player *origin, card *target_card, effect_flags flags) {
        if (origin != target_card->owner && target_card->owner->can_escape(origin, origin_card, flags)) {
            origin->m_game->queue_request<request_steal>(origin_card, origin, target_card->owner, target_card, flags);
        } else {
            on_resolve(origin_card, origin, target_card);
        }
    }

    static void finalize_discard(card *origin_card, player *origin, card *target_card) {
        if (origin->alive() && target_card->owner) {
            if (origin != target_card->owner) {
                origin->m_game->add_log("LOG_DISCARDED_CARD", origin, target_card->owner, target_card);
            } else {
                origin->m_game->add_log("LOG_DISCARDED_SELF_CARD", target_card->owner, target_card);
            }
            target_card->owner->discard_card(target_card);
        }
    }

    void effect_discard::on_resolve(card *origin_card, player *origin, card *target_card) {
        if (origin->m_game->num_queued_requests([&]{
            origin->m_game->call_event<event_type::on_discard_card>(origin, target_card->owner, target_card);
        })) {
            origin->m_game->queue_action_front([=]{ finalize_discard(origin_card, origin, target_card); });
        } else {
            finalize_discard(origin_card, origin, target_card);
        }
    }
    
    struct request_destroy : request_targeting, resolvable_request {
        using request_targeting::request_targeting;

        void on_resolve() override {
            origin->m_game->pop_request();
            effect_destroy{}.on_resolve(origin_card, origin, target_card);
            origin->m_game->update_request();
        }

        game_string status_text(player *owner) const override {
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
    
    void effect_discard::on_play(card *origin_card, player *origin, card *target_card, effect_flags flags) {
        if (origin != target_card->owner && target_card->owner->can_escape(origin, origin_card, flags)) {
            origin->m_game->queue_request<request_destroy>(origin_card, origin, target_card->owner, target_card, flags);
        } else {
            on_resolve(origin_card, origin, target_card);
        }
    }
}