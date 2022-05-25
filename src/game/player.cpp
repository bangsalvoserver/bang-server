#include "player.h"

#include "game.h"

#include "holders.h"
#include "play_verify.h"
#include "game_update.h"

#include "effects/base/requests.h"
#include "effects/armedanddangerous/requests.h"
#include "effects/valleyofshadows/requests.h"

#include <cassert>

template<typename ... Ts> struct overloaded : Ts ... { using Ts::operator() ...; };
template<typename ... Ts> overloaded(Ts ...) -> overloaded<Ts ...>;

namespace banggame {
    using namespace enums::flag_operators;

    void player::equip_card(card *target) {
        m_game->move_card(target, pocket_type::player_table, this, show_card_flags::shown);
        enable_equip(target);
        target->usages = 0;
    }

    void player::enable_equip(card *target_card) {
        if (!m_game->is_disabled(target_card)) {
            target_card->on_enable(this);
        }
    }

    void player::disable_equip(card *target_card) {
        if (!m_game->is_disabled(target_card)) {
            target_card->on_disable(this);
        }
    }

    int player::get_initial_cards() {
        int value = m_max_hp;
        m_game->call_event<event_type::apply_initial_cards_modifier>(this, value);
        return value;
    }

    int player::max_cards_end_of_turn() {
        int n = m_hp;
        m_game->call_event<event_type::apply_maxcards_modifier>(this, n);
        return n;
    }

    card *player::find_equipped_card(card *card) {
        auto it = std::ranges::find(m_table, card->name, &card::name);
        if (it != m_table.end()) {
            return *it;
        } else {
            return nullptr;
        }
    }

    card *player::random_hand_card() {
        return m_hand[std::uniform_int_distribution<int>(0, m_hand.size() - 1)(m_game->rng)];
    }

    static void move_owned_card(player *owner, card *target_card, pocket_type pocket, player *target = nullptr, show_card_flags flags = {}) {
        if (target_card->owner == owner) {
            if (target_card->pocket == pocket_type::player_table) {
                if (target_card->inactive) {
                    target_card->inactive = false;
                    owner->m_game->add_update<game_update_type::tap_card>(target_card->id, false);
                }
                owner->disable_equip(target_card);
                owner->drop_all_cubes(target_card);
                owner->m_game->move_card(target_card, pocket, target, flags);
                target_card->on_unequip(owner);
            } else if (target_card->pocket == pocket_type::player_hand) {
                owner->m_game->move_card(target_card, pocket, target, flags);
            }
        }
    }

    void player::discard_card(card *target) {
        move_owned_card(this, target, target->color == card_color_type::black
            ? pocket_type::shop_discard
            : pocket_type::discard_pile);
    }

    void player::steal_card(card *target) {
        move_owned_card(target->owner, target, pocket_type::player_hand, this);
    }

    void player::damage(card *origin_card, player *origin, int value, bool is_bang, bool instant) {
        if (is_ghost()) return;
        
        if (instant || !m_game->has_expansion(card_expansion_type::valleyofshadows | card_expansion_type::canyondiablo)) {
            m_game->add_log(value == 1 ? "LOG_TAKEN_DAMAGE" : "LOG_TAKEN_DAMAGE_PLURAL", origin_card, this, value);
            set_hp(m_hp - value);
            if (m_hp <= 0) {
                m_game->queue_request_front<request_death>(origin_card, origin, this);
            }
            if (m_game->has_expansion(card_expansion_type::goldrush)) {
                if (origin && origin->m_game->m_playing == origin && origin != this && origin->alive()) {
                    origin->add_gold(value);
                }
            }
            m_game->call_event<event_type::on_hit>(origin_card, origin, this, value, is_bang);
        } else {
            m_game->queue_request_front<timer_damaging>(origin_card, origin, this, value, is_bang);
        }
    }

    void player::heal(int value) {
        if (is_ghost() || m_hp == m_max_hp) return;
        
        if (value == 1) {
            m_game->add_log("LOG_HEALED", this);
        } else {
            m_game->add_log("LOG_HEALED_PLURAL", this, value);
        }
        set_hp(std::min<int>(m_hp + value, m_max_hp));
    }

