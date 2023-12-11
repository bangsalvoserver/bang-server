#ifndef __EVENT_TYPE_H__
#define __EVENT_TYPE_H__

#include <memory>

#include "cards/card_serial.h"
#include "utils/utils.h"

namespace banggame {

    struct request_bang;
    struct request_draw;
    struct draw_check_handler;
    struct effect_context;

    using shared_effect_context = std::shared_ptr<effect_context>;
    using shared_request_bang = std::shared_ptr<request_bang>;
    using shared_request_draw = std::shared_ptr<request_draw>;
    using shared_request_check = std::shared_ptr<draw_check_handler>;

    using card_priority_pair = std::pair<card *, int>;
    
    namespace event_type {

        DEFINE_STRUCT(on_game_setup,
            (player *, first_player)
        )

        DEFINE_STRUCT(check_damage_response,
            (player *, target)
            (nullable_ref<bool>, value)
        )

        DEFINE_STRUCT(apply_sign_modifier,
            (nullable_ref<card_sign>, value)
        )

        DEFINE_STRUCT(apply_beer_modifier,
            (player *, origin)
            (nullable_ref<int>, value)
        )

        DEFINE_STRUCT(apply_maxcards_modifier,
            (player *, origin)
            (nullable_ref<int>, value)
        )

        DEFINE_STRUCT(apply_immunity_modifier,
            (card *, origin_card)
            (player *, origin)
            (const player *, target)
            (effect_flags, flags)
            (nullable_ref<serial::card_list>, cards)
        )

        DEFINE_STRUCT(apply_escapable_modifier,
            (card *, origin_card)
            (player *, origin)
            (const player *, target)
            (effect_flags, flags)
            (nullable_ref<int>, value)
        )

        DEFINE_STRUCT(apply_bang_modifier,
            (player *, origin)
            (shared_request_bang, req)
        )

        DEFINE_STRUCT(count_usages,
            (player *, origin)
            (card *, origin_card)
            (nullable_ref<int>, usages)
        )

        DEFINE_STRUCT(count_num_checks,
            (player *, origin)
            (nullable_ref<int>, num_checks)
        )

        DEFINE_STRUCT(count_bangs_played,
            (player *, origin)
            (nullable_ref<int>, num_bangs_played)
        )

        DEFINE_STRUCT(count_cards_to_draw,
            (player *, origin)
            (nullable_ref<int>, cards_to_draw)
        )
        
        DEFINE_STRUCT(count_range_mod,
            (const player *, origin)
            (range_mod_type, type)
            (nullable_ref<int>, value)
        )

        DEFINE_STRUCT(count_train_equips,
            (player *, origin)
            (nullable_ref<int>, num_cards)
            (nullable_ref<int>, num_advance)
        )

        DEFINE_STRUCT(check_play_card,
            (player *, origin)
            (card *, origin_card)
            (const effect_context &, ctx)
            (nullable_ref<game_string>, out_error)
        )

        // verifica per gli effetti che rubano carte in alcune condizioni
        DEFINE_STRUCT(check_card_taker,
            (player *, target)
            (int, type)
            (nullable_ref<card *>, value)
        )

        // verifica il bersaglio di un'azione
        DEFINE_STRUCT(check_card_target,
            (card *, origin_card)
            (player *, origin)
            (player *, target)
            (effect_flags, flags)
            (nullable_ref<game_string>, out_error)
        )

        // verifica prima di passare il turno
        DEFINE_STRUCT(check_pass_turn,
            (player *, origin)
            (nullable_ref<game_string>, out_error)
        )

        // viene chiamato quando si sta cercando il prossimo giocatore di turno
        DEFINE_STRUCT(check_revivers,
            (player *, origin)
        )
        
        // viene chiamato quando scarti una carta a fine turno
        DEFINE_STRUCT(on_discard_pass,
            (player *, origin)
            (card *, target_card)
        )

        DEFINE_STRUCT(post_discard_pass,
            (player *, origin)
            (int, ndiscarded)
        )

