#ifndef __EVENTS_H__
#define __EVENTS_H__

#include <functional>
#include <map>
#include <set>

#include "player.h"

namespace banggame {

    struct request_bang;

    #define EVENT(name, ...) (name, std::function<void(__VA_ARGS__)>)
    
    DEFINE_ENUM_TYPES(event_type,
        EVENT(apply_sign_modifier,              player *origin, card_sign &value)
        EVENT(apply_beer_modifier,              player *origin, int &value)
        EVENT(apply_maxcards_modifier,          player *origin, int &value)
        EVENT(apply_volcanic_modifier,          player *origin, bool &value)
        EVENT(apply_immunity_modifier,          card *origin_card, player *origin, const player *target, effect_flags flags, bool &value)
        EVENT(apply_escapable_modifier,         card *origin_card, player *origin, const player *target, effect_flags flags, bool &value)
        EVENT(apply_initial_cards_modifier,     player *origin, int &value)
        EVENT(apply_bang_modifier,              player *origin, request_bang *req)

        // verifica l'effetto ha un bersaglio unico
        EVENT(verify_target_unique, card *origin_card, player *origin, player *target, bool &valid)

        // verifica se sei obbligato a giocare una carta prima di passare
        EVENT(verify_mandatory_card, player *origin, card* &value)

        // viene chiamato quando si sta cercando il prossimo giocatore di turno
        EVENT(verify_revivers, player *origin)
        
        // viene chiamato quando scarti una carta a fine turno
        EVENT(on_discard_pass, player *origin, card *target_card)

        // viene chiamata quando un giocatore deve estrarre prima di pescare
        EVENT(on_predraw_check, player *origin, card *target_card)

        // viene chiamata quando estrai una carta nel momento che viene pescata
        EVENT(on_draw_check, player *origin, card *target_card)

        // viene chiamato quando estrai una carta nel momento che viene scelta
        EVENT(on_draw_check_select, player *origin, card *origin_card, card *drawn_card)

        // viene chiamato quando si scarta VOLONTARIAMENTE una carta (si gioca cat balou o panico contro una carta)
        EVENT(on_discard_card, player *origin, player *target, card *target_card)

        // viene chiamato quando una carta arancione viene scartata perche' sono finiti i cubetti
        EVENT(on_discard_orange_card, player *target, card *target_card)

        // viene chiamato quando un giocatore viene colpito
        EVENT(on_hit, card *origin_card, player *origin, player *target, int damage, bool is_bang)

        // viene chiamato quando un giocatore gioca mancato
        EVENT(on_missed, card *origin_card, player *origin, player *target, bool is_bang)

        // viene chiamato quando un giocatore clicca su prendi danno quando muore
        EVENT(on_player_death_resolve, player *target, bool tried_save)

        // viene chiamato quando un giocatore muore
        EVENT(on_player_death, player *origin, player *target)

        // viene chiamato quando un giocatore equipaggia una carta
        EVENT(on_equip_card, player *origin, player *target, card *target_card)

        // viene chiamato quando un giocatore gioca una carta dalla mano
        EVENT(on_play_hand_card, player *origin, card *target_card)

        // viene chiamato dopo la fine di un effetto
        EVENT(on_effect_end, player *origin, card *target_card)

        // viene chiamato quando un giocatore gioca birra
        EVENT(on_play_beer, player *origin)

        // viene chiamato prima dell'inizio del turno, prima delle estrazioni
        EVENT(pre_turn_start, player *origin)

        // viene chiamato all'inizio del turno, prima di pescare
        EVENT(on_turn_start, player *origin)

        // viene chiamato all'inizio del turno, prima di attivare fase di pesca
        EVENT(on_request_draw, player *origin)

        // viene chiamato quando si clicca sul mazzo per pescare in fase di pesca
        EVENT(on_draw_from_deck, player *origin)

        // viene chiamato quando si pesca una carta in fase di pesca
        EVENT(on_card_drawn, player *origin, card *target_card, bool &reveal)

        // viene chiamato dopo che un giocatore finisce la fase di pesca
        EVENT(post_draw_cards, player *origin)
        
        // viene chiamato alla fine del turno
        EVENT(on_turn_end, player *origin)

        // viene chiamato dopo la fine del turno, prima dei turni extra
        EVENT(post_turn_end, player *origin)
    )

