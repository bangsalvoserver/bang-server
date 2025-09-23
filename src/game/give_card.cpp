#include "give_card.h"

#include "game_table.h"

namespace banggame {

    template<card_deck_type E>
    struct give_card_visitor {
        bool is_valid_card(player_ptr target, card_ptr target_card) const { return false; }
        void give_card(player_ptr target, card_ptr target_card) const {}
    };

    template<> struct give_card_visitor<card_deck_type::main_deck> {
        bool is_valid_card(player_ptr target, card_ptr target_card) const {
            return target_card->pocket != pocket_type::player_hand || target_card->owner != target;
        }
        
        void give_card(player_ptr target, card_ptr target_card) const {
            if (target_card->owner) {
                target->steal_card(target_card);
            } else {
                target->add_to_hand(target_card);
            }
        }
    };

    template<> struct give_card_visitor<card_deck_type::character> {
        bool is_valid_card(player_ptr target, card_ptr target_card) const {
            return target_card != target->get_character();
        }
        
        void give_card(player_ptr target, card_ptr target_card) const {
            if (target_card->pocket == pocket_type::player_character && target_card->owner != target) {
                card_ptr old_character = target->get_character();
                target->remove_cards(target->m_characters);
                target_card->owner->set_character(old_character);
            }
            target->set_character(target_card);
        }
    };

    template<> struct give_card_visitor<card_deck_type::highnoon> {
        bool is_valid_card(player_ptr target, card_ptr target_card) const {
            return target->m_game->m_scenario_cards.empty() || target_card != target->m_game->m_scenario_cards.back();
        }
        
        void give_card(player_ptr target, card_ptr target_card) const {
            if (target_card->pocket == pocket_type::scenario_deck) {
                if (auto it = rn::find(target->m_game->m_scenario_deck, target_card); it != target->m_game->m_scenario_deck.end()) {
                    target->m_game->m_scenario_deck.erase(it);
                } else {
                    target->m_game->add_update(game_updates::add_cards{ target_card, pocket_type::scenario_deck });
                }
                target->m_game->m_scenario_deck.push_back(target_card);
                target_card->set_visibility(card_visibility::shown, nullptr, true);
                target->m_game->add_update(game_updates::move_card{ target_card, nullptr, pocket_type::scenario_deck, 0ms });
            } else {
                target_card->move_to(pocket_type::scenario_deck);
            }
        }
    };

    template<> struct give_card_visitor<card_deck_type::fistfulofcards> : give_card_visitor<card_deck_type::highnoon> {};

    template<> struct give_card_visitor<card_deck_type::wildwestshow> {
        bool is_valid_card(player_ptr target, card_ptr target_card) const {
            return target->m_game->m_wws_scenario_cards.empty() || target_card != target->m_game->m_wws_scenario_cards.back();
        }
        
        void give_card(player_ptr target, card_ptr target_card) const {
            if (target_card->pocket == pocket_type::wws_scenario_deck) {
                if (auto it = rn::find(target->m_game->m_wws_scenario_deck, target_card); it != target->m_game->m_wws_scenario_deck.end()) {
                    target->m_game->m_wws_scenario_deck.erase(it);
                } else {
                    target->m_game->add_update(game_updates::add_cards{ target_card, pocket_type::wws_scenario_deck });
                }
                target->m_game->m_wws_scenario_deck.push_back(target_card);
                target_card->set_visibility(card_visibility::shown, nullptr, true);
                target->m_game->add_update(game_updates::move_card{ target_card, nullptr, pocket_type::wws_scenario_deck, 0ms });
            } else {
                target_card->move_to(pocket_type::wws_scenario_deck);
            }
        }
    };

    template<> struct give_card_visitor<card_deck_type::goldrush> {
        bool is_valid_card(player_ptr target, card_ptr target_card) const {
            return target_card->pocket != pocket_type::shop_selection && target_card->pocket != pocket_type::hidden_deck
                && (target_card->pocket != pocket_type::player_table || target_card->owner != target);
        }
        
        void give_card(player_ptr target, card_ptr target_card) const {
            target->m_game->m_shop_selection.front()->move_to(pocket_type::shop_deck, nullptr, card_visibility::shown, false, pocket_position::begin);
            target_card->move_to(pocket_type::shop_selection);
        }
    };
    
    template<> struct give_card_visitor<card_deck_type::train> {
        bool is_valid_card(player_ptr target, card_ptr target_card) const {
            return target_card->owner != target;
        }
        
        void give_card(player_ptr target, card_ptr target_card) const {
            if (target_card->owner) {
                target->steal_card(target_card);
            } else {
                bool from_train = target_card->pocket == pocket_type::train;
                target->equip_card(target_card);
                if (from_train && !target->m_game->m_train_deck.empty()) {
                    target->m_game->m_train_deck.back()->move_to(pocket_type::train);
                }
            }
        }
    };
    
    template<> struct give_card_visitor<card_deck_type::feats> {
        bool is_valid_card(player_ptr target, card_ptr target_card) const {
            return target_card->pocket != pocket_type::feats;
        }
        
        void give_card(player_ptr target, card_ptr target_card) const {
            if (target->m_game->m_feats.size() >= 4) {
                card_ptr last_feat = target->m_game->m_feats.back();
                target->m_game->m_first_player->disable_equip(last_feat);
                last_feat->drop_all_fame();
                last_feat->move_to(pocket_type::feats_discard);
            }
            target_card->move_to(pocket_type::feats);
            target->m_game->m_first_player->enable_equip(target_card);
        }
    };

    bool give_card(player_ptr target, std::string_view card_name) {
        for (card_ptr target_card : target->m_game->m_cards_storage | rv::values | rv::addressof) {
            if (string_equal_icase(card_name, target_card->name)
                && enums::visit_enum([&]<card_deck_type E>(enums::enum_tag_t<E>) {
                    give_card_visitor<E> visitor;

                    if (visitor.is_valid_card(target, target_card)) {
                        target->m_game->clear_request_status();
                        visitor.give_card(target, target_card);
                        target->m_game->commit_updates();

                        return true;
                    }
                    return false;
                }, target_card->deck)
            ) {
                return true;
            }
        }
        return false;
    }
}