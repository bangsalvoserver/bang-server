#include "game.h"
#include "play_verify.h"
#include "possible_to_play.h"
#include "net/bot_info.h"

#include "game/filters.h"

namespace banggame {

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
                        ret.targets.emplace_back(utils::tag<"player">{},
                            random_element(make_equip_set(origin, playing_card, ctx), origin->m_game->bot_rng));
                    }
                } else {
                    for (const effect_holder &holder : playing_card->get_effect_list(is_response)) {
                        const auto &target = ret.targets.emplace_back(play_dispatch::random_target(origin, playing_card, holder, ctx));
                        play_dispatch::add_context(origin, playing_card, holder, ctx, target);
                    }
                }
            } else {
                card *origin_card = cur_node->card;
                auto &targets = ret.modifiers.emplace_back(origin_card).targets;

                origin_card->get_modifier(is_response).add_context(origin_card, origin, ctx);
                for (const effect_holder &holder : origin_card->get_effect_list(is_response)) {
                    const auto &target = targets.emplace_back(play_dispatch::random_target(origin, origin_card, holder, ctx));
                    play_dispatch::add_context(origin, origin_card, holder, ctx, target);
                }

                cur_node = random_element(cur_node->branches
                    | rv::transform([](const card_modifier_node &node) { return &node; }),
                    origin->m_game->bot_rng);
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
                            return random_element(filter, origin->m_game->bot_rng);
                        }
                    }
                    return random_element(node_set, origin->m_game->bot_rng);
                }();

                node_set.erase(selected_node);

                // maybe add random variation?
                bool bypass_prompt = node_set.empty() && i >= bot_info.settings.bypass_prompt_after;
                try {
                    auto args = generate_random_play(origin, *(selected_node.node), is_response);
                    args.bypass_prompt = bypass_prompt;
                    args.timer_id = timer_id;
                    if (verify_and_play(origin, args).type == message_type::ok) {
                        return true;
                    }
                } catch (const random_element_error &error) {
                    // ignore
                }
            }
        }

        // softlock
        std::cerr << "BOT ERROR: could not find card in execute_random_play()\n";

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