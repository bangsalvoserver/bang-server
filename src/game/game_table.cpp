#include "game_table.h"

#include "formatter.h"

#include "player_iterator.h"

namespace banggame {

    using namespace enums::flag_operators;

    card *game_table::find_card(int card_id) {
        if (auto it = m_cards.find(card_id); it != m_cards.end()) {
            return &*it;
        }
        throw std::runtime_error("server.find_card: ID not found");
    }

    player *game_table::find_player(int player_id) {
        if (auto it = m_players.find(player_id); it != m_players.end()) {
            return &*it;
        }
        throw std::runtime_error("server.find_player: ID not found");
    }
    
    std::vector<card *> &game_table::get_pocket(pocket_type pocket, player *owner) {
        switch (pocket) {
        case pocket_type::player_hand:       return owner->m_hand;
        case pocket_type::player_table:      return owner->m_table;
        case pocket_type::player_character:  return owner->m_characters;
        case pocket_type::player_backup:     return owner->m_backup_character;
        case pocket_type::main_deck:         return m_deck;
        case pocket_type::discard_pile:      return m_discards;
        case pocket_type::selection:         return m_selection;
        case pocket_type::shop_deck:         return m_shop_deck;
        case pocket_type::shop_selection:    return m_shop_selection;
        case pocket_type::shop_discard:      return m_shop_discards;
        case pocket_type::hidden_deck:       return m_hidden_deck;
        case pocket_type::scenario_deck:     return m_scenario_deck;
        case pocket_type::scenario_card:     return m_scenario_cards;
        case pocket_type::specials:          return m_specials;
        default: throw std::runtime_error("Invalid pocket");
        }
    }

    int game_table::calc_distance(player *from, player *to) {
        if (from == to) return 0;
        if (check_flags(game_flags::disable_player_distances)) return to->m_distance_mod;
        if (to->alive()) {
            return std::min(
                std::distance(player_iterator(from), player_iterator(to)),
                std::distance(player_iterator(to), player_iterator(from))) + to->m_distance_mod;
        } else {
            return std::min(
                std::distance(player_iterator(from), std::prev(player_iterator(to))),
                std::distance(std::next(player_iterator(to)), player_iterator(from))) + 1 + to->m_distance_mod;
        }
    }

    int game_table::num_alive() const {
        return static_cast<int>(std::ranges::count_if(m_players, &player::alive));
    }

    void game_table::shuffle_cards_and_ids(std::span<card *> vec) {
        for (size_t i = vec.size() - 1; i > 0; --i) {
            size_t i2 = std::uniform_int_distribution<size_t>{0, i}(rng);
            if (i == i2) continue;

            std::swap(vec[i], vec[i2]);
            auto a = m_cards.extract(vec[i]->id);
            auto b = m_cards.extract(vec[i2]->id);
            std::swap(a->id, b->id);
            m_cards.insert(std::move(a));
            m_cards.insert(std::move(b));
        }
    }

    void game_table::send_card_update(card *c, player *owner, show_card_flags flags) {
        if (bool(flags & show_card_flags::hidden)) {
            add_update<game_update_type::hide_card>(c->id, flags);
        } else if (!owner || bool(flags & show_card_flags::shown)) {
            add_update<game_update_type::show_card>(*c, flags);
        } else {
            add_update<game_update_type::hide_card>(update_target::excludes(owner), c->id, flags);
            add_update<game_update_type::show_card>(update_target::includes(owner), *c, flags);
        }
    }

    void game_table::move_card(card *c, pocket_type pocket, player *owner, show_card_flags flags) {
        pocket_type prev_pocket = c->pocket;
        if (c->pocket != pocket || c->owner != owner) {
            send_card_update(c, owner, flags);

            auto &prev_pile = get_pocket(c->pocket, c->owner);
            prev_pile.erase(std::ranges::find(prev_pile, c));

            get_pocket(pocket, owner).emplace_back(c);
            
            c->pocket = pocket;
            c->owner = owner;
            
            add_update<game_update_type::move_card>(c->id, owner ? owner->id : 0, pocket, flags);
        }
        if (prev_pocket == pocket_type::main_deck && m_deck.empty()) {
            card *top_discards = m_discards.back();
            if (m_options.keep_last_card_shuffling) {
                m_discards.pop_back();
            }
            m_deck = std::move(m_discards);
            for (card *c : m_deck) {
                c->pocket = pocket_type::main_deck;
                c->owner = nullptr;
            }
            m_discards.clear();
            if (m_options.keep_last_card_shuffling) {
                m_discards.emplace_back(top_discards);
            }
            shuffle_cards_and_ids(m_deck);
            add_log("LOG_DECK_RESHUFFLED");
            add_update<game_update_type::deck_shuffled>(pocket_type::main_deck);
        }
    }

