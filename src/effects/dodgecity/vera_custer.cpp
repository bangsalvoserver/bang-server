#include "vera_custer.h"

#include "game/game.h"
#include "cards/filter_enums.h"

namespace banggame {

    static card *get_card_copy(card *target_card) {
        for (card *c : target_card->m_game->get_all_cards()) {
            if (c != target_card && c->deck == target_card->deck && c->name == target_card->name) {
                return c;
            }
        }
        return target_card->m_game->add_card(*target_card);
    }

    static void copy_characters(player *origin, player *target) {
        auto new_cards = target->m_characters
            | rv::take_last(2)
            | rv::transform(get_card_copy)
            | rn::to_vector;

        if (!rn::equal(origin->m_characters | rv::drop(1), new_cards)) {
            origin->remove_extra_characters();

            for (card *target_card : new_cards) {
                origin->m_game->add_log("LOG_COPY_CHARACTER", origin, target_card);

                target_card->pocket = pocket_type::player_character;
                target_card->owner = origin;
                
                origin->m_characters.emplace_back(target_card);
                origin->enable_equip(target_card);

                origin->m_game->add_update<"add_cards">(std::vector{to_card_backface(target_card)}, pocket_type::player_character, origin);
                target_card->set_visibility(card_visibility::shown, nullptr, true);
            }
        }
    }

    struct request_vera_custer : request_base {
        request_vera_custer(card *origin_card, player *target)
            : request_base(origin_card, nullptr, target, {}, -8) {}

        void on_update() override {
            if (target->alive() && target->m_game->m_playing == target && !target->m_game->is_disabled(origin_card)) {
                if (target->m_game->num_alive() == 2) {
                    target->m_game->pop_request();
                    copy_characters(target, *std::next(player_iterator(target)));
                }
            } else {
                target->m_game->pop_request();
                target->remove_extra_characters();
            }
        }

        game_string status_text(player *owner) const override {
            if (owner == target) {
                return {"STATUS_VERA_CUSTER", origin_card};
            } else {
                return {"STATUS_VERA_CUSTER_OTHER", target, origin_card};
            }
        }
    };

    void equip_vera_custer::on_enable(card *origin_card, player *origin) {
        origin->m_game->add_listener<event_type::pre_turn_start>(origin_card, [=](player *target) {
            if (origin == target) {
                origin->m_game->queue_request<request_vera_custer>(origin_card, target);
            }
        });
    }

    game_string effect_vera_custer::get_error(card *origin_card, player *origin, player *target) {
        if (origin->m_game->top_request<request_vera_custer>(origin) == nullptr) {
            return "ERROR_INVALID_ACTION";
        }
        return {};
    }

    void effect_vera_custer::on_play(card *origin_card, player *origin, player *target) {
        origin->m_game->pop_request();
        copy_characters(origin, target);
    }

}