    void player::set_hp(int value, bool instant) {
        m_hp = value;
        m_game->add_update<game_update_type::player_hp>(id, value, instant);
    }

    void player::add_gold(int amount) {
        if (amount) {
            m_gold += amount;
            m_game->add_update<game_update_type::player_gold>(id, m_gold);
        }
    }

    bool player::immune_to(card *c) {
        bool value = false;
        m_game->call_event<event_type::apply_immunity_modifier>(c, this, value);
        return value;
    }

    bool player::can_respond_with(card *c) {
        return !m_game->is_disabled(c) && !c->responses.empty()
            && std::ranges::all_of(c->responses, [&](const effect_holder &e) {
                return e.can_respond(c, this);
            });
    }

    void player::queue_request_add_cube(card *origin_card, int ncubes) {
        int nslots = max_cubes - m_characters.front()->num_cubes;
        int ncards = nslots > 0;
        for (card *c : m_table) {
            if (c->color == card_color_type::orange) {
                ncards += c->num_cubes < max_cubes;
                nslots += max_cubes - c->num_cubes;
            }
        }
        ncubes = std::min<int>(ncubes, m_game->num_cubes);
        if (nslots <= ncubes || ncards <= 1) {
            auto do_add_cubes = [&](card *c) {
                int cubes_to_add = std::min<int>(ncubes, max_cubes - c->num_cubes);
                ncubes -= cubes_to_add;
                add_cubes(c, cubes_to_add);
            };
            do_add_cubes(m_characters.front());
            for (card *c : m_table) {
                if (c->color == card_color_type::orange) {
                    do_add_cubes(c);
                }
            }
        } else {
            m_game->queue_request<request_add_cube>(origin_card, this, ncubes);
        }
    }

    bool player::can_escape(player *origin, card *origin_card, effect_flags flags) const {
        if (bool(flags & effect_flags::escapable)
            && m_game->has_expansion(card_expansion_type::valleyofshadows)) return true;
        
        bool value = false;
        m_game->call_event<event_type::apply_escapable_modifier>(origin_card, origin, this, flags, value);
        return value;
    }
    
    void player::add_cubes(card *target, int ncubes) {
        ncubes = std::min<int>({ncubes, m_game->num_cubes, max_cubes - target->num_cubes});
        if (ncubes > 0) {
            m_game->num_cubes -= ncubes;
            target->num_cubes += ncubes;
            m_game->add_update<game_update_type::move_cubes>(ncubes, 0, target->id);
        }
    }

    void player::pay_cubes(card *origin, int ncubes) {
        move_cubes(origin, nullptr, ncubes);
    }

    void player::move_cubes(card *origin, card *target, int ncubes) {
        ncubes = std::min<int>(ncubes, origin->num_cubes);
        if (target && ncubes > 0 && target->num_cubes < max_cubes) {
            int added_cubes = std::min<int>(ncubes, max_cubes - target->num_cubes);
            target->num_cubes += added_cubes;
            origin->num_cubes -= added_cubes;
            ncubes -= added_cubes;
            m_game->add_update<game_update_type::move_cubes>(added_cubes, origin->id, target->id);
        }
        if (ncubes > 0) {
            origin->num_cubes -= ncubes;
            m_game->num_cubes += ncubes;
            m_game->add_update<game_update_type::move_cubes>(ncubes, origin->id, 0);
        }
        if (origin->sign && origin->num_cubes == 0) {
            m_game->add_log("LOG_DISCARDED_ORANGE_CARD", this, origin);
            disable_equip(origin);
            m_game->move_card(origin, pocket_type::discard_pile);
            m_game->call_event<event_type::on_discard_orange_card>(this, origin);
            origin->on_unequip(this);
        }
    }