    card *game_table::draw_card_to(pocket_type pocket, player *owner, show_card_flags flags) {
        card *drawn_card = m_deck.back();
        move_card(drawn_card, pocket, owner, flags);
        return drawn_card;
    }

    card *game_table::phase_one_drawn_card() {
        if (!check_flags(game_flags::phase_one_draw_discard) || m_discards.empty()) {
            return m_deck.back();
        } else {
            return m_discards.back();
        }
    }

    card *game_table::draw_phase_one_card_to(pocket_type pocket, player *owner, show_card_flags flags) {
        card *drawn_card = phase_one_drawn_card();
        move_card(drawn_card, pocket, owner, flags);
        return drawn_card;
    }

    card *game_table::draw_shop_card() {
        card *drawn_card = m_shop_deck.back();
        add_log("LOG_DRAWN_SHOP_CARD", drawn_card);
        move_card(drawn_card, pocket_type::shop_selection);
        if (drawn_card->modifier == card_modifier_type::shopchoice) {
            for (card *c : m_hidden_deck) {
                if (c->get_tag_value(tag_type::shopchoice) == drawn_card->get_tag_value(tag_type::shopchoice)) {
                    send_card_update(c, nullptr, show_card_flags::instant);
                }
            }
        }
        if (m_shop_deck.empty()) {
            m_shop_deck = std::move(m_shop_discards);
            for (card *c : m_shop_deck) {
                c->pocket = pocket_type::shop_deck;
                c->owner = nullptr;
            }
            m_shop_discards.clear();
            shuffle_cards_and_ids(m_shop_deck);
            add_update<game_update_type::deck_shuffled>(pocket_type::shop_deck);
        }
        return drawn_card;
    }

    void game_table::draw_scenario_card() {
        if (m_scenario_deck.empty()) return;

        if (m_scenario_deck.size() > 1) {
            send_card_update(*(m_scenario_deck.rbegin() + 1), nullptr, show_card_flags::instant);
        }
        if (!m_scenario_cards.empty()) {
            m_scenario_cards.back()->on_disable(m_first_player);
            set_game_flags({});
        }
        add_log("LOG_DRAWN_SCENARIO_CARD", m_scenario_deck.back());
        move_card(m_scenario_deck.back(), pocket_type::scenario_card);
        m_scenario_cards.back()->on_enable(m_first_player);
    }

    void game_table::flash_card(card *c) {
        add_update<game_update_type::flash_card>(c->id);
    }

    void game_table::add_disabler(event_card_key key, card_disabler_fun &&fun) {
        const auto disable_if_new = [&](card *c) {
            if (!is_disabled(c) && fun(c)) {
                c->on_disable(c->owner);
            }
        };

        for (auto &p : m_players) {
            std::ranges::for_each(p.m_table, disable_if_new);
            std::ranges::for_each(p.m_characters, disable_if_new);
        }

        m_disablers.emplace(std::make_pair(key, std::move(fun)));
    }

    void game_table::remove_disablers(event_card_key key) {
        const auto enable_if_old = [&](card *c) {
            bool a = false;
            bool b = false;
            for (const auto &[t, fun] : m_disablers) {
                if (t != key) a = a || fun(c);
                else b = b || fun(c);
            }
            if (!a && b) {
                c->on_enable(c->owner);
            }
        };

        for (auto &p : m_players) {
            std::ranges::for_each(p.m_table, enable_if_old);
            std::ranges::for_each(p.m_characters, enable_if_old);
        }

        m_disablers.erase(key);
    }

    card *game_table::get_disabler(card *target_card) {
        for (auto &[card_key, fun] : m_disablers) {
            if (fun(target_card)) return card_key.target_card;
        }
        return nullptr;
    }

    bool game_table::is_disabled(card *target_card) {
        return get_disabler(target_card) != nullptr;
    }

    void game_table::set_game_flags(game_flags type) {
        m_game_flags = type;
        add_update<game_update_type::game_flags>(type);
    }

}