#include "handcuffs.h"

#include "cards/base/draw.h"
#include "cards/filter_enums.h"

#include "game/game.h"

namespace banggame {
    
    struct request_handcuffs : selection_picker {
        request_handcuffs(card *origin_card, player *target)
            : selection_picker(origin_card, nullptr, target, {}, -8) {}

        void on_update() override {
            if (!live) {
                for (card *c : target->m_game->m_hidden_deck
                    | std::views::filter([](card *c) { return c->has_tag(tag_type::handcuffs); })
                    | ranges::to<std::vector>
                ) {
                    target->m_game->move_card(c, pocket_type::selection, nullptr, card_visibility::shown, true);
                }
            }
        }

        void on_pick(card *target_card) override {
            target->m_game->flash_card(target_card);
            
            auto declared_suit = static_cast<card_suit>(*target_card->get_tag_value(tag_type::handcuffs));
            switch (declared_suit) {
            case card_suit::clubs:
                target->m_game->add_log("LOG_DECLARED_CLUBS", target, origin_card);
                break;
            case card_suit::diamonds:
                target->m_game->add_log("LOG_DECLARED_DIAMONDS", target, origin_card);
                break;
            case card_suit::hearts:
                target->m_game->add_log("LOG_DECLARED_HEARTS", target, origin_card);
                break;
            case card_suit::spades:
                target->m_game->add_log("LOG_DECLARED_SPADES", target, origin_card);
                break;
            }

            target->m_game->add_listener<event_type::check_play_card>({origin_card, 1},
                [origin_card=origin_card, target=target, declared_suit](player *origin, card *c, const effect_context &ctx, game_string &out_error) {
                    if (c->pocket == pocket_type::player_hand && c->owner == target && c->sign.suit != declared_suit) {
                        out_error = {"ERROR_INVALID_SUIT", origin_card, c};
                    }
                });

            target->m_game->pop_request();
            while (!target->m_game->m_selection.empty()) {
                target->m_game->move_card(target->m_game->m_selection.front(), pocket_type::hidden_deck, nullptr, card_visibility::shown, true);
            }
        }

        game_string status_text(player *owner) const override {
            if (target == owner) {
                return {"STATUS_HANDCUFFS", origin_card};
            } else {
                return {"STATUS_HANDCUFFS_OTHER", target, origin_card};
            }
        }
    };

    void equip_handcuffs::on_enable(card *target_card, player *target) {
        target->m_game->add_listener<event_type::on_turn_start>(target_card, [=](player *origin) {
            origin->m_game->queue_request<request_handcuffs>(target_card, origin);
        });
        target->m_game->add_listener<event_type::on_turn_end>(target_card, [=](player *origin, bool skipped) {
            origin->m_game->remove_listeners(event_card_key{target_card, 1});
        });
    }
}