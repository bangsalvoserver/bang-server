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

    static play_card_args generate_random_play(player *origin, card *origin_card, bool is_response) {
        play_card_args ret;
        std::vector<card *> modifiers;
        effect_context ctx;

        card *playing_card = nullptr;
        while (!playing_card) {
            if (!is_response && filters::is_equip_card(origin_card)) {
                playing_card = origin_card;
                if (!origin_card->self_equippable()) {
                    ret.targets.emplace_back(enums::enum_tag<target_type::player>,
                        random_element(make_equip_set(origin, origin_card, ctx), origin->m_game->rng));
                }
            } else if (origin_card->is_modifier()) {
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

                modifiers.push_back(origin_card);
                auto cards = cards_playable_with_modifiers(origin, modifiers, is_response, ctx);
                origin_card = random_element(cards, origin->m_game->rng);
            } else {
                playing_card = origin_card;
                for (const effect_holder &holder : origin_card->get_effect_list(is_response)) {
                    ret.targets.push_back(generate_random_target(origin, origin_card, holder, ctx));
                }
                if (is_possible_to_play_effects(origin, origin_card, origin_card->optionals, ctx)) {
                    for (const effect_holder &holder : origin_card->optionals) {
                        ret.targets.push_back(generate_random_target(origin, origin_card, holder, ctx));
                    }
                }
            }
        }
        ret.card = playing_card;
        ret.is_response = is_response;
        return ret;
    }

    static bool execute_random_play(player *origin, bool is_response, const serial::card_list &cards, std::initializer_list<pocket_type> pockets) {
        for (int i=0; i<10; ++i) {
            auto card_set = cards | ranges::to<std::set<card *>>;
            while (!card_set.empty()) {
                card *selected_card = [&]{
                    for (pocket_type pocket : pockets) {
                        if (auto filter = card_set | std::views::filter([&](card *c) { return c->pocket == pocket; })) {
                            return random_element(filter, origin->m_game->rng);
                        }
                    }
                    return random_element(card_set, origin->m_game->rng);
                }();

                card_set.erase(selected_card);

                if (!origin->handle_action(enums::enum_tag<game_action_type::play_card>,
                    generate_random_play(origin, selected_card, is_response)))
                {
                    if (origin->m_prompt) {
                        // maybe add random variation to fix softlock?
                        bool response = card_set.empty() && i>=5;
                        origin->handle_action(enums::enum_tag<game_action_type::prompt_respond>, response);
                        if (response) return true;
                    } else {
                        return true;
                    }
                }
            }
        }

        // softlock
        std::cout << "BOT ERROR: could not find card in execute_random_play\n";
        return false;
    }

    static bool respond_to_request(player *origin) {
        auto update = origin->m_game->make_request_update(origin);

        if (!update.pick_cards.empty() && std::ranges::all_of(update.respond_cards, [](card *c) { return c->pocket == pocket_type::button_row; })) {
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
    
    struct bot_delay_request : request_base {
        bot_delay_request(player *origin)
            : request_base(nullptr, nullptr, nullptr)
            , m_timer(this, origin) {}
        
        struct bot_delay_timer : request_timer {
            bot_delay_timer(bot_delay_request *request, player *origin)
                : request_timer(request, 0ms)
                , origin(origin) {}
            
            player *origin;

            void on_finished() override {
                play_in_turn(origin);
            }
        };
        
        bot_delay_timer m_timer;
        request_timer *timer() override { return &m_timer; }
    };

    static constexpr size_t game_max_updates = 5000;

    bool game::request_bot_play(player *origin, bool is_response) {
        if (is_response) {
            return respond_to_request(origin);
        } else if (origin->m_game->m_updates.size() > game_max_updates) {
            queue_request_front<bot_delay_request>(origin);
            return true;
        } else {
            return play_in_turn(origin);
        }
    }
}