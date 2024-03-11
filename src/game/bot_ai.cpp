#include "game.h"
#include "play_verify.h"
#include "possible_to_play.h"
#include "net/bot_info.h"

#include "cards/effect_enums.h"
#include "cards/filters.h"

namespace banggame {

    struct bot_error : std::exception {
        game_string message;
        bot_error(game_string message): message{message} {}
    };

    template<rn::range Range, typename Rng>
    decltype(auto) random_element(Range &&range, Rng &rng) {
        rn::range_value_t<Range> ret;
        if (rn::sample(std::forward<Range>(range), &ret, 1, rng).out == &ret) {
            throw bot_error{"EMPTY_RANGE_IN_RANDOM_ELEMENT"};
        }
        return ret;
    }

    struct random_target_visitor {
        player *origin;
        card *origin_card;
        const effect_holder &holder;
        const effect_context &ctx;

        player *operator()(enums::enum_tag_t<target_type::player>) const {
            return random_element(make_player_target_set(origin, origin_card, holder, ctx), origin->m_game->rng);
        }

        player *operator()(enums::enum_tag_t<target_type::conditional_player>) const {
            auto targets = make_player_target_set(origin, origin_card, holder, ctx) | rn::to_vector;
            if (targets.empty()) {
                return nullptr;
            } else {
                return random_element(targets, origin->m_game->rng);
            }
        }

        serial::player_list operator()(enums::enum_tag_t<target_type::adjacent_players>) const {
            auto targets = make_adjacent_players_target_set(origin, origin_card, ctx) | rn::to_vector;
            auto [target1, target2] = random_element(targets, origin->m_game->rng);
            return {target1, target2};
        }

        card *operator()(enums::enum_tag_t<target_type::card>) const {
            auto targets = make_card_target_set(origin, origin_card, holder, ctx) | rn::to_vector;
            return random_element(targets, origin->m_game->rng);
        }

        card *operator()(enums::enum_tag_t<target_type::extra_card> tag) const {
            if (ctx.repeat_card) {
                return nullptr;
            } else {
                auto targets = make_card_target_set(origin, origin_card, holder, ctx) | rn::to_vector;
                return random_element(targets, origin->m_game->rng);
            }
        }

        auto operator()(enums::enum_tag_t<target_type::cards> tag) const {
            auto targets = make_card_target_set(origin, origin_card, holder, ctx) | rn::to_vector;
            return targets
                | rv::sample(holder.target_value, origin->m_game->rng)
                | rn::to<serial::card_list>;
        }

        auto operator()(enums::enum_tag_t<target_type::max_cards> tag) const {
            auto targets = make_card_target_set(origin, origin_card, holder, ctx) | rn::to_vector;
            size_t count = holder.target_value;
            if (count == 0) {
                count = std::uniform_int_distribution<size_t>{1, targets.size()}(origin->m_game->rng);
            }
            return targets
                | rv::sample(count, origin->m_game->rng)
                | rn::to<serial::card_list>;
        }

        auto operator()(enums::enum_tag_t<target_type::cards_other_players>) const {
            serial::card_list ret;
            for (player *target : range_other_players(origin)) {
                if (auto targets = rv::concat(
                    target->m_table | rv::remove_if(&card::is_black),
                    target->m_hand | rv::take(1)
                )) {
                    ret.push_back(random_element(targets, origin->m_game->rng));
                }
            }
            return ret;
        }

        auto operator()(enums::enum_tag_t<target_type::select_cubes>) const {
            auto cubes = origin->cube_slots()
                | rv::for_each([](card *slot) {
                    return rv::repeat_n(slot, slot->num_cubes);
                })
                | rn::to_vector;
            return cubes
                | rv::sample(holder.target_value, origin->m_game->rng)
                | rn::to<serial::card_list>;
        }
    };

    static play_card_target generate_random_target(player *origin, card *origin_card, const effect_holder &holder, const effect_context &ctx) {
        return enums::visit_enum([&]<target_type E>(enums::enum_tag_t<E> tag) -> play_card_target {
            if constexpr (play_card_target::has_type<E>) {
                return {tag, random_target_visitor{origin, origin_card, holder, ctx}(tag)};
            } else {
                return tag;
            }
        }, holder.target);
    }

