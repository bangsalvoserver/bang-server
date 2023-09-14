#include "game_table.h"

#include "cards/filter_enums.h"
#include "cards/game_enums.h"

#include "player_iterator.h"

namespace banggame {

    game_table::game_table(unsigned int seed)
        : disabler_map(this)
    {
        if (seed == 0) {
            std::random_device rd;
            rng_seed = rd();
        } else {
            rng_seed = seed;
        }
        rng.seed(rng_seed);
    }

    player *game_table::find_player_by_userid(int user_id) const {
        if (auto it = std::ranges::find(m_players, user_id, &player::user_id); it != m_players.end()) {
            return *it;
        }
        return nullptr;
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
        case pocket_type::wws_scenario_deck: return m_wws_scenario_deck;
        case pocket_type::wws_scenario_card: return m_wws_scenario_cards;
        case pocket_type::button_row:        return m_button_row;
        case pocket_type::stations:          return m_stations;
        case pocket_type::train:             return m_train;
        case pocket_type::train_deck:        return m_train_deck;
        default: throw std::runtime_error("Invalid pocket");
        }
    }

    int game_table::calc_distance(const player *from, const player *to) {
        if (from == to) return 0;

        int distance_mod = to->get_distance_mod();

        if (check_flags(game_flags::disable_player_distances)) {
            return 1 + distance_mod;
        }

        player_iterator from_it{from};
        player_iterator to_it{to};

        if (to->alive()) {
            return std::min(
                std::distance(from_it, to_it),
                std::distance(std::reverse_iterator(from_it), std::reverse_iterator(to_it))
            ) + distance_mod;
        } else {
            return std::min(
                std::distance(from_it, std::prev(to_it)),
                std::distance(std::reverse_iterator(from_it), std::reverse_iterator(std::next(to_it)))
            ) + 1 + distance_mod;
        }
    }

    int game_table::num_alive() const {
        return int(std::ranges::count_if(m_players, &player::alive));
    }

    void game_table::shuffle_cards_and_ids(std::span<card *> vec) {
        for (size_t i = vec.size() - 1; i > 0; --i) {
            size_t i2 = std::uniform_int_distribution<size_t>{0, i}(rng);
            if (i == i2) continue;

            std::swap(vec[i], vec[i2]);
            auto a = m_context.cards.extract(vec[i]->id);
            auto b = m_context.cards.extract(vec[i2]->id);
            std::swap(a->id, b->id);
            m_context.cards.insert(std::move(a));
            m_context.cards.insert(std::move(b));
        }
    }

    void game_table::set_card_visibility(card *c, player *owner, card_visibility visibility, bool instant) {
        if (visibility == card_visibility::hidden) {
            if (c->visibility == card_visibility::show_owner) {
                add_update<game_update_type::hide_card>(update_target::includes(c->owner), c, instant);
            } else if (c->visibility == card_visibility::shown) {
                add_update<game_update_type::hide_card>(c, instant);
            }
            c->visibility = card_visibility::hidden;
        } else if (!owner || visibility == card_visibility::shown) {
            if (c->visibility == card_visibility::show_owner) {
                add_update<game_update_type::show_card>(update_target::excludes(c->owner), c, *c, instant);
            } else if (c->visibility == card_visibility::hidden) {
                add_update<game_update_type::show_card>(c, *c, instant);
            }
            c->visibility = card_visibility::shown;
        } else if (c->owner != owner || c->visibility != card_visibility::show_owner) {
            if (c->visibility == card_visibility::shown) {
                add_update<game_update_type::hide_card>(update_target::excludes(owner), c, instant);
            } else {
                if (c->visibility == card_visibility::show_owner) {
                    add_update<game_update_type::hide_card>(update_target::includes(c->owner), c, instant);
                }
                add_update<game_update_type::show_card>(update_target::includes(owner), c, *c, instant);
            }
            c->visibility = card_visibility::show_owner;
        }
    }

    void game_table::move_card(card *c, pocket_type pocket, player *owner, card_visibility visibility, bool instant) {
        if (c->pocket == pocket && c->owner == owner) return;
        
        set_card_visibility(c, owner, visibility, instant);

        auto &prev_pile = get_pocket(c->pocket, c->owner);
        prev_pile.erase(std::ranges::find(prev_pile, c));

        get_pocket(pocket, owner).emplace_back(c);
        
        c->pocket = pocket;
        c->owner = owner;
        
        add_update<game_update_type::move_card>(c, owner, pocket, instant);
    }

    card *game_table::top_of_deck() {
        if (m_deck.empty()) {
            if (m_discards.empty()) {
                throw std::runtime_error("Deck is empty. Cannot shuffle");
            }
            m_deck = std::move(m_discards);
            m_discards.clear();
            for (card *c : m_deck) {
                c->pocket = pocket_type::main_deck;
                c->owner = nullptr;
                c->visibility = card_visibility::hidden;
            }
            shuffle_cards_and_ids(m_deck);
            add_log("LOG_DECK_RESHUFFLED");
            play_sound(nullptr, "shuffle");
            add_update<game_update_type::deck_shuffled>(pocket_type::main_deck);
        }
        return m_deck.back();
    }

    card *game_table::phase_one_drawn_card() {
        if (!check_flags(game_flags::phase_one_draw_discard) || m_discards.empty()) {
            return top_of_deck();
        } else {
            return m_discards.back();
        }
    }

    card *game_table::draw_shop_card() {
        if (m_shop_deck.empty()) {
            if (m_shop_discards.empty()) {
                throw std::runtime_error("Shop deck is empty. Cannot reshuffle");
            }
            m_shop_deck = std::move(m_shop_discards);
            m_shop_discards.clear();
            for (card *c : m_shop_deck) {
                c->pocket = pocket_type::shop_deck;
                c->owner = nullptr;
                c->visibility = card_visibility::hidden;
            }
            shuffle_cards_and_ids(m_shop_deck);
            add_log("LOG_SHOP_RESHUFFLED");
            play_sound(nullptr, "shuffle");
            add_update<game_update_type::deck_shuffled>(pocket_type::shop_deck);
        }
        card *drawn_card = m_shop_deck.back();
        add_log("LOG_DRAWN_SHOP_CARD", drawn_card);
        move_card(drawn_card, pocket_type::shop_selection);
        return drawn_card;
    }

    void game_table::flash_card(card *c) {
        add_update<game_update_type::flash_card>(c);
    }

    void game_table::add_short_pause(card *c) {
        add_update<game_update_type::short_pause>(c);
    }

    void game_table::tap_card(card *c, bool inactive) {
        if (inactive != c->inactive) {
            add_update<game_update_type::tap_card>(c, inactive);
            c->inactive = inactive;
        }
    }

    void game_table::play_sound(player *target, const std::string &file_id) {
        if (target) {
            add_update<game_update_type::play_sound>(update_target::includes_private(target), file_id);
        } else {
            add_update<game_update_type::play_sound>(file_id);
        }
    }
    
    void game_table::add_cubes(card *target, int ncubes) {
        ncubes = std::min<int>({ncubes, num_cubes, max_cubes - target->num_cubes});
        if (ncubes > 0) {
            num_cubes -= ncubes;
            target->num_cubes += ncubes;
            if (ncubes == 1) {
                add_log("LOG_ADD_CUBE", target->owner, target);
            } else {
                add_log("LOG_ADD_CUBES", target->owner, target, ncubes);
            }
            add_update<game_update_type::move_cubes>(ncubes, nullptr, target);
        }
    }

    void game_table::move_cubes(card *origin, card *target, int ncubes) {
        ncubes = std::min<int>(ncubes, origin->num_cubes);
        if (target && ncubes > 0 && target->num_cubes < max_cubes) {
            int added_cubes = std::min<int>(ncubes, max_cubes - target->num_cubes);
            target->num_cubes += added_cubes;
            origin->num_cubes -= added_cubes;
            ncubes -= added_cubes;
            if (origin->owner == target->owner) {
                if (added_cubes == 1) {
                    add_log("LOG_MOVED_CUBE", target->owner, origin, target);
                } else {
                    add_log("LOG_MOVED_CUBES", target->owner, origin, target, added_cubes);
                }
            } else {
                if (added_cubes == 1) {
                    add_log("LOG_MOVED_CUBE_FROM", target->owner, origin->owner, origin, target);
                } else {
                    add_log("LOG_MOVED_CUBES_FROM", target->owner, origin->owner, origin, target, added_cubes);
                }
            }
            add_update<game_update_type::move_cubes>(added_cubes, origin, target);
        }
        if (ncubes > 0) {
            origin->num_cubes -= ncubes;
            num_cubes += ncubes;
            if (ncubes == 1) {
                add_log("LOG_PAID_CUBE", origin->owner, origin);
            } else {
                add_log("LOG_PAID_CUBES", origin->owner, origin, ncubes);
            }
            add_update<game_update_type::move_cubes>(ncubes, origin, nullptr);
        }
        if (origin->sign && origin->num_cubes == 0) {
            add_log("LOG_DISCARDED_ORANGE_CARD", origin->owner, origin);
            call_event<event_type::on_discard_orange_card>(origin->owner, origin);
            origin->owner->disable_equip(origin);
            move_card(origin, pocket_type::discard_pile);
        }
    }

    void game_table::drop_cubes(card *target) {
        if (target->num_cubes > 0) {
            if (target->num_cubes == 1) {
                add_log("LOG_DROP_CUBE", target->owner, target);
            } else {
                add_log("LOG_DROP_CUBES", target->owner, target, target->num_cubes);
            }
            num_cubes += target->num_cubes;
            add_update<game_update_type::move_cubes>(target->num_cubes, target, nullptr);
            target->num_cubes = 0;
        }
    }

    void game_table::add_game_flags(game_flags flags) {
        m_game_flags |= flags;
        add_update<game_update_type::game_flags>(m_game_flags);
    }

    void game_table::remove_game_flags(game_flags flags) {
        m_game_flags &= ~flags;
        add_update<game_update_type::game_flags>(m_game_flags);
    }

    bool game_table::check_flags(game_flags flags) const {
        return bool(m_game_flags & flags);
    }

    bool game_table::is_game_over() const {
        return check_flags(game_flags::game_over);
    }

}