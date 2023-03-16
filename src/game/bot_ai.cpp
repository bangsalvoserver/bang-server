#include "game.h"
#include "play_verify.h"
#include "possible_to_play.h"

#include "cards/effect_enums.h"
#include "cards/filters.h"

namespace banggame {

    template<std::ranges::range Range, typename Rng>
    decltype(auto) random_element(Range &&range, Rng &rng) {
        std::ranges::range_value_t<Range> ret;
        if (std::ranges::sample(std::forward<Range>(range), &ret, 1, rng) == &ret) {
            throw std::runtime_error("empty range in random_element");
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
            auto targets = ranges::to<std::vector>(make_player_target_set(origin, origin_card, holder, ctx));
            if (targets.empty()) {
                return nullptr;
            } else {
                return random_element(targets, origin->m_game->rng);
            }
        }

        card *operator()(enums::enum_tag_t<target_type::card>) const {
            auto targets = ranges::to<std::vector>(make_card_target_set(origin, origin_card, holder, ctx));
            return random_element(targets, origin->m_game->rng);
        }

        card *operator()(enums::enum_tag_t<target_type::extra_card> tag) const {
            if (ctx.repeat_card) {
                return nullptr;
            } else {
                auto targets = ranges::to<std::vector>(make_card_target_set(origin, origin_card, holder, ctx));
                return random_element(targets, origin->m_game->rng);
            }
        }

        auto operator()(enums::enum_tag_t<target_type::cards> tag) const {
            auto targets = ranges::to<std::vector>(make_card_target_set(origin, origin_card, holder, ctx));
            return targets
                | ranges::views::sample(holder.target_value, origin->m_game->rng)
                | ranges::to<serial::card_list>;
        }

        auto operator()(enums::enum_tag_t<target_type::cards_other_players>) const {
            return range_other_players(origin)
                | ranges::views::transform([](player *target) {
                    return ranges::views::concat(
                        target->m_table | ranges::views::remove_if(&card::is_black),
                        target->m_hand | ranges::views::take(1)
                    );
                })
                | ranges::views::remove_if(ranges::empty)
                | ranges::views::transform([&](auto &&range) {
                    return random_element(range, origin->m_game->rng);
                })
                | ranges::to<serial::card_list>;
        }

        auto operator()(enums::enum_tag_t<target_type::select_cubes>) const {
            auto cubes = origin->cube_slots()
                | ranges::views::for_each([](card *slot) {
                    return ranges::views::repeat_n(slot, slot->num_cubes);
                })
                | ranges::to<std::vector>;
            return cubes
                | ranges::views::sample(holder.target_value, origin->m_game->rng)
                | ranges::to<serial::card_list>;
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

    static play_card_args generate_random_play(player *origin, const card_modifier_node &node, bool is_response) {
        play_card_args ret;
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
                    | ranges::views::transform([](const card_modifier_node &node) { return &node; }),
                    origin->m_game->rng);
            }
        }
        ret.card = playing_card;
        ret.is_response = is_response;
        return ret;
    }

    static bool execute_random_play(player *origin, bool is_response, const card_modifier_tree &cards, std::initializer_list<pocket_type> pockets) {
        for (int i=0; i<10; ++i) {
            auto node_set = cards
                | ranges::views::transform([](const card_modifier_node &node) { return &node; })
                | ranges::to<std::set>;
            
            while (!node_set.empty()) {
                const card_modifier_node *selected_node = [&]{
                    for (pocket_type pocket : pockets) {
                        if (auto filter = node_set
                            | std::views::filter([&](const card_modifier_node *node) { return node->card->pocket == pocket; }))
                        {
                            return random_element(filter, origin->m_game->rng);
                        }
                    }
                    return random_element(node_set, origin->m_game->rng);
                }();

                node_set.erase(selected_node);

                try {
                    if (!origin->handle_action(enums::enum_tag<game_action_type::play_card>,
                        generate_random_play(origin, *selected_node, is_response)))
                    {
                        if (origin->m_prompt) {
                            // maybe add random variation to fix softlock?
                            bool response = node_set.empty() && i>=5;
                            origin->handle_action(enums::enum_tag<game_action_type::prompt_respond>, response);
                            if (response) return true;
                        } else {
                            return true;
                        }
                    }
                } catch (const std::exception &e) {
                    std::cout << "BOT ERROR: " << e.what() << std::endl;
                }
            }
        }

        // softlock
        std::cout << "BOT ERROR: could not find card in execute_random_play" << std::endl;
        return false;
    }

    static bool respond_to_request(player *origin) {
        auto update = origin->m_game->make_request_update(origin);

        if (!update.pick_cards.empty() && std::ranges::all_of(update.respond_cards, [](const card_modifier_node &node) {
            return node.card->has_tag(tag_type::confirm);
        })) {
            origin->handle_action(enums::enum_tag<game_action_type::pick_card>,
                random_element(update.pick_cards, origin->m_game->rng));
            return true;
        } else if (!update.respond_cards.empty()) {
            return execute_random_play(origin, true, update.respond_cards, {
                pocket_type::player_character,
                pocket_type::player_table,
                pocket_type::player_hand
            });
        }
        return false;
    }

    static bool play_in_turn(player *origin) {
        auto update = origin->m_game->make_status_ready_update(origin);

        return execute_random_play(origin, false, update.play_cards, {
            pocket_type::player_character,
            pocket_type::player_table,
            pocket_type::player_hand,
            pocket_type::shop_selection
        });
    }

    bool game::request_bot_play(player *origin, bool is_response) {
        if (is_response) {
            return respond_to_request(origin);
        } else {
            return play_in_turn(origin);
        }
    }
}