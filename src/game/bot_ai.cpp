#include "game.h"
#include "play_verify.h"
#include "possible_to_play.h"
#include "net/bot_info.h"

#include "game/filters.h"

namespace banggame {

    static game_action generate_random_play(player *origin, const card_modifiers_pair &pair, bool is_response) {
        game_action ret { .card = pair.card };
        effect_context ctx;
        
        for (card *mod_card : pair.modifiers) {
            auto &targets = ret.modifiers.emplace_back(mod_card).targets;

            mod_card->get_modifier(is_response).add_context(mod_card, origin, ctx);
            for (const effect_holder &holder : mod_card->get_effect_list(is_response)) {
                const auto &target = targets.emplace_back(play_dispatch::random_target(origin, mod_card, holder, ctx));
                play_dispatch::add_context(origin, mod_card, holder, ctx, target);
            }
        }

        if (filters::is_equip_card(pair.card)) {
            if (!pair.card->self_equippable()) {
                ret.targets.emplace_back(utils::tag<"player">{},
                    random_element(make_equip_set(origin, pair.card, ctx), origin->m_game->bot_rng));
            }
        } else {
            for (const effect_holder &holder : pair.card->get_effect_list(is_response)) {
                const auto &target = ret.targets.emplace_back(play_dispatch::random_target(origin, pair.card, holder, ctx));
                play_dispatch::add_context(origin, pair.card, holder, ctx, target);
            }
        }

        return ret;
    }

    static bool execute_random_play(player *origin, bool is_response, std::optional<timer_id_t> timer_id, const playable_cards_list &play_cards) {
        auto &pockets = is_response ? bot_info.settings.response_pockets : bot_info.settings.in_play_pockets;

        using card_node = const card_modifiers_pair *;
        std::set<card_node> play_card_set = play_cards | rv::addressof | rn::to<std::set>;
        
        for (int i=0; i < bot_info.settings.max_random_tries; ++i) {
            auto node_set = play_card_set;
            
            while (!node_set.empty()) {
                card_node selected_node = [&]{
                    for (pocket_type pocket : pockets) {
                        if (auto filter = node_set | rv::filter([&](card_node node) {
                            if (node->modifiers.empty()) {
                                return node->card->pocket == pocket;
                            } else {
                                return node->modifiers.front()->pocket == pocket;
                            }
                        })) {
                            return random_element(filter, origin->m_game->bot_rng);
                        }
                    }
                    return random_element(node_set, origin->m_game->bot_rng);
                }();

                node_set.erase(selected_node);

                // maybe add random variation?
                bool bypass_prompt = node_set.empty() && i >= bot_info.settings.bypass_prompt_after;
                try {
                    auto args = generate_random_play(origin, *selected_node, is_response);
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
                playable_cards_list play_cards = generate_playable_cards_list(origin, true);
                
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
            playable_cards_list play_cards = generate_playable_cards_list(m_playing);
            return execute_random_play(m_playing, false, std::nullopt, play_cards);
        }
        return false;
    }

    
}