#ifndef __CARD_ENUMS_H__
#define __CARD_ENUMS_H__

#include "utils/enum_variant.h"
#include "utils/reflector.h"

namespace banggame {

    DEFINE_ENUM_DATA(card_suit,
        (none,      "")
        (hearts,    "\u2665")
        (diamonds,  "\u2666")
        (clubs,     "\u2663")
        (spades,    "\u2660")
    )

    DEFINE_ENUM_DATA(card_rank,
        (none,      "")
        (rank_A,    "A")
        (rank_2,    "2")
        (rank_3,    "3")
        (rank_4,    "4")
        (rank_5,    "5")
        (rank_6,    "6")
        (rank_7,    "7")
        (rank_8,    "8")
        (rank_9,    "9")
        (rank_10,   "10")
        (rank_J,    "J")
        (rank_Q,    "Q")
        (rank_K,    "K")
    )

    struct card_sign {REFLECTABLE(
        (card_suit) suit,
        (card_rank) rank
    )
        explicit operator bool () const {
            return suit != card_suit::none && rank != card_rank::none;
        }
    };

    struct unofficial_expansion{};
    
    DEFINE_ENUM_FLAGS_DATA(card_expansion_type,
        (characterchoice)
        (dodgecity)
        (goldrush)
        (armedanddangerous)
        (valleyofshadows)
        (canyondiablo,      unofficial_expansion{})
        (highnoon)
        (fistfulofcards)
        (wildwestshow)
        (thebullet)
        (ghostcards)
    )

    DEFINE_ENUM(card_color_type,
        (none)
        (brown)
        (blue)
        (green)
        (black)
        (orange)
    )

    DEFINE_ENUM(player_role,
        (unknown)
        (sheriff)
        (deputy)
        (outlaw)
        (renegade)
        (deputy_3p)
        (outlaw_3p)
        (renegade_3p)
    )

    DEFINE_ENUM_FLAGS(target_player_filter,
        (any)
        (dead)
        (self)
        (notself)
        (notsheriff)
        (range_1)
        (range_2)
        (reachable)
    )

    DEFINE_ENUM_FLAGS(target_card_filter,
        (table)
        (hand)
        (blue)
        (black)
        (clubs)
        (bang)
        (missed)
        (beer)
        (bronco)
        (cube_slot)
        (cube_slot_card)
        (can_repeat)
    )

    DEFINE_ENUM_TYPES(play_card_target_type,
        (none)
        (player, int)
        (card, int)
        (other_players)
        (cards_other_players, std::vector<int>)
    )

    using play_card_target_ids = enums::enum_variant<play_card_target_type>;

    DEFINE_ENUM(card_deck_type,
        (none)
        (main_deck)
        (character)
        (role)
        (goldrush)
        (highnoon)
        (fistfulofcards)
        (wildwestshow)
    )

    DEFINE_ENUM(pocket_type,
        (none)
        (player_hand)
        (player_table)
        (player_character)
        (player_backup)
        (main_deck)
        (discard_pile)
        (selection)
        (shop_deck)
        (shop_discard)
        (shop_selection)
        (hidden_deck)
        (scenario_deck)
        (scenario_card)
        (specials)
    )

    DEFINE_ENUM(card_modifier_type,
        (none)
        (bangmod)
        (discount)
        (leevankliff)
        (shopchoice)
        (belltower)
        (bandolier)
    )

    DEFINE_ENUM_FLAGS(effect_flags,
        (escapable)
        (single_target)
    )
    