    static game_action generate_random_play(player *origin, const card_modifier_node &node, bool is_response) {
        game_action ret;
        effect_context ctx;

        const card_modifier_node *cur_node = &node;
        card *playing_card = nullptr;
        while (!playing_card) {
            if (cur_node->branches.empty()) {
                playing_card = cur_node->card;
                if (filters::is_equip_card(playing_card)) {
                    if (!playing_card->self_equippable()) {
                        ret.targets.emplace_back(enums::enum_tag<target_type::player>,
                            random_element(make_equip_set(origin, playing_card, ctx), origin->m_game->rng));
                    }
                } else {
                    for (const effect_holder &holder : playing_card->get_effect_list(is_response)) {
                        ret.targets.push_back(generate_random_target(origin, playing_card, holder, ctx));
                    }
                    if (is_possible_to_play_effects(origin, playing_card, playing_card->optionals, ctx)) {
                        for (const effect_holder &holder : playing_card->optionals) {
                            ret.targets.push_back(generate_random_target(origin, playing_card, holder, ctx));
                        }
                    }
                }
            } else {
                card *origin_card = cur_node->card;
                auto &targets = ret.modifiers.emplace_back(origin_card).targets;

                origin_card->modifier.add_context(origin_card, origin, ctx);
                for (const effect_holder &holder : origin_card->get_effect_list(is_response)) {
                    const auto &target = targets.emplace_back(generate_random_target(origin, origin_card, holder, ctx));
                    if (holder.type == effect_type::ctx_add) {
                        if (target.is(target_type::card)) {
                            origin_card->modifier.add_context(origin_card, origin, target.get<target_type::card>(), ctx);
                        } else if (target.is(target_type::player)) {
                            origin_card->modifier.add_context(origin_card, origin, target.get<target_type::player>(), ctx);
                        }
                    }
                }

                cur_node = random_element(cur_node->branches
                    | rv::transform([](const card_modifier_node &node) { return &node; }),
                    origin->m_game->rng);
            }
        }
        ret.card = playing_card;
        return ret;
    }

    struct compare_card_node {
        bool operator ()(const card_modifier_node &lhs, const card_modifier_node &rhs) const {
            return lhs.card == rhs.card
                ? rn::lexicographical_compare(lhs.branches, rhs.branches, compare_card_node{})
                : get_card_order(lhs.card) < get_card_order(rhs.card);
        }
    };

    struct play_card_node {
        const card_modifier_node *node;
        
        auto operator < (const play_card_node &other) const {
            return compare_card_node{}(*node, *other.node);
        }
    };

    static bool execute_random_play(player *origin, bool is_response, std::optional<timer_id_t> timer_id, const card_modifier_tree &play_cards) {
        auto &pockets = is_response ? bot_info.settings.response_pockets : bot_info.settings.in_play_pockets;

        std::set<play_card_node> play_card_set;
        for (const card_modifier_node &node : play_cards) {
            play_card_set.emplace(play_card_node{ &node });
        }
        
        for (int i=0; i < bot_info.settings.max_random_tries; ++i) {
            auto node_set = play_card_set;
            
            while (!node_set.empty()) {
                play_card_node selected_node = [&]{
                    for (pocket_type pocket : pockets) {
                        if (auto filter = node_set
                            | rv::filter([&](const play_card_node &node) {
                                return node.node->card->pocket == pocket;
                            }))
                        {
                            return random_element(filter, origin->m_game->rng);
                        }
                    }
                    return random_element(node_set, origin->m_game->rng);
                }();

                node_set.erase(selected_node);

                // maybe add random variation?
                bool bypass_prompt = node_set.empty() && i >= bot_info.settings.bypass_prompt_after;
                try {
                    auto args = generate_random_play(origin, *(selected_node.node), is_response);
                    args.bypass_prompt = bypass_prompt;
                    args.timer_id = timer_id;
                    if (verify_and_play(origin, args).is(message_type::ok)) {
                        return true;
                    }
                } catch (const bot_error &error) {
                    // ignore
                }
            }
        }

        // softlock
        fmt::print(stderr, "BOT ERROR: could not find card in execute_random_play()\n");

        return false;
    }

    bool game::request_bot_play() {
        if (pending_requests()) {
            for (player *origin : m_players | rv::filter(&player::is_bot)) {
                card_modifier_tree play_cards = generate_card_modifier_tree(origin, true);
                
                if (!play_cards.empty()) {
                    std::optional<timer_id_t> timer_id;
                    if (auto *timer = origin->m_game->top_request()->timer()) {
                        timer_id = timer->get_timer_id();
                    }

                    if (execute_random_play(origin, true, timer_id, play_cards)) {
                        return true;
                    }
                }
            }
        } else if (m_playing && m_playing->is_bot()) {
            card_modifier_tree play_cards = generate_card_modifier_tree(m_playing);
            return execute_random_play(m_playing, false, std::nullopt, play_cards);
        }
        return false;
    }

    
}