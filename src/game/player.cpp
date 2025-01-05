#include "player.h"

#include "game.h"

#include "play_verify.h"
#include "game_update.h"

#include "game/filters.h"
#include "cards/filter_enums.h"
#include "cards/game_enums.h"

#include "effects/base/bang.h"
#include "effects/base/damage.h"
#include "effects/base/draw.h"
#include "effects/base/heal.h"
#include "effects/base/predraw_check.h"
#include "effects/base/requests.h"

#include <cassert>
#include <numeric>

namespace banggame {

    bool player::is_bot() const {
        return user_id < 0;
    }

    static bool has_ghost_tag(const_player_ptr origin) {
        return origin->check_player_flags(player_flag::ghost_1)
            || origin->check_player_flags(player_flag::ghost_2)
            || origin->check_player_flags(player_flag::temp_ghost);
    }

    bool player::is_ghost() const {
        return check_player_flags(player_flag::dead) && has_ghost_tag(this);
    }

    bool player::alive() const {
        return !check_player_flags(player_flag::dead) || has_ghost_tag(this);
    }

    void player::equip_card(card_ptr target) {
        target->move_to(pocket_type::player_table, this, card_visibility::shown);
        enable_equip(target);
    }

    void player::enable_equip(card_ptr target_card) {
        bool card_disabled = m_game->is_disabled(target_card);
        for (const equip_holder &holder : target_card->equips) {
            if (!card_disabled || holder.is_nodisable()) {
                holder.on_enable(target_card, this);
            }
        }
    }

    void player::disable_equip(card_ptr target_card) {
        bool card_disabled = m_game->is_disabled(target_card);
        for (const equip_holder &holder : target_card->equips | rv::reverse) {
            if (!card_disabled || holder.is_nodisable()) {
                holder.on_disable(target_card, this);
            }
        }
    }

    void player::play_sound(std::string_view sound_id) {
        m_game->add_update<"play_sound">(update_target::includes_private(this), std::string(sound_id));
    }

    int player::max_cards_end_of_turn() const {
        int ncards = m_hp;
        m_game->call_event(event_type::apply_maxcards_modifier{ this, ncards });
        return ncards;
    }

    int player::get_num_checks() const {
        int nchecks = 1;
        m_game->call_event(event_type::count_num_checks{ this, nchecks });
        return nchecks;
    }

    int player::get_bangs_played() const {
        int nbangs = 0;
        m_game->call_event(event_type::count_bangs_played{ this, nbangs });
        return nbangs;
    }

    int player::get_range_mod() const {
        int mod = 0;
        m_game->call_event(event_type::count_range_mod{ this, range_mod_type::range_mod, mod });
        return mod;
    }

    int player::get_weapon_range() const {
        int range = 1;
        m_game->call_event(event_type::count_range_mod{ this, range_mod_type::weapon_range, range });
        return range;
    }

    int player::get_distance_mod() const {
        int mod = 0;
        m_game->call_event(event_type::count_range_mod{ this, range_mod_type::distance_mod, mod });
        return mod;
    }

    player_ptr player::get_next_player() const {
        auto start = rn::find(m_game->m_players, this);
        auto it = start;
        while (true) {
            ++it;
            if (it == m_game->m_players.end()) {
                it = m_game->m_players.begin();
            }
            if ((*it)->alive()) {
                break;
            }
            if (it == start) {
                throw game_error("Infinite loop in get_next_player");
            }
        }
        return *it;
    }

    player_ptr player::get_prev_player() const {
        auto start = rn::find(m_game->m_players, this);
        auto it = start;
        while (true) {
            if (it == m_game->m_players.begin()) {
                it = m_game->m_players.end();
            }
            --it;
            if ((*it)->alive()) {
                break;
            }
            if (it == start) {
                throw game_error("Infinite loop in get_prev_player");
            }
        }
        return *it;
    }

    card_ptr player::find_equipped_card(const_card_ptr card) const {
        auto it = rn::find(m_table, card->name, &card::name);
        if (it != m_table.end()) {
            return *it;
        } else {
            return nullptr;
        }
    }