    struct event_card_key {
        card *target_card;
        int priority;

        event_card_key(card *target_card, int priority = 0)
            : target_card(target_card), priority(priority) {}

        bool operator == (const event_card_key &other) const = default;

        auto operator <=> (const event_card_key &other) const {
            return target_card == other.target_card ?
                priority <=> other.priority :
                target_card->id <=> other.target_card->id;
        }

        auto operator <=> (card *other) const {
            return target_card->id <=> other->id;
        }
    };

    template<typename Key, enums::reflected_enum EnumType, typename Compare = std::less<Key>>
    struct priority_double_map {
    private:
        template<typename T>
        struct priority_pair : T {
            int priority;

            template<typename ... Ts>
            priority_pair(int priority, Ts ... args)
                : T{args ...}
                , priority{priority} {}

            auto operator <=> (const priority_pair &other) const {
                return priority <=> other.priority;
            }
        };

        template<typename T> using priority_set = std::multiset<priority_pair<T>, std::greater<>>;
        template<EnumType E> using enum_value_set = priority_set<enums::enum_type_t<E>>;
        template<EnumType E> struct enum_value_iterator : enum_value_set<E>::iterator {};

        template<typename> struct make_enum_table;
        template<EnumType ... Es> struct make_enum_table<enums::enum_sequence<Es ...>> {
            using type = std::tuple<enum_value_set<Es> ...>;
        };
        using enum_table = typename make_enum_table<enums::make_enum_sequence<EnumType>>::type;

        template<typename> struct make_iterator_variant;
        template<EnumType ... Es> struct make_iterator_variant<enums::enum_sequence<Es ...>> {
            using type = std::variant<enum_value_iterator<Es> ...>;
        };
        using iterator_variant = typename make_iterator_variant<enums::make_enum_sequence<EnumType>>::type;

        enum_table m_table;
        std::multimap<Key, iterator_variant, Compare> m_map;

    public:
        template<EnumType E>
        auto &get_table() {
            return std::get<enums::indexof(E)>(m_table);
        }

        template<EnumType E, typename ... Ts>
        void add(Key key, int priority, Ts && ... args) {
            enum_value_iterator<E> it{get_table<E>().emplace(priority, std::forward<Ts>(args) ...)};
            m_map.emplace(std::make_pair(std::move(key), it));
        }

        void erase(auto key) {
            auto [low, high] = m_map.equal_range(key);
            std::for_each(low, high, [this](const auto &pair) {
                std::visit([this]<EnumType E>(enum_value_iterator<E> it) {
                    get_table<E>().erase(it);
                }, pair.second);
            });
            m_map.erase(low, high);
        }
    };

    template<typename T, typename U> struct same_function_args {};

    template<typename Function, typename RetType, typename ... Ts>
    struct same_function_args<Function, std::function<RetType(Ts...)>> : std::is_invocable_r<RetType, Function, Ts...> {};

    template<typename T, event_type E>
    concept invocable_for_event = same_function_args<T, enums::enum_type_t<E>>::value;

    template<typename Function> struct function_argument;
    template<typename T, typename Arg> struct function_argument<void (T::*) (Arg)> : std::type_identity<Arg> {};
    template<typename T, typename Arg> struct function_argument<void (T::*) (Arg) const> : std::type_identity<Arg> {};
    template<typename Function> struct deduce_event_args : function_argument<decltype(&Function::operator())> {};

    struct listener_map {
        priority_double_map<event_card_key, event_type, std::less<>> m_listeners;

        template<event_type E, invocable_for_event<E> Function>
        void add_listener(event_card_key key, Function &&fun) {
            m_listeners.add<E>(key, key.priority, std::forward<Function>(fun));
        }

        void remove_listeners(auto key) {
            m_listeners.erase(key);
        }

        template<event_type E, typename ... Ts>
        void call_event(Ts && ... args) {
            auto &table = m_listeners.get_table<E>();
            auto it = table.begin();
            while (it != table.end()) {
                auto current = it++;
                std::invoke(*current, args ...);
            }
        }
    };

}

#endif