    void player::drop_all_cubes(card *target) {
        if (target->num_cubes > 0) {
            m_game->num_cubes += target->num_cubes;
            m_game->add_update<game_update_type::move_cubes>(target->num_cubes, target->id, 0);
            target->num_cubes = 0;
        }
    }

    void player::add_to_hand(card *target) {
        m_game->move_card(target, pocket_type::player_hand, this);
    }

    void player::draw_card(int ncards, card *origin_card) {
        if (ncards == 1) {
            if (origin_card) {
                m_game->add_log(update_target::excludes(this), "LOG_DRAWN_A_CARD_FOR", this, origin_card);
            } else {
                m_game->add_log(update_target::excludes(this), "LOG_DRAWN_A_CARD", this);
            }
        } else {
            if (origin_card) {
                m_game->add_log(update_target::excludes(this), "LOG_DRAWN_N_CARDS_FOR", this, ncards, origin_card);
            } else {
                m_game->add_log(update_target::excludes(this), "LOG_DRAWN_N_CARDS", this, ncards);
            }
        }
        for (int i=0; i<ncards; ++i) {
            card *drawn_card = m_game->m_deck.back();
            if (origin_card) {
                m_game->add_log(update_target::includes(this), "LOG_DRAWN_CARD_FOR", this, drawn_card, origin_card);
            } else {
                m_game->add_log(update_target::includes(this), "LOG_DRAWN_CARD", this, drawn_card);
            }
            m_game->draw_card_to(pocket_type::player_hand, this);
        }
    }

    void player::set_last_played_card(card *c) {
        m_last_played_card = c;
        m_game->add_update<game_update_type::last_played_card>(update_target::includes(this), c ? c->id : 0);
        remove_player_flags(player_flags::start_of_turn);
    }

    void player::set_forced_card(card *c) {
        m_forced_card = c;
        m_game->add_update<game_update_type::force_play_card>(update_target::includes(this), c ? c->id : 0);
    }

    void player::set_mandatory_card(card *c) {
        m_mandatory_card = c;
    }

    bool player::is_bangcard(card *card_ptr) {
        return (check_player_flags(player_flags::treat_missed_as_bang)
                && card_ptr->has_tag(tag_type::missedcard))
            || card_ptr->has_tag(tag_type::bangcard);
    };

    void player::handle_action(enums::enum_tag_t<game_action_type::pick_card>, const pick_card_args &args) {
        if (m_prompt) {
            throw game_error("ERROR_INVALID_ACTION");
        }
        
        if (m_game->m_requests.empty()) {
            throw game_error("ERROR_INVALID_ACTION");
        }

        auto &req = m_game->top_request();
        if (req.target() != this) {
            throw game_error("ERROR_INVALID_ACTION");
        }

        m_game->add_update<game_update_type::confirm_play>(update_target::includes(this));
        player *target_player = args.player_id ? m_game->find_player(args.player_id) : nullptr;
        card *target_card = args.card_id ? m_game->find_card(args.card_id) : nullptr;
        if (req.can_pick(args.pocket, target_player, target_card)) {
            req.on_pick(args.pocket, target_player, target_card);
        }
    }

    void player::play_card_action(card *card_ptr) {
        switch (card_ptr->pocket) {
        case pocket_type::player_hand:
            m_game->move_card(card_ptr, pocket_type::discard_pile);
            m_game->call_event<event_type::on_play_hand_card>(this, card_ptr);
            break;
        case pocket_type::player_table:
            if (card_ptr->color == card_color_type::green) {
                m_game->move_card(card_ptr, pocket_type::discard_pile);
            }
            break;
        case pocket_type::shop_selection:
            if (card_ptr->color == card_color_type::brown) {
                m_game->move_card(card_ptr, pocket_type::shop_discard);
            }
            break;
        default:
            break;
        }
    }

