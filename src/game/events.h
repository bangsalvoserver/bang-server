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

        EVENT(verify_play_card, player *origin, card *origin_card, game_string &out_error)

        // verifica per gli effetti che rubano carte in alcune condizioni
        EVENT(verify_card_taker, player *target, equip_type type, bool &value)

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

        // viene chiamato prima che un giocatore viene colpito
        EVENT(before_hit, card *origin_card, player *origin, player *target, int damage, bool is_bang)

        // viene chiamato dopo che un giocatore viene colpito
        EVENT(after_hit, card *origin_card, player *origin, player *target, int damage, bool is_bang)

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
        EVENT(phase_one_override, player *origin)

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
            : target_card(target_card)
            , priority(priority) {}

        bool operator == (const event_card_key &other) const = default;

        auto operator <=> (const event_card_key &other) const {
            return target_card == other.target_card ?
                priority <=> other.priority :
                target_card <=> other.target_card;
        }

        auto operator <=> (card *other) const {
            return target_card <=> other;
        }

        bool priority_greater(const event_card_key &other) const {
            return priority == other.priority ?
                target_card < other.target_card :
                priority > other.priority;
        }
    };

    template<typename T>
    concept priority_key = requires (T value) {
        { value.priority_greater(value) } -> std::convertible_to<bool>;
    };

    template<priority_key Key, enums::reflected_enum EnumType>
    struct priority_double_map {
    private:
        enum value_status : uint8_t {
            inactive, active, erased
        };

        struct value_variant {
            const enums::enum_variant<EnumType> value;
            value_status status = inactive;

            template<typename ... Ts>
            value_variant(Ts && ... args) : value{FWD(args)...} {}
        };
        
        struct iterator_compare {
            template<typename T>
            bool operator()(const T &lhs, const T &rhs) const {
                return lhs->first.priority_greater(rhs->first)
                    || !rhs->first.priority_greater(lhs->first)
                    && (&*lhs < &*rhs);
            }
        };

        using container_map = std::multimap<Key, value_variant, std::less<>>;
        using container_iterator = typename container_map::iterator;
        using iterator_set = std::set<container_iterator, iterator_compare>;

        struct set_lock_pair {
            uint8_t lock_count = 0;
            iterator_set set;
        };

        template<std::invocable Function, typename T>
        class on_destroy_do : public T {
        private:
            Function m_fun;
        
        public:
            on_destroy_do(Function &&fun, T &&value)
                : T{std::move(value)}
                , m_fun(fun) {}

            on_destroy_do(const on_destroy_do &) = delete;
            on_destroy_do(on_destroy_do &&other)
                noexcept(std::is_nothrow_move_constructible_v<T>
                    && std::is_nothrow_move_constructible_v<Function>)
                : T{std::move(other)}
                , m_fun{std::move(other.m_fun)} {}

            on_destroy_do &operator = (const on_destroy_do &) = delete;
            on_destroy_do &operator = (on_destroy_do &&other)
                noexcept(std::is_nothrow_move_assignable_v<T>
                    && std::is_nothrow_move_assignable_v<Function>)
            {
                static_cast<T &>(*this) = std::move(other);
                std::swap(m_fun, other.m_fun);
                return *this;
            }

            ~on_destroy_do() {
                std::invoke(m_fun);
            }
        };

        using iterator_table = std::array<set_lock_pair, enums::num_members_v<EnumType>>;
        using iterator_vector = std::vector<container_iterator>;

        container_map m_map;
        iterator_table m_table;
        iterator_vector m_changes;

    public:
        template<EnumType E, typename ... Ts>
        void add(Key key, Ts && ... args) {
            auto it = m_map.emplace(std::piecewise_construct,
                std::make_tuple(std::move(key)),
                std::make_tuple(enums::enum_tag<E>, FWD(args) ...));
            m_table[enums::indexof(E)].set.emplace(it);
            m_changes.push_back(it);
        }

        template<typename T>
        void erase(const T &key) {
            auto [low, high] = m_map.equal_range(key);
            for (; low != high; ++low) {
                low->second.status = erased;
                m_changes.push_back(low);
            }
        }

        void commit_changes() {
            std::erase_if(m_changes, [&](container_iterator it) {
                auto &set = m_table[it->second.value.index()];
                if (set.lock_count == 0) {
                    switch (it->second.status) {
                    case inactive:
                        it->second.status = active;
                        break;
                    case erased:
                        set.set.erase(it);
                        m_map.erase(it);
                    }
                    return true;
                }
                return false;
            });
        }

        template<EnumType E>
        auto get_table() {
            auto &set = m_table[enums::indexof(E)];
            ++set.lock_count;
            return on_destroy_do([&]{
                --set.lock_count;
            }, set.set
                | std::views::filter([](container_iterator it) {
                    return it->second.status == active;
                })
                | std::views::transform([](container_iterator it) -> decltype(auto) {
                    return it->second.value.template get<E>();
                }));
        }
    };

    template<typename T, typename U> struct same_function_args {};

    template<typename Function, typename RetType, typename ... Ts>
    struct same_function_args<Function, std::function<RetType(Ts...)>> : std::is_invocable_r<RetType, Function, Ts...> {};

    template<typename T, event_type E>
    concept invocable_for_event = same_function_args<T, enums::enum_type_t<E>>::value;

    template<typename Function> struct count_reference_params;
    template<typename Function> static constexpr size_t count_reference_params_v = count_reference_params<Function>::value;

    template<typename RetType, typename ... Ts>
    struct count_reference_params<std::function<RetType(Ts...)>> {
        static constexpr size_t value = (std::is_lvalue_reference_v<Ts> + ...);
    };

    template<typename Function> struct find_reference_param;
    template<typename Function> static constexpr size_t find_reference_param_v = find_reference_param<Function>::value;

    template<typename ... Ts> struct find_reference;

    template<typename First, typename ... Ts>
    struct find_reference<First, Ts ...> {
        static constexpr size_t value = 1 + find_reference<Ts...>::value;
    };

    template<typename First, typename ... Ts>
    requires std::is_lvalue_reference_v<First>
    struct find_reference<First, Ts ...> {
        static constexpr size_t value = 0;
    };

    template<typename RetType, typename ... Ts>
    struct find_reference_param<std::function<RetType(Ts...)>> : find_reference<Ts...> {};

    template<typename Function> struct function_argument_tuple;
    template<typename Function> using function_argument_tuple_t = typename function_argument_tuple<Function>::type;

    template<typename RetType, typename ... Ts>
    struct function_argument_tuple<std::function<RetType(Ts...)>> {
        using type = std::tuple<std::remove_reference_t<Ts> ...>;
    };

    class listener_map {
    private:
        priority_double_map<event_card_key, event_type> m_listeners;

    public:
        template<event_type E, invocable_for_event<E> Function>
        void add_listener(event_card_key key, Function &&fun) {
            m_listeners.add<E>(key, std::forward<Function>(fun));
            m_listeners.commit_changes();
        }

        void remove_listeners(auto key) {
            m_listeners.erase(key);
            m_listeners.commit_changes();
        }

        template<event_type E, typename ... Ts>
        auto call_event(Ts && ... args) {
            using function_type = enums::enum_type_t<E>;
            function_argument_tuple_t<function_type> tup{FWD(args) ...};

            for (auto &fun : m_listeners.get_table<E>()) {
                std::apply(fun, tup);
            }
            m_listeners.commit_changes();
            
            if constexpr (count_reference_params_v<function_type> == 1) {
                return std::get<find_reference_param_v<function_type>>(tup);
            }
        }
    };

}

#endif