    DEFINE_ENUM_FWD_TYPES(effect_type,
        (none)
        (play_card_action,      effect_play_card_action)
        (max_usages,            effect_max_usages)
        (pass_turn,             effect_pass_turn)
        (resolve,               effect_resolve)
        (mth_add,               effect_empty)
        (bang,                  effect_bang)
        (bangcard,              effect_bangcard)
        (banglimit,             effect_banglimit)
        (missedlike,            effect_missedlike)
        (missed,                effect_missed)
        (bangresponse,          effect_bangresponse)
        (barrel,                effect_barrel)
        (destroy,               effect_destroy)
        (choose_card,           effect_choose_card)
        (startofturn,           effect_startofturn)
        (while_drawing,         effect_while_drawing)
        (end_drawing,           effect_end_drawing)
        (steal,                 effect_steal)
        (duel,                  effect_duel)
        (beer,                  effect_beer)
        (heal,                  effect_heal)
        (heal_notfull,          effect_heal_notfull)
        (saloon,                effect_saloon)
        (indians,               effect_indians)
        (draw,                  effect_draw)
        (draw_discard,          effect_draw_discard)
        (draw_to_discard,       effect_draw_to_discard)
        (draw_one_less,         effect_draw_one_less)
        (generalstore,          effect_generalstore)
        (deathsave,             effect_deathsave)
        (backfire,              effect_backfire)
        (bandidos,              effect_bandidos)
        (aim,                   effect_aim)
        (poker,                 effect_poker)
        (tornado,               effect_tornado)
        (damage,                effect_damage)
        (saved,                 effect_saved)
        (escape,                effect_escape)
        (sell_beer,             effect_sell_beer)
        (discard_black,         effect_discard_black)
        (add_gold,              effect_add_gold)
        (pay_gold,              effect_pay_gold)
        (rum,                   effect_rum)
        (goldrush,              effect_goldrush)
        (select_cube,           effect_select_cube)
        (pay_cube,              effect_pay_cube)
        (add_cube,              effect_add_cube)
        (reload,                effect_reload)
        (rust,                  effect_rust)
        (doublebarrel,          effect_doublebarrel)
        (thunderer,             effect_thunderer)
        (buntlinespecial,       effect_buntlinespecial)
        (bigfifty,              effect_bigfifty)
        (bandolier,             effect_bandolier)
        (move_bomb,             effect_move_bomb)
        (tumbleweed,            effect_tumbleweed)
        (sniper,                effect_sniper)
        (ricochet,              effect_ricochet)
        (teren_kill,            effect_teren_kill)
        (greygory_deck,         effect_greygory_deck)
        (lemonade_jim,          effect_lemonade_jim)
        (josh_mccloud,          effect_josh_mccloud)
        (frankie_canton,        effect_frankie_canton)
        (evelyn_shebang,        effect_evelyn_shebang)
        (red_ringo,             effect_red_ringo)
        (al_preacher,           effect_al_preacher)
        (ms_abigail,            effect_ms_abigail)
        (graverobber,           effect_graverobber)
        (mirage,                effect_mirage)
        (disarm,                effect_disarm)
        (sacrifice,             effect_sacrifice)
        (lastwill,              effect_lastwill)
    )

