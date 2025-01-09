#include "ruleset.h"

#include "perform_feat.h"

#include "effects/base/pick.h"
#include "effects/base/resolve.h"

#include "game/game_table.h"

namespace banggame {

    card_ptr draw_next_feat(player_ptr origin) {
        auto &feats_deck = origin->m_game->m_feats_deck;

        if (feats_deck.empty()) {
            auto &feats_discard = origin->m_game->m_feats_discard;
            if (feats_discard.empty()) {
                throw game_error("Feats deck is empty. Cannot shuffle");
            }
            feats_deck = std::move(feats_discard);
            feats_discard.clear();
            for (card_ptr c : feats_deck) {
                c->pocket = pocket_type::feats_deck;
                c->owner = nullptr;
                c->visibility = card_visibility::hidden;
            }
            origin->m_game->shuffle_cards_and_ids(feats_deck);
            origin->m_game->add_log("LOG_FEATS_RESHUFFLED");
            origin->m_game->play_sound("shuffle");
            origin->m_game->add_update<"deck_shuffled">(pocket_type::feats_deck);
        }
        
        card_ptr feat_card = feats_deck.back();
        origin->m_game->add_log("LOG_DRAWN_FEAT", feat_card);
        feat_card->set_visibility(card_visibility::shown);
        if (origin->m_game->m_feats.size() < 4) {
            feat_card->move_to(pocket_type::feats);
        }
        origin->m_game->m_first_player->enable_equip(feat_card);
        return feat_card;
    }

    static int count_feat_tot_fame(const_card_ptr target_card) {
        int result = 0;
        for (const auto &[token, count] : target_card->tokens) {
            if (token != card_token_type::cube) {
                result += count;
            }
        }
        return result;
    }

    struct request_discard_feat : request_picking {
        request_discard_feat(player_ptr target)
            : request_picking{nullptr, nullptr, target} {}
        
        int max_fame = 0;

        void on_update() override {
            max_fame = rn::max(target->m_game->m_feats | rv::transform(count_feat_tot_fame));
            auto_pick();
        }
        
        bool can_pick(const_card_ptr target_card) const override {
            if (target_card->pocket == pocket_type::feats
                || target_card->pocket == pocket_type::feats_deck && target_card->visibility == card_visibility::shown
            ) {
                return count_feat_tot_fame(target_card) == max_fame;
            }
            return false;
        }

        void on_pick(card_ptr target_card) override {
            target->m_game->pop_request();
            target->m_game->add_log("LOG_DISCARDED_FEAT", target, target_card);
            target_card->drop_all_fame();
            target->m_game->m_first_player->disable_equip(target_card);
            target_card->move_to(pocket_type::feats_discard);
            target->m_game->m_feats_deck.back()->move_to(pocket_type::feats);
        }

        game_string status_text(player_ptr owner) const override {
            if (owner == target) {
                return "STATUS_DISCARD_FEAT";
            } else {
                return {"STATUS_DISCARD_FEAT_OTHER", target};
            }
        }
    };

    struct request_boast_add_fame : request_picking {
        request_boast_add_fame(player_ptr target)
            : request_picking{nullptr, nullptr, target} {}

        void on_update() override {
            auto_pick();
        }

        bool can_pick(const_card_ptr target_card) const override {
            if (target_card->pocket == pocket_type::feats) {
                auto [token, count] = get_card_fame_token_type(target->first_character());
                return target_card->tokens[token] == 0;
            }
            return false;
        }

        void on_pick(card_ptr target_card) override {
            target->m_game->pop_request();
            target->m_game->add_log("LOG_FEAT_BOASTED", target, target_card);
            card_ptr character_card = target->first_character();
            auto [token, count] = get_card_fame_token_type(character_card);
            character_card->move_tokens(token, target_card, 1);
        }

        game_string status_text(player_ptr owner) const override {
            if (owner == target) {
                return "STATUS_BOAST_ADD_FAME";
            } else {
                return {"STATUS_BOAST_ADD_FAME_OTHER", target};
            }
        }
    };

    struct request_boast_feat : request_picking, interface_resolvable {
        request_boast_feat(player_ptr target)
            : request_picking{nullptr, nullptr, target} {}

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
            if (target->m_game->m_feats_deck.empty()) {
                return target_card->pocket == pocket_type::feats_discard;
            } else {
                return target_card->pocket == pocket_type::feats_deck;
            }
        }

        void on_pick(card_ptr target_card) override {
            target->m_game->pop_request();
            card_ptr feat_card = draw_next_feat(target);
            if (feat_card->pocket == pocket_type::feats_deck) {
                target->m_game->queue_request<request_discard_feat>(target);
            }
            target->m_game->queue_request<request_boast_add_fame>(target);
        }

        game_string status_text(player_ptr owner) const override {
            if (owner == target) {
                return "STATUS_BOAST_FEAT";
            } else {
                return {"STATUS_BOAST_FEAT_OTHER", target};
            }
        }
    };

    void ruleset_legends::on_apply(game_ptr game) {
        game->add_listener<event_type::on_game_setup>({nullptr, 0}, [](player_ptr origin) {
            card_token_type tokens[] = {
                card_token_type::fame1,
                card_token_type::fame2,
                card_token_type::fame3,
                card_token_type::fame4,
                card_token_type::fame5,
                card_token_type::fame6,
                card_token_type::fame7,
                card_token_type::fame8,
            };

            rn::shuffle(tokens, origin->m_game->rng);

            for (auto [token_type, target] : rv::zip(tokens, origin->m_game->range_all_players(origin))) {
                origin->m_game->add_tokens(token_type, 5, target->first_character());
            }
            
            draw_next_feat(origin);
        });

        game->add_listener<event_type::count_initial_cards>({nullptr, -1}, [](const_player_ptr origin, int &value) {
            int count = 0;
            for (player_ptr p : origin->m_game->range_all_players(origin->m_game->m_first_player)) {
                ++count;
                if (p == origin) break;
            }
            if (count >= 6) {
                value = 5;
            } else {
                value = 4;
            }
        });

        game->add_listener<event_type::apply_maxcards_modifier>({nullptr, 20}, [](const_player_ptr origin, int &value) {
            if (origin->m_hp <= 1) {
                ++value;
            }
        });

        game->add_listener<event_type::on_turn_end>({nullptr, 20}, [](player_ptr origin, bool skipped) {
            card_ptr character_card = origin->first_character();
            if (character_card->deck != card_deck_type::legends && get_count_performed_feats(origin) == 0) {
                auto [token, count] = get_card_fame_token_type(character_card);
                if (count != 0) {
                    origin->m_game->queue_request<request_boast_feat>(origin);
                }
            }
        });
    }
    
}