    void player::log_played_card(card *card_ptr, bool is_response) {
        switch (card_ptr->pocket) {
        case pocket_type::player_hand:
        case pocket_type::scenario_card:
            m_game->add_log(is_response ? "LOG_RESPONDED_WITH_CARD" : "LOG_PLAYED_CARD", card_ptr, this);
            break;
        case pocket_type::player_table:
            m_game->add_log(is_response ? "LOG_RESPONDED_WITH_CARD" : "LOG_PLAYED_TABLE_CARD", card_ptr, this);
            break;
        case pocket_type::player_character:
            m_game->add_log(is_response ?
                card_ptr->has_tag(tag_type::drawing)
                    ? "LOG_DRAWN_WITH_CHARACTER"
                    : "LOG_RESPONDED_WITH_CHARACTER"
                : "LOG_PLAYED_CHARACTER", card_ptr, this);
            break;
        case pocket_type::shop_selection:
            m_game->add_log("LOG_BOUGHT_CARD", card_ptr, this);
            break;
        }
    }

    static std::vector<card *> find_cards(game *game, const std::vector<int> &args) {
        std::vector<card *> ret;
        for (int id : args) {
            ret.push_back(game->find_card(id));
        }
        return ret;
    }

    static target_list parse_target_id_vector(game *game, const std::vector<play_card_target_ids> &args) {
        target_list ret;
        for (const auto &t : args) {
            ret.push_back(enums::visit_indexed(overloaded{
                [](enums::enum_tag_for<target_type> auto tag) {
                    return play_card_target(tag);
                },
                [game](enums::enum_tag_t<target_type::player> tag, int player_id) {
                    return play_card_target(tag, game->find_player(player_id));
                },
                [game](enums::enum_tag_t<target_type::conditional_player> tag, int player_id) {
                    if (player_id) {
                        return play_card_target(tag, game->find_player(player_id));
                    } else {
                        return play_card_target(tag);
                    }
                },
                [game](enums::enum_tag_t<target_type::card> tag, int card_id) {
                    return play_card_target(tag, game->find_card(card_id));
                },
                [game](enums::enum_tag_t<target_type::extra_card> tag, int card_id) {
                    if (card_id) {
                        return play_card_target(tag, game->find_card(card_id));
                    } else {
                        return play_card_target(tag);
                    }
                },
                [game](enums::enum_tag_t<target_type::cards_other_players> tag, const std::vector<int> &card_ids) {
                    return play_card_target(tag, find_cards(game, card_ids));
                },
                [game](enums::enum_tag_t<target_type::cube> tag, const std::vector<int> &card_ids) {
                    return play_card_target(tag, find_cards(game, card_ids));
                }
            }, t));
        }
        return ret;
    }

    void player::handle_action(enums::enum_tag_t<game_action_type::play_card>, const play_card_args &args) {
        if (m_prompt) {
            throw game_error("ERROR_INVALID_ACTION");
        }

        if (!m_game->m_requests.empty() || m_game->m_playing != this) {
            throw game_error("ERROR_INVALID_ACTION");
        }

        if (auto error = play_card_verify{
            this,
            m_game->find_card(args.card_id),
            false,
            parse_target_id_vector(m_game, args.targets),
            find_cards(m_game, args.modifier_ids)
        }.verify_and_play()) {
            throw std::move(*error);
        }
    }
    
    void player::handle_action(enums::enum_tag_t<game_action_type::respond_card>, const play_card_args &args) {
        if (m_prompt) {
            throw game_error("ERROR_INVALID_ACTION");
        }
        
        play_card_verify verifier{
            this,
            m_game->find_card(args.card_id),
            true,
            parse_target_id_vector(m_game, args.targets),
            find_cards(m_game, args.modifier_ids)
        };

        if (!can_respond_with(verifier.card_ptr)
            || (verifier.card_ptr->pocket == pocket_type::player_hand
            && verifier.card_ptr->color != card_color_type::brown)
            || (!verifier.modifiers.empty())
        ) {
            throw game_error("ERROR_INVALID_ACTION");
        }
        
        if (auto error = verifier.verify_card_targets()) {
            throw std::move(*error);
        }
        prompt_then(verifier.check_prompt(), [=, this]{
            verifier.do_play_card();
            set_last_played_card(nullptr);
        });
    }