        DEFINE_STRUCT(get_predraw_checks,
            (player *, origin)
            (nullable_ref<std::vector<card_priority_pair>>, result)
        )

        // viene chiamata quando un giocatore deve estrarre prima di pescare
        DEFINE_STRUCT(on_predraw_check,
            (player *, origin)
            (card *, target_card)
        )

        // viene chiamata quando estrai una carta nel momento che viene pescata
        DEFINE_STRUCT(on_draw_check_resolve,
            (player *, origin)
            (card *, target_card)
        )

        // viene chiamato quando estrai una carta nel momento che viene scelta
        DEFINE_STRUCT(on_draw_check_select,
            (player *, origin)
            (shared_request_check, req)
            (nullable_ref<bool>, handled)
        )

        // viene chiamato quando si scarta VOLONTARIAMENTE una carta (si gioca cat balou o panico contro una carta)
        DEFINE_STRUCT(on_destroy_card,
            (player *, origin)
            (player *, target)
            (card *, target_card)
        )

        // viene chiamato quando una carta arancione viene scartata perche' sono finiti i cubetti
        DEFINE_STRUCT(on_discard_orange_card,
            (player *, target)
            (card *, target_card)
        )

        // viene chiamato quando il treno avanza
        DEFINE_STRUCT(on_train_advance,
            (player *, origin)
            (shared_effect_context, ctx)
        )

        // viene chiamato per attivare l'effetto della locomotiva
        DEFINE_STRUCT(on_locomotive_effect,
            (player *, origin)
            (shared_effect_context, ctx)
        )

        // viene chiamato quando un giocatore viene colpito
        DEFINE_STRUCT(on_hit,
            (card *, origin_card)
            (player *, origin)
            (player *, target)
            (int, damage)
            (effect_flags, flags)
        )

        // viene chiamato quando un giocatore gioca mancato
        DEFINE_STRUCT(on_missed,
            (card *, origin_card)
            (player *, origin)
            (player *, target)
            (card *, missed_card)
            (effect_flags, flags)
        )

        // viene chiamato quando un giocatore clicca su prendi danno quando muore
        DEFINE_STRUCT(on_player_death_resolve,
            (player *, target)
            (bool, tried_save)
        )

        // viene chiamato quando un giocatore muore
        DEFINE_STRUCT(on_player_death,
            (player *, origin)
            (player *, target)
        )

        // viene chiamato quando un giocatore equipaggia una carta
        DEFINE_STRUCT(on_equip_card,
            (player *, origin)
            (player *, target)
            (card *, target_card)
            (const effect_context &, ctx)
        )

        // viene chiamato quando un giocatore gioca o scarta una carta dalla mano
        DEFINE_STRUCT(on_discard_hand_card,
            (player *, origin)
            (card *, target_card)
            (bool, used)
        )

        // viene chiamato quando un giocatore gioca birra
        DEFINE_STRUCT(on_play_beer,
            (player *, origin)
        )

        // viene chiamato durante il cambio del turno da un giocatore all'altro
        DEFINE_STRUCT(on_turn_switch,
            (player *, origin)
        )

        // viene chiamato prima dell'inizio del turno, prima delle estrazioni
        DEFINE_STRUCT(pre_turn_start,
            (player *, origin)
        )

        // viene chiamato all'inizio del turno, prima di pescare
        DEFINE_STRUCT(on_turn_start,
            (player *, origin)
        )

        // viene chiamato quando si clicca sul mazzo per pescare in fase di pesca
        DEFINE_STRUCT(on_draw_from_deck,
            (player *, origin)
            (shared_request_draw, req)
            (nullable_ref<bool>, handled)
        )

        // viene chiamato quando si pesca una carta in fase di pesca
        DEFINE_STRUCT(on_card_drawn,
            (player *, origin)
            (card *, target_card)
            (shared_request_draw, req)
            (nullable_ref<bool>, reveal)
        )
        
        // viene chiamato alla fine del turno
        DEFINE_STRUCT(on_turn_end,
            (player *, origin)
            (bool, skipped)
        )

    }

}

#endif