    card_ptr player::random_hand_card() const {
        return m_hand[std::uniform_int_distribution(0, int(m_hand.size() - 1))(m_game->rng)];
    }

    static bool move_owned_card(player_ptr owner, card_ptr target_card, bool used) {
        if (target_card->owner == owner) {
            if (target_card->pocket == pocket_type::player_table) {
                target_card->set_inactive(false);
                owner->disable_equip(target_card);
                target_card->drop_all_tokens();
                return true;
            } else if (target_card->pocket == pocket_type::player_hand) {
                owner->m_game->call_event(event_type::on_discard_hand_card{ owner, target_card, used });
                return true;
            }
        }
        return false;
    }

    void player::discard_card(card_ptr target, bool used) {
        if (move_owned_card(this, target, used)) {
            if (target->is_train()) {
                if (m_game->m_train.size() < 4) {
                    target->move_to(pocket_type::train);
                } else {
                    target->move_to(pocket_type::train_deck, nullptr, card_visibility::shown, false, pocket_position::begin);
                    target->set_visibility(card_visibility::hidden, nullptr, true);
                }
            } else if (target->is_black()) {
                target->move_to(pocket_type::shop_deck, nullptr, card_visibility::shown, false, pocket_position::begin);
            } else {
                target->move_to(pocket_type::discard_pile);
                m_game->call_event(event_type::on_discard_any_card{ this, target });
            }
        }
    }

    void player::steal_card(card_ptr target) {
        if (target->owner != this || target->pocket != pocket_type::player_table || !target->is_train()) {
            if (move_owned_card(target->owner, target, false)) {
                add_to_hand(target);
            }
        }
    }

    void player::damage(card_ptr origin_card, player_ptr origin, int value, effect_flags flags) {
        m_game->queue_request<request_damage>(origin_card, origin, this, value, flags);
    }

    void player::heal(int value) {
        if (is_ghost() || m_hp == m_max_hp) return;
        m_game->add_log("LOG_HEALED", this, value);
        set_hp(std::min<int>(m_hp + value, m_max_hp));
        m_game->call_event(event_type::on_heal{ this });
    }

    void player::set_hp(int value, bool instant) {
        if (value != m_hp) {
            m_hp = value;
            m_game->add_update<"player_hp">(this, value, instant ? 0ms : durations.player_hp);
        }
    }

    void player::add_gold(int amount) {
        if (amount) {
            m_gold += amount;
            m_game->add_update<"player_gold">(this, m_gold);
        }
    }

    bool player::immune_to(card_ptr origin_card, player_ptr origin, effect_flags flags, bool quiet) const {
        card_list cards;
        m_game->call_event(event_type::apply_immunity_modifier{ origin_card, origin, this, flags, cards });
        if (!quiet) {
            for (card_ptr target_card : cards) {
                m_game->add_log("LOG_PLAYER_IMMUNE_TO_CARD", this, origin_card, target_card);
                target_card->flash_card();
            }
        }
        return !cards.empty();
    }

    escape_type player::get_escape_type(player_ptr origin, card_ptr origin_card, effect_flags flags) const {
        escape_type result = escape_type::no_escape;
        m_game->call_event(event_type::apply_escapable_modifier{ origin_card, origin, this, flags, result });
        return result;
    }

    void player::add_to_hand(card_ptr target) {
        if (target->deck == card_deck_type::train) {
            equip_card(target);
        } else {
            target->move_to(pocket_type::player_hand, this, m_game->check_flags(game_flag::hands_shown)
                ? card_visibility::shown : card_visibility::show_owner);
        }
        m_game->call_event(event_type::on_drawn_any_card{ target });
    }

