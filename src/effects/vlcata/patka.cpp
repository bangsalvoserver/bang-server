#include "patka.h"

#include "cards/game_events.h"
#include "effects/base/resolve.h"
#include "effects/base/pick.h"

#include "target_types/base/players.h"

#include "game/game_table.h"

namespace banggame {

    static bool affects_all_players(card_ptr c) {
        for (const effect_holder &holder : c->get_effect_list(effect_list_type::effects)) {
            if (holder.target == TARGET_TYPE(players)) {
                return true;
            }
        }
        return false;
    }

    void equip_patka::on_enable(card_ptr target_card, player_ptr target) {
        auto used = std::make_shared<std::array<bool, 2>>();
        used->fill(false);

        target->m_game->add_listener<event_type::on_turn_start>(target_card, [target, used](player_ptr origin) {
            if (origin == target) used->fill(false);
        });

        target->m_game->add_listener<event_type::check_patka_ability>(target_card, [target, used](player_ptr origin, int ability) -> bool {
            if (origin != target) return true;
            return !(*used)[ability];
        });

        target->m_game->add_listener<event_type::mark_patka_used>(target_card, [target, used](player_ptr origin, int ability) {
            if (origin == target) (*used)[ability] = true;
        });
    }

    struct request_patka_discard : request_picking, interface_resolvable {
        request_patka_discard(card_ptr origin_card, player_ptr target, bool mass_avail, bool discard_avail)
            : request_picking(origin_card, nullptr, target)
            , mass_avail(mass_avail), discard_avail(discard_avail)
        {
            // Bots must never be left to freely pile up an open-ended discard-X-for-X-1
            // choice (they always prefer picking another hand card over pressing confirm,
            // so they'd empty their whole hand). Cap a bot's pile at exactly the cards it
            // would otherwise have to discard for being over the hand limit, so the
            // ability only ever converts "unused bad cards" it was going to lose anyway.
            if (target->is_bot()) {
                int diff = int(target->m_hand.size()) - target->max_cards_end_of_turn();
                if (diff > 0) bot_discard_goal = diff;
            }
        }

        bool mass_avail;
        bool discard_avail;
        int discarded = 0;
        int bot_discard_goal = 0;

        bool can_pick(card_ptr target_card) const override {
            if (target_card->owner != target || target_card->pocket != pocket_type::player_hand) {
                return false;
            }
            if (discarded == 0) {
                return (mass_avail && affects_all_players(target_card)) || discard_avail;
            }
            return discard_avail;
        }

        void on_pick(card_ptr target_card) override {
            if (discarded == 0 && mass_avail && affects_all_players(target_card)) {
                target->discard_card(target_card);
                pop_request();
                target->draw_card(2, origin_card);
                target->m_game->call_event(event_type::mark_patka_used{target, 0});
                return;
            }

            target->discard_card(target_card);
            ++discarded;
            if (target->empty_hand() || (bot_discard_goal > 0 && discarded >= bot_discard_goal)) {
                on_resolve();
            }
        }

        void on_resolve() override {
            pop_request();
            if (discarded > 0) {
                if (discarded > 1) {
                    target->draw_card(discarded - 1, origin_card);
                }
                target->m_game->call_event(event_type::mark_patka_used{target, 1});
            }
        }

        game_string status_text(player_ptr owner) const override {
            if (owner == target) {
                return {"STATUS_KIT_CARLSON", origin_card};
            } else {
                return {"STATUS_KIT_CARLSON_OTHER", target, origin_card};
            }
        }
    };

    // Bots only ever consider the mass (discard-on-all-players-card, draw 2) ability on
    // their own; the open-ended discard-X-for-X-1 ability is bot-eligible only once they're
    // already over their end-of-turn hand limit, i.e. right when they'd otherwise be forced
    // to discard those cards for free anyway.
    static bool patka_discard_avail_for(player_ptr origin, bool discard_avail) {
        if (origin->is_bot()) {
            return discard_avail && int(origin->m_hand.size()) > origin->max_cards_end_of_turn();
        }
        return discard_avail;
    }

    bool effect_patka_discard::can_play(card_ptr origin_card, player_ptr origin) {
        bool mass_avail = origin->m_game->call_event(event_type::check_patka_ability{origin, 0});
        bool discard_avail = patka_discard_avail_for(origin, origin->m_game->call_event(event_type::check_patka_ability{origin, 1}));
        return (mass_avail || discard_avail) && !origin->empty_hand();
    }

    void effect_patka_discard::on_play(card_ptr origin_card, player_ptr origin) {
        bool mass_avail = origin->m_game->call_event(event_type::check_patka_ability{origin, 0});
        bool discard_avail = patka_discard_avail_for(origin, origin->m_game->call_event(event_type::check_patka_ability{origin, 1}));
        origin->m_game->queue_request<request_patka_discard>(origin_card, origin, mass_avail, discard_avail);
    }

}