    DEFINE_ENUM_FWD_TYPES(equip_type,
        (none)
        (mustang,               effect_mustang)
        (scope,                 effect_scope)
        (jail,                  effect_jail)
        (dynamite,              effect_dynamite)
        (horse,                 effect_horse)
        (weapon,                effect_weapon)
        (volcanic,              effect_volcanic)
        (pickaxe,               effect_pickaxe)
        (calumet,               effect_calumet)
        (boots,                 effect_boots)
        (ghost,                 effect_ghost)
        (snake,                 effect_snake)
        (shotgun,               effect_shotgun)
        (bounty,                effect_bounty)
        (el_gringo,             effect_el_gringo)
        (horsecharm,            effect_horsecharm)
        (luckycharm,            effect_luckycharm)
        (gunbelt,               effect_gunbelt)
        (wanted,                effect_wanted)
        (bomb,                  effect_bomb)
        (tumbleweed,            effect_tumbleweed)
        (bronco,                effect_bronco)
        (calamity_janet,        effect_calamity_janet)
        (black_jack,            effect_black_jack)
        (kit_carlson,           effect_kit_carlson)
        (claus_the_saint,       effect_claus_the_saint)
        (bill_noface,           effect_bill_noface)
        (slab_the_killer,       effect_slab_the_killer)
        (suzy_lafayette,        effect_suzy_lafayette)
        (vulture_sam,           effect_vulture_sam)
        (johnny_kisch,          effect_johnny_kisch)
        (bellestar,             effect_bellestar)
        (greg_digger,           effect_greg_digger)
        (herb_hunter,           effect_herb_hunter)
        (molly_stark,           effect_molly_stark)
        (tequila_joe,           effect_tequila_joe)
        (vera_custer,           effect_vera_custer)
        (tuco_franziskaner,     effect_tuco_franziskaner)
        (colorado_bill,         effect_colorado_bill)
        (henry_block,           effect_henry_block)
        (big_spencer,           effect_big_spencer)
        (gary_looter,           effect_gary_looter)
        (john_pain,             effect_john_pain)
        (youl_grinner,          effect_youl_grinner)
        (don_bell,              effect_don_bell)
        (dutch_will,            effect_dutch_will)
        (madam_yto,             effect_madam_yto)
        (greygory_deck,         effect_greygory_deck)
        (lemonade_jim,          effect_lemonade_jim)
        (evelyn_shebang,        effect_evelyn_shebang)
        (julie_cutter,          effect_julie_cutter)
        (bloody_mary,           effect_bloody_mary)
        (red_ringo,             effect_red_ringo)
        (al_preacher,           effect_al_preacher)
        (ms_abigail,            effect_ms_abigail)
        (blessing,              effect_blessing)
        (curse,                 effect_curse)
        (thedaltons,            effect_thedaltons)
        (thedoctor,             effect_thedoctor)
        (trainarrival,          effect_trainarrival)
        (thirst,                effect_thirst)
        (highnoon,              effect_highnoon)
        (shootout,              effect_shootout)
        (invert_rotation,       effect_invert_rotation)
        (reverend,              effect_reverend)
        (hangover,              effect_hangover)
        (sermon,                effect_sermon)
        (ghosttown,             effect_ghosttown)
        (handcuffs,             effect_handcuffs)
        (ambush,                effect_ambush)
        (lasso,                 effect_lasso)
        (judge,                 effect_judge)
        (peyote,                effect_peyote)
        (russianroulette,       effect_russianroulette)
        (abandonedmine,         effect_abandonedmine)
        (deadman,               effect_deadman)
        (fistfulofcards,        effect_fistfulofcards)
        (packmule,              effect_packmule)
        (indianguide,           effect_indianguide)
        (taxman,                effect_taxman)
        (prompt_on_self_equip,  effect_prompt_on_self_equip)
        (brothel,               effect_brothel)
        (newidentity,           effect_newidentity)
        (lawofthewest,          effect_lawofthewest)
        (vendetta,              effect_vendetta)
    )

    DEFINE_ENUM_FWD_TYPES(mth_type,
        (none)
        (doc_holyday,           handler_doc_holyday)
        (flint_westwood,        handler_flint_westwood)
        (draw_atend,            handler_draw_atend)
        (heal_multi,            handler_heal_multi)
        (fanning,               handler_fanning)
        (flintlock,             handler_flintlock)
        (duck,                  handler_duck)
        (move_bomb,             handler_move_bomb)
        (squaw,                 handler_squaw)
        (card_sharper,          handler_card_sharper)
        (lastwill,              handler_lastwill)
    )

    DEFINE_ENUM(tag_type,
        (none)
        (bangcard)
        (missedcard)
        (bangproxy)
        (beer)
        (indians)
        (drawing)
        (weapon)
        (horse)
        (repeatable)
        (shopchoice)
        (peyote)
        (handcuffs)
        (buy_cost)
        (max_hp)
        (vulture_sam)
        (gary_looter)
        (john_pain)
        (bronco)
    )

}

#endif