    void player::draw_card(int ncards, card_ptr origin_card) {
        if (!m_game->check_flags(game_flag::hands_shown)) {
            if (origin_card) {
                m_game->add_log(update_target::excludes(this), "LOG_DRAWN_CARDS_FOR", this, ncards, origin_card);
            } else {
                m_game->add_log(update_target::excludes(this), "LOG_DRAWN_CARDS", this, ncards);
            }
        }
        for (int i=0; i<ncards; ++i) {
            card_ptr drawn_card = m_game->top_of_deck();
            if (m_game->check_flags(game_flag::hands_shown)) {
                if (origin_card) {
                    m_game->add_log("LOG_DRAWN_CARD_FOR", this, drawn_card, origin_card);
                } else {
                    m_game->add_log("LOG_DRAWN_CARD", this, drawn_card);
                }
            } else {
                if (origin_card) {
                    m_game->add_log(update_target::includes(this), "LOG_DRAWN_CARD_FOR", this, drawn_card, origin_card);
                } else {
                    m_game->add_log(update_target::includes(this), "LOG_DRAWN_CARD", this, drawn_card);
                }
            }
            add_to_hand(drawn_card);
        }
    }

    void player::start_of_turn() {
        m_game->m_playing = this;
        m_played_cards.clear();

        m_game->add_log("LOG_TURN_START", this);
        m_game->add_update<"switch_turn">(this);

        m_game->queue_action([this]{
            m_game->call_event(event_type::pre_turn_start{ this });
            m_game->queue_request<request_predraw>(this);
        }, -30);

        m_game->queue_action([this]{
            if (alive() && m_game->m_playing == this) {
                m_game->call_event(event_type::on_turn_start{ this });
                m_game->queue_request<request_draw>(this);
            }
        }, -30);
    }

    void player::pass_turn() {
        if (m_hand.size() > max_cards_end_of_turn()) {
            m_game->queue_request<request_discard_pass>(this);
        } else {
            m_game->call_event(event_type::on_turn_end{ this, false });
            m_game->queue_action([this]{
                if (m_extra_turns == 0) {
                    remove_player_flags(player_flag::extra_turn);
                    m_game->start_next_turn();
                } else {
                    --m_extra_turns;
                    add_player_flags(player_flag::extra_turn);
                    start_of_turn();
                }
            }, -10);
        }
    }

    void player::skip_turn() {
        remove_player_flags(player_flag::extra_turn);
        m_game->call_event(event_type::on_turn_end{ this, true });
        m_game->start_next_turn();
    }

    void player::remove_extra_characters() {
        if (auto range = m_characters | rv::drop(1)) {
            m_game->add_update<"remove_cards">(range | rn::to_vector);

            for (card_ptr character : range) {
                disable_equip(character);
                character->visibility = card_visibility::hidden;
            }

            m_characters.resize(1);
        }
    }

    void player::reveal_hand() {
        if (!empty_hand() && !m_game->check_flags(game_flag::hands_shown)) {
            for (card_ptr target_card : m_hand) {
                target_card->set_visibility(card_visibility::shown);
                m_game->add_log("LOG_REVEALED_CARD", this, target_card);
            }
            m_game->add_short_pause();
            for (card_ptr target_card : m_hand) {
                target_card->set_visibility(card_visibility::show_owner, this);
            }
        }
    }

    void player::set_role(player_role role, bool instant) {
        m_role = role;

        animation_duration duration = instant ? 0ms : durations.flip_card;

        if (role == player_role::sheriff || m_game->m_players.size() <= 3 || check_player_flags(player_flag::role_revealed)) {
            m_game->add_update<"player_show_role">(this, m_role, duration);
            add_player_flags(player_flag::role_revealed);
        } else {
            m_game->add_update<"player_show_role">(update_target::includes(this), this, m_role, duration);
        }
    }

    void player::reset_max_hp() {
        m_max_hp = first_character()->get_tag_value(tag_type::max_hp).value_or(4) + (m_role == player_role::sheriff);
    }

    bool player::add_player_flags(player_flag flags) {
        if (!check_player_flags(flags)) {
            m_player_flags.add(flags);
            m_game->add_update<"player_flags">(this, m_player_flags);
            return true;
        }
        return false;
    }

    bool player::remove_player_flags(player_flag flags) {
        if (check_player_flags(flags)) {
            m_player_flags.remove(flags);
            m_game->add_update<"player_flags">(this, m_player_flags);
            return true;
        }
        return false;
    }

    bool player::check_player_flags(player_flag flags) const {
        return m_player_flags.check(flags);
    }

    int player::count_cubes() const {
        return rn::accumulate(cube_slots()
            | rv::transform(&card::num_cubes), 0);
    }
}