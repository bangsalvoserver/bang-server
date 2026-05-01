#include "ballad.h"

#include "game/game_table.h"

#include "game/prompts.h"
#include "game/bot_suggestion.h"

#include "effects/base/pick.h"
#include "effects/base/resolve.h"
#include "effects/base/steal_destroy.h"
#include "effects/base/escapable.h"

namespace banggame {

    static bool is_valid_ballad_card(card_ptr target_card) {
        return !target_card->is_black() && !target_card->is_train();
    }

    game_string effect_ballad::on_prompt(card_ptr origin_card, player_ptr origin, player_ptr target) {
        if ((origin == target || target->empty_hand()) && rn::none_of(target->m_table, is_valid_ballad_card)) {
            return {"PROMPT_CARD_NO_EFFECT", origin_card};
        }

        MAYBE_RETURN(prompts::prompt_target_self(origin_card, origin, target));

        return {};
    }

    struct request_ballad_steal : request_picking, interface_resolvable {
        using request_picking::request_picking;

        prompt_string pick_prompt(card_ptr target_card) const override {
            return prompts::bot_check_target_enemy(target, origin);
        }

        bool can_pick(card_ptr target_card) const override {
            return target_card->pocket == pocket_type::player_hand
                && target_card->owner == origin;
        }

        void on_pick(card_ptr target_card) override {
            pop_request();

            bot_suggestion::signal_hostile_action(target, origin);
            
            card_ptr stolen_card = origin->random_hand_card();

            destroy_flags flags{};
            target->m_game->call_event(event_type::on_destroy_card{ target, stolen_card, false, flags });
            target->m_game->queue_action([=, origin=origin, target=target]{
                if (target->alive() && origin->alive() && stolen_card->owner == origin) {
                    if (stolen_card->get_visibility() != card_visibility::shown) {
                        target->m_game->add_log(update_target::includes(target, origin), "LOG_STOLEN_CARD", target, origin, stolen_card);
                        target->m_game->add_log(update_target::excludes(target, origin), "LOG_STOLEN_CARD_FROM_HAND", target, origin);
                    } else {
                        target->m_game->add_log("LOG_STOLEN_CARD", target, origin, stolen_card);
                    }
                    target->steal_card(stolen_card);
                }
            }, 42);
        }

        resolve_type get_resolve_type() const override {
            return resolve_type::dismiss;
        }

        void on_resolve() override {
            pop_request();
        }

        game_string status_text(player_ptr owner) const override {
            if (target == owner) {
                return {"STATUS_BALLAD_STEAL", origin_card, origin};
            } else {
                return {"STATUS_BALLAD_STEAL_OTHER", target, origin_card, origin};
            }
        }
    };

    struct request_ballad : request_escapable {
        using request_escapable::request_escapable;
        
        card_list get_highlights(player_ptr owner) const override {
            return target->m_table;
        }

        void on_resolve() override {
            pop_request();

            card_list target_cards = target->m_table | rv::filter(is_valid_ballad_card) | rn::to<std::vector>();
            for (card_ptr target_card : target_cards) {
                target->m_game->add_log("LOG_STOLEN_SELF_CARD", target, target_card);
                target->steal_card(target_card);
            }
            
            if (origin != target && !target->empty_hand()) {
                origin->m_game->queue_request<request_ballad_steal>(origin_card, target, origin);
            }
        }

        game_string status_text(player_ptr owner) const override {
            if (target == owner) {
                return {"STATUS_BALLAD", origin_card, origin};
            } else {
                return {"STATUS_BALLAD_OTHER", target, origin_card, origin};
            }
        }
    };

    void effect_ballad::on_play(card_ptr origin_card, player_ptr origin, player_ptr target) {
        origin->m_game->add_log("LOG_PLAYED_CARD_ON", origin_card, origin, target);
        origin->m_game->queue_request<request_ballad>(origin_card, origin, target);
    }
}