    void player::prompt_then(opt_fmt_str &&message, std::function<void()> &&fun) {
        if (message) {
            m_prompt.emplace(std::move(fun), *message);
            m_game->add_update<game_update_type::game_prompt>(update_target::includes(this), std::move(*message));
        } else {
            m_game->add_update<game_update_type::confirm_play>(update_target::includes(this));
            std::invoke(fun);
        }
    }

    void player::handle_action(enums::enum_tag_t<game_action_type::prompt_respond>, bool response) {
        if (!m_prompt) {
            throw game_error("ERROR_INVALID_ACTION");
        }

        m_game->add_update<game_update_type::confirm_play>(update_target::includes(this));
        if (response) {
            std::invoke(m_prompt->first);
        }
        m_prompt.reset();
    }

    void player::draw_from_deck() {
        int save_numcards = m_num_cards_to_draw;
        m_game->call_event<event_type::on_draw_from_deck>(this);
        if (m_game->pop_request<request_draw>()) {
            m_game->add_log("LOG_DRAWN_FROM_DECK", this);
            while (m_num_drawn_cards<m_num_cards_to_draw) {
                ++m_num_drawn_cards;
                card *drawn_card = m_game->phase_one_drawn_card();
                m_game->add_log(update_target::excludes(this), "LOG_DRAWN_A_CARD", this);
                m_game->add_log(update_target::includes(this), "LOG_DRAWN_CARD", this, drawn_card);
                m_game->move_card(drawn_card, pocket_type::player_hand, this);
                m_game->call_event<event_type::on_card_drawn>(this, drawn_card);
            }
        }
        m_num_cards_to_draw = save_numcards;
        m_game->queue_action([this]{
            m_game->call_event<event_type::post_draw_cards>(this);
        });
    }

    card_sign player::get_card_sign(card *target_card) {
        card_sign sign = target_card->sign;
        m_game->call_event<event_type::apply_sign_modifier>(this, sign);
        return sign;
    }

    void player::start_of_turn() {
        if (this != m_game->m_playing && this == m_game->m_first_player) {
            m_game->draw_scenario_card();
        }
        
        m_game->m_playing = this;

        m_mandatory_card = nullptr;
        m_bangs_played = 0;
        m_bangs_per_turn = 1;
        m_num_drawn_cards = 0;
        add_player_flags(player_flags::start_of_turn);
        
        for (card *c : m_characters) {
            c->usages = 0;
        }
        for (card *c : m_table) {
            c->usages = 0;
        }
        for (auto &[card_id, obj] : m_predraw_checks) {
            obj.resolved = false;
        }
        
        m_game->add_update<game_update_type::switch_turn>(id);
        m_game->add_log("LOG_TURN_START", this);
        m_game->call_event<event_type::pre_turn_start>(this);

        m_game->queue_action([this]{
            next_predraw_check(nullptr);
        });
    }

    void player::next_predraw_check(card *target_card) {
        if (auto it = m_predraw_checks.find(target_card); it != m_predraw_checks.end()) {
            it->second.resolved = true;
        }
        if (alive() && m_game->m_playing == this && !m_game->m_game_over) {
            if (std::ranges::all_of(m_predraw_checks | std::views::values, &predraw_check::resolved)) {
                request_drawing();
            } else {
                using predraw_check_pair = decltype(player::m_predraw_checks)::value_type;
                auto unresolved = m_predraw_checks
                    | std::views::filter([](const predraw_check_pair &pair) {
                        return !pair.second.resolved;
                    });
                auto top_priority = unresolved
                    | std::views::filter([value = std::ranges::max(unresolved
                        | std::views::values
                        | std::views::transform(&player::predraw_check::priority))]
                    (const predraw_check_pair &pair) {
                        return pair.second.priority == value;
                    });
                if (std::ranges::distance(top_priority) == 1) {
                    card *predraw_card = top_priority.begin()->first;
                    m_game->call_event<event_type::on_predraw_check>(this, predraw_card);
                    m_game->queue_action([this, predraw_card]{
                        next_predraw_check(predraw_card);
                    });
                } else {
                    m_game->queue_request<request_predraw>(this);
                }
            }
        }
    }

