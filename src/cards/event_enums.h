#ifndef __EVENT_ENUMS_H__
#define __EVENT_ENUMS_H__

#include <functional>
#include <memory>

#include "card_serial.h"
#include "utils/enums.h"

namespace banggame {

    struct request_bang;
    struct effect_context;

    using shared_effect_context = std::shared_ptr<effect_context>;

    using card_priority_pair = std::pair<card *, int>;

    #define EVENT(name, ...) (name, std::function<void(__VA_ARGS__)>)
    
    DEFINE_ENUM_TYPES(event_type,
        EVENT(on_game_setup,                    player *first_player)

        EVENT(check_damage_response, player *target, bool &value)

        EVENT(apply_sign_modifier,              card_sign &value)
        EVENT(apply_beer_modifier,              player *origin, int &value)
        EVENT(apply_maxcards_modifier,          player *origin, int &value)
        EVENT(apply_immunity_modifier,          card *origin_card, player *origin, const player *target, effect_flags flags, serial::card_list &cards)
        EVENT(apply_escapable_modifier,         card *origin_card, player *origin, const player *target, effect_flags flags, int &value)
        EVENT(apply_bang_modifier,              player *origin, request_bang *req)

        EVENT(count_usages,                     player *origin, card *origin_card, int &usages)
        EVENT(count_num_checks,                 player *origin, int &num_checks)
        EVENT(count_bangs_played,               player *origin, int &num_bangs_played)
        EVENT(count_cards_to_draw,              player *origin, int &cards_to_draw)
        EVENT(count_range_mod,                  const player *origin, range_mod_type type, int &value)
        EVENT(count_train_equips,               player *origin, int &num_cards, int &num_advance)

        EVENT(check_play_card, player *origin, card *origin_card, const effect_context &ctx, game_string &out_error)

        // verifica per gli effetti che rubano carte in alcune condizioni
        EVENT(check_card_taker, player *target, int type, card* &value)

        // verifica il bersaglio di un'azione
        EVENT(check_card_target, card *origin_card, player *origin, player *target, effect_flags flags, game_string &out_error)

        // verifica prima di passare il turno
        EVENT(check_pass_turn, player *origin, game_string &out_error)

        // viene chiamato quando si sta cercando il prossimo giocatore di turno
        EVENT(check_revivers, player *origin)
        
        // viene chiamato quando scarti una carta a fine turno
        EVENT(on_discard_pass, player *origin, card *target_card)

        EVENT(post_discard_pass, player *origin, int ndiscarded)

        EVENT(get_predraw_checks, player *origin, std::vector<card_priority_pair> &result)

        // viene chiamata quando un giocatore deve estrarre prima di pescare
        EVENT(on_predraw_check, player *origin, card *target_card)

        // viene chiamata quando estrai una carta nel momento che viene pescata
        EVENT(on_draw_check_resolve, player *origin, card *target_card)

        // viene chiamato quando estrai una carta nel momento che viene scelta
        EVENT(on_draw_check_select, player *origin, bool &auto_resolve)

        // viene chiamato quando si scarta VOLONTARIAMENTE una carta (si gioca cat balou o panico contro una carta)
        EVENT(on_destroy_card, player *origin, player *target, card *target_card)

        // viene chiamato quando una carta arancione viene scartata perche' sono finiti i cubetti
        EVENT(on_discard_orange_card, player *target, card *target_card)

        // viene chiamato quando il treno avanza
        EVENT(on_train_advance, player *origin, shared_effect_context ctx)

        // viene chiamato per attivare l'effetto della locomotiva
        EVENT(on_locomotive_effect, player *origin, shared_effect_context ctx)

        // viene chiamato quando un giocatore viene colpito
        EVENT(on_hit, card *origin_card, player *origin, player *target, int damage, effect_flags flags)

        // viene chiamato quando un giocatore gioca mancato
        EVENT(on_missed, card *origin_card, player *origin, player *target, effect_flags flags)

        // viene chiamato quando un giocatore clicca su prendi danno quando muore
        EVENT(on_player_death_resolve, player *target, bool tried_save)

        // viene chiamato quando un giocatore muore
        EVENT(on_player_death, player *origin, player *target)

        // viene chiamato quando un giocatore equipaggia una carta
        EVENT(on_equip_card, player *origin, player *target, card *target_card, const effect_context &ctx)

        // viene chiamato quando un giocatore gioca o scarta una carta dalla mano
        EVENT(on_discard_hand_card, player *origin, card *target_card, bool used)

        // viene chiamato quando un giocatore gioca birra
        EVENT(on_play_beer, player *origin)

        // viene chiamato prima dell'inizio del turno, prima delle estrazioni
        EVENT(pre_turn_start, player *origin)

        // viene chiamato all'inizio del turno, prima di pescare
        EVENT(on_turn_start, player *origin)

        // viene chiamato quando si clicca sul mazzo per pescare in fase di pesca
        EVENT(on_draw_from_deck, player *origin)

        // viene chiamato quando si pesca una carta in fase di pesca
        EVENT(on_card_drawn, player *origin, card *target_card, bool &reveal)
        
        // viene chiamato alla fine del turno
        EVENT(on_turn_end, player *origin, bool skipped)
    )

    #undef EVENT

}

#endif