    void player::request_drawing() {
        m_game->call_event<event_type::on_turn_start>(this);
        m_game->queue_action([this]{
            m_game->call_event<event_type::on_request_draw>(this);
            if (m_game->m_requests.empty()) {
                m_game->queue_request<request_draw>(this);
            }
        });
    }

    void player::pass_turn() {
        m_mandatory_card = nullptr;
        if (m_hand.size() > max_cards_end_of_turn()) {
            m_game->queue_request<request_discard_pass>(this);
        } else {
            untap_inactive_cards();

            m_game->call_event<event_type::on_turn_end>(this);
            if (!check_player_flags(player_flags::extra_turn)) {
                m_game->call_event<event_type::post_turn_end>(this);
            }
            m_game->queue_action([&]{
                if (m_extra_turns == 0) {
                    remove_player_flags(player_flags::extra_turn);
                    if (remove_player_flags(player_flags::temp_ghost)) {
                        --m_num_cards_to_draw;
                        if (!check_player_flags(player_flags::ghost)) {
                            m_game->player_death(nullptr, this);
                        }
                    }
                    m_game->start_next_turn();
                } else {
                    --m_extra_turns;
                    add_player_flags(player_flags::extra_turn);
                    start_of_turn();
                }
            });
        }
    }

    void player::skip_turn() {
        untap_inactive_cards();
        remove_player_flags(player_flags::extra_turn);
        m_game->call_event<event_type::on_turn_end>(this);
        m_game->start_next_turn();
    }

    void player::untap_inactive_cards() {
        for (card *c : m_table) {
            if (c->inactive) {
                c->inactive = false;
                m_game->add_update<game_update_type::tap_card>(c->id, false);
            }
        }
    }

    void player::discard_all() {
        while (!m_table.empty()) {
            m_game->add_log("LOG_DISCARDED_SELF_CARD", this, m_table.front());
            discard_card(m_table.front());
        }
        drop_all_cubes(m_characters.front());
        while (!m_hand.empty()) {
            m_game->add_log("LOG_DISCARDED_SELF_CARD", this, m_hand.front());
            m_game->move_card(m_hand.front(), pocket_type::discard_pile);
        }
    }

    void player::set_role(player_role role) {
        m_role = role;

        if (role == player_role::sheriff || m_game->m_players.size() <= 3) {
            m_game->add_update<game_update_type::player_show_role>(id, m_role, true);
            add_player_flags(player_flags::role_revealed);
        } else {
            m_game->add_update<game_update_type::player_show_role>(update_target::includes(this), id, m_role, true);
        }
    }

    void player::reset_max_hp() {
        m_max_hp = m_characters.front()->get_tag_value(tag_type::max_hp).value_or(4) + (m_role == player_role::sheriff);
    }

    void player::send_player_status() {
        m_game->add_update<game_update_type::player_status>(id, m_player_flags, m_range_mod, m_weapon_range, m_distance_mod);
    }

    bool player::add_player_flags(player_flags flags) {
        if (!check_player_flags(flags)) {
            m_player_flags |= flags;
            send_player_status();
            return true;
        }
        return false;
    }

    bool player::remove_player_flags(player_flags flags) {
        if (check_player_flags(flags)) {
            m_player_flags &= ~flags;
            send_player_status();
            return true;
        }
        return false;
    }

    bool player::check_player_flags(player_flags flags) const {
        return (m_player_flags & flags) == flags;
    }

    bool player::has_character_tag(tag_type tag) const {
        return std::ranges::any_of(m_characters, [=](const card *c) {
            return c->has_tag(tag);
        });
    }

    int player::count_cubes() const {
        return m_characters.front()->num_cubes
            + std::transform_reduce(
                m_table.begin(),
                m_table.end(),
                0, std::plus(),
                std::mem_fn(&card::num_cubes));
    }
}