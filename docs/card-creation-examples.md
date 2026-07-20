# Practical examples: how to create a new card in bang-server

This guide builds on the two architecture documents already produced, showing **concrete use cases** for adding a card, in increasing order of complexity. For each case I indicate: what needs to change, where, and why — using real patterns already present in the codebase (`base.yml` and `src/effects/base/`) as reference.

General rule to keep in mind: **if the effect/target you need already exists** (even by combining several), all you need is YAML — zero C++, zero recompilation of logic. C++ is only needed when the card introduces **new behavior** that can't be expressed with the existing building blocks.

---

## Case 1 — A card using an already-existing effect and target (YAML only)

**Goal**: add a brown card, "SURPRISE SHOT," that steals a random card from a player's hand within distance 1 — exactly *Panic!*'s behavior, but with different suits (hypothesis: a new reprint/variant).

All it takes is a block in `src/config/sets/base.yml` (or a dedicated expansion file), reusing the `steal` effect and the `random_if_hand_card` target already implemented for *Panic!*:

```yaml
- name: SURPRISE_SHOT
  signs:
    - 7 diamonds
  image: 01_surpriseshot
  color: brown
  effects:
    - steal random_if_hand_card range_1
  tags:
    - skip_logs
    - catbalou_panic
    - ranged_effect
```

What happens at build time: `parse_bang_cards.py` reads this entry, resolves `steal` → `GET_EFFECT(steal)` (implemented in `effects/base/steal_destroy.cpp`) and `random_if_hand_card` → `TARGET_TYPE(random_if_hand_card)` (in `target_types/base/random_if_hand_card.cpp`), and generates the corresponding C++ initialization. **No C++ file needs to be touched.**

Frontend step: add the entry in `Locale/<Language>/Cards.tsx` with a localized name and description, and the matching image in `public/cards/`. If you skip this, the card still works but shows up with only its technical identifier (`hideTitle: true`, per the fallback in `getCardRegistryEntry`).

---

## Case 2 — Composing several existing effects in sequence (still YAML only)

**Goal**: a character who, like *Sid Ketchum*, can discard 2 cards to heal 1 HP — both as an in-turn action and as a response to a lethal attack.

Looking at the real implementation of *Sid Ketchum* in `base.yml`:

```yaml
- name: SID_KETCHUM
  image: 01_sidketchum
  effects:
    - discard cards(2) self | hand
    - heal
  responses:
    - deathsave
    - discard cards(2) self | hand
    - heal
```

Here, **three already-existing "building blocks"** (`discard`, `heal`, `deathsave`) are chained together: first, 2 cards are discarded from your own hand (`cards(2)` is a multi-target with a numeric parameter, `self | hand` combines the player filter `self` and the card filter `hand`), then you heal. The `responses` section makes the same combination available as a reaction when the character is about to die (`deathsave` is the special effect/target that hooks the response into the last-second save mechanic).

This pattern — **composing generic effects in sequence** — covers the vast majority of the game's "simple" cards (also see `STAGECOACH`: `draw(2)` + `changewws`, or `GENERAL_STORE`: `generalstore` + `generalstore players alive`). No C++ is needed as long as the effects involved are generic and parametrizable.

---

## Case 3 — Equipment with a permanent passive ability (needs a new C++ effect)

**Goal**: a new equippable blue card that increases your weapons' range by 1 (like *Scope*, but we're using it as an example of a "new" effect implemented from scratch).

When the ability can't be parametrized from YAML but requires **reacting to an engine event**, a small C++ struct is needed. Here is the minimal skeleton, modeled on the real implementation of *Scope*:

`src/effects/base/my_binoculars.h`:
```cpp
#ifndef __BASE_MY_BINOCULARS_H__
#define __BASE_MY_BINOCULARS_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_my_binoculars : event_equip {
        void on_enable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(my_binoculars, equip_my_binoculars)
}

#endif
```

`src/effects/base/my_binoculars.cpp`:
```cpp
#include "my_binoculars.h"
#include "cards/game_events.h"
#include "game/game_table.h"

namespace banggame {
    void equip_my_binoculars::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::count_range_mod>(target_card,
            [=](const_player_ptr origin, range_mod_type type, int &value) {
                if (origin == target && type == range_mod_type::range_mod) {
                    ++value;
                }
            });
    }
}
```

Necessary steps:
1. Register the new file in `effects/base/`'s `CMakeLists.txt` (or the relevant expansion folder's).
2. Include it from that folder's collector header, `effects/base/effects.h` (or the relevant expansion's) — this is what makes the implementation reachable from the generated `bang_cards.cpp`; see the server module doc's explanation of the `vtable_build.h` two-phase macro trick for exactly why this include is what actually builds the vtable.
3. In the YAML, use the name registered by the `DEFINE_EQUIP(my_binoculars, ...)` macro:

```yaml
- name: MY_BINOCULARS
  signs:
    - A hearts
  image: 01_mybinoculars
  color: blue
  equip:
    - my_binoculars
```

`event_equip::on_enable` is called when the card is equipped (and the analogous, by-default-inherited `on_disable` when it's removed/disabled) — the framework automatically cleans up the listener when the card leaves play, thanks to the `event_card_key` system described in the architecture document.

---

## Case 4 — A reactive ability with deferred action (the "Bart Cassidy" pattern)

**Goal**: a character who, every time they take damage, automatically draws cards proportional to the damage taken — exactly *Bart Cassidy*. Useful as an example of an **event-driven ability with an asynchronous action**, more complex than a simple numeric modifier like Case 3.

Real implementation (`effects/base/bart_cassidy.h/.cpp`):

```cpp
// bart_cassidy.h
struct equip_bart_cassidy : event_equip {
    int ncards;
    equip_bart_cassidy(int ncards = 1): ncards{ncards} {}
    void on_enable(card_ptr target_card, player_ptr target);
};
DEFINE_EQUIP(bart_cassidy, equip_bart_cassidy)
```

```cpp
// bart_cassidy.cpp
void equip_bart_cassidy::on_enable(card_ptr target_card, player_ptr p) {
    p->m_game->add_listener<event_type::on_hit>({target_card, 1},
        [p, target_card, ncards=ncards](card_ptr origin_card, player_ptr origin,
                                          player_ptr target, int damage, effect_flags flags) {
            if (p == target) {
                target->m_game->queue_action([=]{
                    if (target->alive()) {
                        target_card->flash_card();
                        target->draw_card(damage * ncards, target_card);
                    }
                });
            }
        });
}
```

Key points to note, useful as a "recipe" for similar abilities:
- **`add_listener<event_type::on_hit>({target_card, 1}, ...)`** — registers on the general "a player took damage" event (not just from `Bang!`, but from any damage source), with **priority `1`** (to define the order in which it acts relative to other cards reacting to the same event, e.g. other players' *Barrel*).
- **`p == target`** — the listener receives the event for *anyone* taking damage; the check filters for the case where the card's owner is the one being hit.
- **`queue_action([=]{...})`** — the actual action (drawing cards) isn't executed immediately inside the listener, but **queued** as a low-priority request (`request_action`, default priority 0) rather than run synchronously. Since the `request_queue` resolves higher-priority requests first (see the priority-ordering discussion in the checklist below), this means the deferred action actually waits until every higher-priority pending request already in the queue has resolved — for example, if the hit came from a Gatling that's still working through multiple targets, or from a chain of hits triggering deaths that still need resolving, those all clear first, and Bart Cassidy's draw happens once the queue actually gets to it.
- **`target->alive()`** — a safety check: if the damage was lethal, nothing is drawn anyway (the player has died in the meantime).

The `ncards` constructor parameter allows the same implementation to be reused with different `equip_value`s (e.g. for "upgraded" variants in other expansions), similarly to how `weapon(3)` in YAML passes an integer to `weapon`'s `effect_value`.

---

## Case 5 — An effect generating an entire interactive request (the "Duel" pattern)

**Goal**: a card that doesn't just apply an immediate effect, but **starts a back-and-forth** between two players (respond with a Bang! or take damage, indefinitely) — the case of *Duel*. This is the highest level of complexity: a struct with `on_play` isn't enough; a **full-blown request** (`request_base`) queued in the engine's `request_queue` is needed.

Simplified structure (adapted from `effects/base/duel.cpp`):

```cpp
struct request_duel : request_auto_resolvable, interface_picking, respondable_with_bang, interface_escapable {
    player_ptr respond_to;

    // on every tick: checks immunity, sound effects, auto-resolve conditions
    void on_update() override { /* ... */ }

    // which cards in hand are "pickable" as a response (here: any Bang!)
    bool can_pick(card_ptr target_card) const override {
        return target_card->is_bang_card(target) && /* ... */;
    }

    // the player chose a Bang! card as a response: bounce the duel back to the other player
    void respond_with_bang() override {
        pop_request();
        target->m_game->queue_request<request_duel>(origin_card, origin, respond_to, target);
    }

    // no response possible / request resolved: the player takes damage
    void on_resolve() override {
        pop_request();
        target->damage(origin_card, origin, 1);
    }

    game_string status_text(player_ptr owner) const override { /* text for the status bar */ }
};

struct effect_duel {
    prompt_string on_prompt(card_ptr origin_card, player_ptr origin, player_ptr target, effect_flags flags = {});
    void on_play(card_ptr origin_card, player_ptr origin, player_ptr target, effect_flags flags = {}) {
        target->m_game->queue_request<request_duel>(origin_card, origin, target, origin, flags);
    }
};
DEFINE_EFFECT(duel, effect_duel)
```

What this example teaches:
- **`queue_request<request_duel>(...)`** queues the request in the `game_table`'s `request_queue` — from that point on, the engine blocks every other action until this request is resolved (`pop_request()`), exactly as described in §3.3 of the server architecture document.
- The **mixed interfaces** (`request_auto_resolvable`, `interface_picking`, `respondable_with_bang`, `interface_escapable`) are mixins that add reusable behavior: "auto-resolve after a timeout," "allow choosing a card from the hand," "allow responding by playing a Bang!," "allow discarding to escape" (for variants that permit it). They compose these capabilities instead of duplicating them per card.
- `on_prompt` (separate from `on_play`) handles **preventive validation on the bot/UI side** (e.g. suggesting the bot avoid challenging an ally, checking immunity) before the effect is even executed.

This is the right pattern whenever a card introduces **a new "mini-turn"** of interaction (duels, multiple choices, chained requests) instead of a single, immediate effect.

---

## Case 6 — Introducing a new *target type* (not just a new effect)

**Goal**: sometimes it's not the effect that's new, but **the shape of the target**. The real `adjacent_players` target (used by *Fanning* in *Valley of Shadows* — a Bang! against two players who are adjacent to *each other*, not to the origin) is a good example: the second player picked has to be within range of the *first* one picked, not of the origin.

If you need a genuinely new target, the work goes in **`target_types/<expansion>/`**, following the same `vtables.h` structure seen for effects — but note that `targeting_vtable` is stricter than the `effect`/`equip`/`modifier` vtables covered elsewhere in this guide: `build_targeting_vtable` (`cards/vtable_build.h`) only wraps `serialize_args` and `random_target` in an `if constexpr (requires {...})` fallback. **`get_error`, `on_prompt`, `add_context`, and `on_play` are called unconditionally, with no fallback** — every new target type has to implement all four, plus one of `possible_targets` (enumerate every valid candidate — the usual case) or `is_possible` (a bare yes/no check, for target types like `none` that carry no real value at all). Skip one of the mandatory four and it's a hard compile error, not a silently-missing feature.

One sharp edge worth knowing about `is_possible`: the generated `possible_targets` wrapper doesn't just skip enumeration when you use it — it yields exactly one `play_card_target{value_type{}}`, a **default-constructed** value, standing in for "the" target. That placeholder isn't inert: `possible_to_play.cpp`'s `check_recurse` calls `effect.add_context(origin_card, origin, target, ctx)` on every yielded candidate as part of ordinary enumeration, so `add_context` *will* be invoked with this default-constructed value during possibility-checking, not just during real execution. For `none` (`value_type = target_args::empty`) that's harmless, since a default-constructed empty struct is exactly the "no target" it means to represent — but for any target type whose `value_type` is a pointer (`player_ptr`, `card_ptr`) or similar, a default-constructed value is null, and `add_context` has to be written to tolerate that rather than assume it's always a real target. In practice this means: only reach for `is_possible` when your `value_type` is something that's genuinely meaningless/harmless in its default-constructed form, and write `add_context` (and `get_error`/`on_prompt`/`on_play`, called the same way once a real action is actually verified) defensively if there's any doubt.

In practice, though, a new target type rarely means writing all of that from scratch — the real `missed_and_same_suit` target (`target_types/legends/`) is a good model for the common case: it *inherits* from the existing `targeting_cards` and only overrides the three methods that actually need to change (`is_possible`, `random_target`, `get_error`), reusing `targeting_cards`'s enumeration, prompt, context, and execution logic unchanged. Here's a similarly-built example, closer to what many new target types actually look like: a brown card, **Sleight of Hand** — "discard any number of cards, no two of the same suit." Call the target type `distinct_suits`. Since "any number, confirmable, up to a limit" is already exactly what `targeting_max_cards` (`target_types/fistfulofcards/`) does, this one only has to add a single method on top of it:

```cpp
// target_types/base/distinct_suits.h
#ifndef __TARGET_TYPE_DISTINCT_SUITS_H__
#define __TARGET_TYPE_DISTINCT_SUITS_H__

#include "target_types/fistfulofcards/max_cards.h"

namespace banggame {

    struct targeting_distinct_suits : targeting_max_cards {
        using targeting_max_cards::targeting_max_cards;

        game_string get_error(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, const card_list &targets);
    };

    DEFINE_TARGETING(distinct_suits, targeting_distinct_suits)
}

#endif
```

```cpp
// target_types/base/distinct_suits.cpp
#include "distinct_suits.h"

namespace banggame {

    game_string targeting_distinct_suits::get_error(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, const card_list &targets) {
        MAYBE_RETURN(targeting_max_cards::get_error(origin_card, origin, effect, ctx, targets));
        for (size_t i = 0; i < targets.size(); ++i) {
            for (size_t j = i + 1; j < targets.size(); ++j) {
                if (targets[i]->sign.suit == targets[j]->sign.suit) {
                    return "ERROR_DUPLICATE_SUITS";
                }
            }
        }
        return {};
    }
}
```

A few things worth being precise about, since they're easy to get subtly wrong:
- **One override is enough here because the relational check is the *only* thing that differs.** `targeting_max_cards::get_error` already handles "is this a legal-sized selection of individually-valid cards" (including its own `confirmable` early-stop logic); this override just calls that first via `MAYBE_RETURN`, then adds the one new constraint on top. Nothing about enumeration, prompting, `add_context`, or execution needed to change, so nothing here re-declares them.
- **`is_possible` and `random_target` are inherited unchanged, and that's a real, honest trade-off, not a free lunch.** `targeting_max_cards::random_target` samples candidates without any awareness of suits, so a bot *can* occasionally end up trying a combination with two same-suited cards — which just costs it a rejected attempt and a retry (the same graceful failure mode described for bot prompts in Case 11), not a broken card. A more careful version would override `random_target` too, to only ever sample valid combinations; inheriting it as-is is a legitimate choice, just one worth making deliberately rather than by accident.
- **Field order/params still come from the constructor you inherit.** `using targeting_max_cards::targeting_max_cards;` pulls in its `(target_args::card args, int ncards = 0, bool confirmable = false)` constructor unchanged, which is also what fixes the YAML parameter list below.

Then reference it by name in YAML exactly like any other target — `distinct_suits(0, true)` means no fixed count and confirmable (per `targeting_max_cards`'s own constructor: `ncards = 0` skips the upper bound, `confirmable = true` allows stopping early, so this reads as "at least one, no explicit cap" — the suit constraint naturally caps it at 4 anyway):
```yaml
- name: SLEIGHT_OF_HAND
  color: brown
  effects:
    - discard distinct_suits(0, true) self | hand
```

Unlike Cases 3-5 (where updating the backend is enough), a new target type **almost always also requires a frontend change** — the client has no generic fallback for a target type it doesn't recognize, so skipping this step means the card is simply unplayable from the UI even though the server would accept it. Concretely, for `distinct_suits` above, three files need something added (see the frontend doc's §4.3 deep-dive on `TargetDispatch.ts` for how the pieces below actually fit together), and the same "reuse, override one thing" shape carries over to this side too — the real `max_cards` entries in `CardTarget.ts`/`TargetDispatch.ts` are the direct model, not something to write from a blank page:

1. **`bangweb/src/Scenes/Game/Model/CardTarget.ts`** — a new entry in `CardTargetTypes`, identical in shape to the real `max_cards` entry, since the underlying value (a growing list of cards plus a running cap) is exactly the same kind of thing:
```ts
distinct_suits: {
    value: { cards: Card[], max_cards: number },
    target: CardId[],
    effect: CardTargetArgs & { ncards: number, confirmable: boolean }
},
```

2. **`bangweb/src/Scenes/Game/Model/TargetDispatch.ts`** — a `reservedDispatch(...)` entry that's the real `max_cards` entry with exactly one line changed:
```ts
distinct_suits: reservedDispatch({
    isCardSelected: ({ cards }, card) => containsId(cards, card),
    appendCardTarget: (table, selector, { cards, max_cards }, effect, card) => ({ cards: cards.concat(card), max_cards }),
    isValidCardTarget: (table, selector, target, effect, card) =>
        target.cards.every(c => c.cardData.sign.suit !== card.cardData.sign.suit)
        && isValidCardTarget(table, selector, target, effect, card),
    isSelectionConfirmable: ({ cards }, effect) => effect.confirmable && cards.length !== 0,
    isSelectionFinished: ({ cards, max_cards }, effect) => cards.length === max_cards,
    confirmSelection: ({ cards }) => ({ cards, max_cards: cards.length }),
    buildAutoTarget: (table, selector, effect) => {
        let max_cards = getValidCardTargets(table, selector, effect).length;
        if (effect.ncards !== 0 && max_cards > effect.ncards) {
            max_cards = effect.ncards;
        }
        return { cards: [], max_cards };
    },
    generateTarget: ({ cards }) => mapIds(cards),
}),
```
Every line here is copied straight from the real `max_cards` dispatch except `isValidCardTarget`, which adds the same-suit rejection and then falls through to the ordinary shared `isValidCardTarget` helper (the one `max_cards` itself uses directly, unwrapped) for the ordinary filter check — the exact same "call through after adding one constraint" shape as the C++ side's `MAYBE_RETURN(targeting_max_cards::get_error(...))`.

3. **Any new string identifier** — our server-side `get_error` returns `"ERROR_DUPLICATE_SUITS"`; it needs a matching entry in `Locale/<every language>/GameStrings.tsx`, or it'll render as a raw, untranslated key if a player ever actually triggers it (see the full new-card checklist, §7).

With all three in place, the card behaves identically to any built-in one: `possible_to_play.cpp` and `TargetDispatch.ts`'s `isValidCardTarget` independently agree on which combinations qualify (both re-derived from the same underlying rule by hand, not shared code — worth keeping in sync deliberately if the rule ever changes on one side), the UI stops offering already-used suits as the selection grows, and the resulting `GameAction` serializes to the same `CardId[]` the server's `deserialize_target` expects.

---

## Case 7 — Stacking effects: cards that modify another card's resolution

**Goal**: make a new card change *how another card resolves*, without either card knowing about the other — the way real expansions layer independently-authored abilities on top of the base game's *Bang!*/*Missed!* exchange.

This is a different problem from every previous case. Cases 1-2 compose effects *within* a single card's own effect list. Case 4 reacts to a generic event (`on_hit`) that fires *after* something has already happened. Case 7 is about **hooking into the brief window where another effect is still being assembled**, so several cards can each nudge the same in-flight object before it's finalized — and have those nudges automatically add up.

The base game already exposes exactly this kind of hook for *Bang!*. When a `Bang!` is played, the engine builds a shared, mutable request object *before* queuing it:

```cpp
// effects/base/bang.cpp — effect_bang::on_play (simplified)
auto req = std::make_shared<request_bang>(origin_card, origin, target);
req->origin->m_game->call_event(event_type::apply_bang_modifier{ req->origin, req });
req->origin->m_game->queue_request(std::move(req));
```

`request_bang` (`effects/base/bang.h`) carries the fields that actually drive resolution:

```cpp
struct request_bang : request_auto_resolvable, interface_missable, ... {
    int bang_strength = 1;   // how many Missed! are needed to cancel this Bang!
    int bang_damage = 1;     // how much damage it deals if not cancelled
    ...
    void on_miss(card_ptr missed_card, effect_flags flags) override {
        if (--bang_strength == 0) {
            // only cancelled once bang_strength reaches 0
            ...
        }
    }
};
```

Because `call_event(apply_bang_modifier{...})` fires **after the request exists but before it's queued**, *any* card can register a listener that mutates `bang_strength`/`bang_damage` on the fly, and the base *Bang!*/*Missed!* logic (`on_miss`, decrementing `bang_strength` per card played) never has to know which cards did the mutating. Two real, independently-authored examples:

**Slab the Killer** — a permanent, *offensive* character passive (`effects/base/slab_the_killer.cpp`) — boosts Slab's own Bang!s, not Bang!s aimed at him:
```cpp
void equip_slab_the_killer::on_enable(card_ptr target_card, player_ptr target) {
    target->m_game->add_listener<event_type::apply_bang_modifier>(target_card,
        [target](player_ptr origin, shared_request_bang req) {
            if (target == origin) {
                ++req->bang_strength;   // Bang!s Slab plays now need 2 Missed! to cancel
            }
        });
}
```

**Aim** — a one-shot offensive card played by the attacker (`effects/valleyofshadows/aim.cpp`):
```cpp
void effect_aim::on_play(card_ptr origin_card, player_ptr origin) {
    origin->m_game->add_listener<event_type::apply_bang_modifier>(origin_card,
        [=](player_ptr p, shared_request_bang req) {
            if (p == origin) {
                ++req->bang_damage;                    // origin's next Bang! deals +1 damage
                origin->m_game->remove_listeners(origin_card); // ...but only once
            }
        });
}
```

Neither file references the other, and neither lives in the same expansion (Slab the Killer is a base character; Aim is from *Valley of Shadows*). Both check the same thing — "am I the one currently playing this Bang!" (`target == origin` in one, `p == origin` in the other). Yet if a player *who is Slab the Killer* plays *Aim* and then plays a *Bang!*, the single `request_bang` object created for that Bang! ends up with `bang_strength == 2` **and** `bang_damage == 2` — both modifications apply automatically, because both listeners independently reacted to the same `apply_bang_modifier` event, each recognizing the same player as the one currently attacking. This is what "stacking" means in practice: the combined behavior is an emergent sum of every listener currently registered, not something either card has to anticipate. (The same event is reused by several other cards across expansions — `sniper.cpp`, `bigfifty.cpp`, `buntlinespecial.cpp`, `doublebarrel.cpp`, `colorado_bill.cpp` — all of them independently adjusting `bang_strength`/`bang_damage` the same way.)

**The general recipe**, for any effect you want a new card to plug into:
1. Check whether the effect you want to interact with already fires a "modifier event" between constructing its request and queuing it (search the effect's header for an `event_type::apply_*_modifier`-style struct — `bang.h`, for instance, also documents `check_bang_target` for veto-style checks, and `on_missed` for reacting *after* a Bang! gets cancelled).
2. If it does, just `add_listener<that_event>(...)` in your new card's `on_enable` (permanent equip/character ability) or `on_play` (one-shot card, removing the listener immediately after use, as *Aim* does).
3. If the effect you want to extend doesn't expose such a hook yet, and you're not just adding an expansion card but changing a base mechanic, you'd need to add the event to the base effect yourself (see Case 12 for exactly what that involves) — but this is uncommon, since the built-in effects that are meant to be extensible (`Bang!`, `Duel`, damage, draw checks...) already expose one.
4. Nothing here needs a frontend change: the client never sees `bang_strength`/`bang_damage` directly — it only reacts to the resulting `game_update`s (damage dealt, a `request_status` asking for a Missed!, etc.), so stacking is entirely a backend-side concern.

---

## Case 8 — A card that must be played together with another (a new *modifier*)

**Goal**: a brown card, **Steady Aim**, played *before* a Missed!: "if the Missed! cancels a Bang!, deal 1 damage back to whoever Banged you." Unlike Case 7 (where two independent cards each quietly react to the same event), here the point is that the new card **can't be played on its own at all** — the client must collect it, then immediately require a specific follow-up card, and send both as one combined action.

Real modifiers like *Aim*'s `bangmod` (covered in the architecture doc) reuse the SAME modifier for every card that needs "must be followed by a Bang!." Steady Aim needs the mirror-image rule — "must be followed by a Missed!" — which doesn't exist yet, so this is a case where defining a brand-new `modifier_vtable` is the right call, modeled directly on `modifier_bangmod`:

```cpp
// effects/base/steady_aim.h
#ifndef __BASE_STEADY_AIM_H__
#define __BASE_STEADY_AIM_H__

#include "cards/card_effect.h"

namespace banggame {

    struct modifier_steadyaim {
        bool valid_with_equip(card_ptr origin_card, player_ptr origin, card_ptr target_card) {
            return false;                                          // never before an equip card
        }
        bool valid_with_modifier(card_ptr origin_card, player_ptr origin, card_ptr target_card) {
            return target_card->has_tag(tag_type::steadyaimmod);    // allow stacking with itself
        }
        bool valid_with_card(card_ptr origin_card, player_ptr origin, card_ptr target_card) {
            return target_card->pocket == pocket_type::player_hand
                && target_card->has_tag(tag_type::missedcard);      // must be a real Missed!
        }
    };
    DEFINE_MODIFIER(steadyaimmod, modifier_steadyaim)

    struct effect_steady_aim {
        void on_play(card_ptr origin_card, player_ptr origin);
    };
    DEFINE_EFFECT(steady_aim, effect_steady_aim)
}

#endif
```

```cpp
// effects/base/steady_aim.cpp
#include "steady_aim.h"
#include "effects/base/bang.h"
#include "game/game_table.h"

namespace banggame {
    void effect_steady_aim::on_play(card_ptr origin_card, player_ptr origin) {
        if (auto req = origin->m_game->top_request<request_bang>([&](const request_base &base) {
            return base.target == origin;
        })) {
            player_ptr attacker = req->origin;
            origin->m_game->add_listener<event_type::on_missed>(origin_card,
                [=](card_ptr missed_card, effect_flags flags, shared_request_bang req) {
                    if (req->target == origin) {
                        origin->m_game->remove_listeners(origin_card);
                        attacker->damage(origin_card, origin, 1);
                    }
                });
        }
    }
}
```

```yaml
- name: STEADY_AIM
  signs:
    - 9 spades
  image: 01_steadyaim
  color: brown
  modifier: steadyaimmod
  effects:
    - steady_aim
```

A few things worth noticing:
- `modifier_steadyaim` only defines the three `valid_with_*` checks — no custom `get_error` override, no `add_context`. Both are individually optional (probed with `requires`, exactly like every other vtable in this system); Steady Aim doesn't need to inject anything into the shared `effect_context`, so it simply doesn't implement `add_context` and the framework's default (a no-op beyond the automatic `contexts::modifier_card` bookkeeping) applies.
- `effect_steady_aim::on_play` runs as part of the normal chain-execution order described in the architecture doc: modifiers' own effects run *before* the terminal card, so at the point this fires, `request_bang` is still the pending top request — `top_request<request_bang>` finds it, and the listener it registers is guaranteed to see the Missed! that's about to be played immediately afterward.
- Nothing here needs a frontend change, for the same reason Case 3-5 didn't: the client already knows how to walk a `modifiers` chain generically (see the frontend doc's explanation of `getAllPlayableCards`) — it just needed the server to expose one more named modifier, and a card that references it.

---

## Case 9 — An effect that needs several different targets at once (a new *multi-target handler*)

**Goal**: a brown card, **Marksman**: "discard 2 cards from your hand, then deal 1 damage to a player within distance 2 — plus 1 extra damage for every Heart among the discarded cards." The two target groups (2 hand cards, 1 player) are easy to express as ordinary effects on their own — but the *damage amount* depends on which specific cards were discarded, which is exactly the situation an `mth` (multi-target handler) is for: no single independent effect entry can see across both groups, so the two need to be delivered to one function together.

```yaml
- name: MARKSMAN
  signs:
    - K clubs
  image: 01_marksman
  color: brown
  effects:
    - discard cards(2) self | hand
    - none player alive range_2
  mth_effect: marksman(0,1)
```

The two target groups are declared exactly like they would be on any other multi-effect card — `discard cards(2) self | hand` is a real, independently-filtered effect (the cards actually get discarded on its own `on_play`); `none player alive range_2` uses the placeholder `none` effect purely to get a validated, filtered player target with no behavior of its own. `mth_effect: marksman(0,1)` then says: take slots 0 and 1 of the flat target list (there's no leading gate like *Doc Holyday*'s `max_usages(1)` here, so indices start at 0) and hand them both to one handler:

```cpp
// effects/base/marksman.h
#ifndef __BASE_MARKSMAN_H__
#define __BASE_MARKSMAN_H__

#include "cards/card_effect.h"

namespace banggame {

    struct handler_marksman {
        void on_play(card_ptr origin_card, player_ptr origin, const card_list &discarded, player_ptr target);
    };

    DEFINE_MTH(marksman, handler_marksman)
}

#endif
```

```cpp
// effects/base/marksman.cpp
#include "marksman.h"
#include "game/game_table.h"

namespace banggame {
    void handler_marksman::on_play(card_ptr origin_card, player_ptr origin, const card_list &discarded, player_ptr target) {
        int amount = 1 + rn::count_if(discarded, [](card_ptr c) { return c->sign.is_hearts(); });
        target->damage(origin_card, origin, amount);
    }
}
```

A few things worth noticing:
- `handler_marksman` only implements `on_play` — `get_error` and `on_prompt` are just as optional here as they were for the modifier above; without a custom `get_error`, the handler imposes no extra joint constraint beyond what each slot's own filter already checked, and without `on_prompt`, the client falls back to whatever generic confirmation text the individual slots would produce.
- `const card_list &discarded` and `player_ptr target` are ordinary, strongly-typed C++ parameters — `mth_unwrapper` reads those exact types straight off `&handler_marksman::on_play`'s own signature and reconstructs them from the flat, type-erased target list at the positions given by `(0,1)`. Nothing about the indices or the reconstruction is hand-written elsewhere.
- This *had* to be an `mth` rather than two ordinary effects, because the damage amount reads the content of the discard group while acting on the player group. A simpler version of Marksman that just dealt a flat 1 damage regardless of what was discarded wouldn't need `mth` at all — that's just two independent effects, like *Sid Ketchum*'s discard-then-heal (Case 2). The moment one group's *content* has to influence what happens to another group, that's the signal an `mth` handler is actually earning its keep, rather than just being reached for out of habit.
- Once again, no frontend change is needed: the client walks Marksman's two target slots exactly like it would any other multi-effect card, unaware that a combined handler exists behind them (see the frontend doc's note on `mth` having no client-side counterpart at all).

---

## Case 10 — Steering which card a bot picks (`bot_rules`)

**Goal**: make sure bots play your new card at a sensible moment, without writing any AI logic — most of the time this is a **tagging decision**, not a code change.

Bots don't evaluate cards individually. `game::request_bot_play` (`game/bot_ai.cpp`) gathers every legal `(card, targets)` combination for the current player into a weighted pool, then `get_selected_node` picks one by walking a short, **ordered** list of generic predicates and taking the first one that matches anything still in the pool:

```yaml
# config/bot_info.yml
settings:
  in_play_rules:
    - repeat                              # keep playing the same card again if that's still legal
    - tag_value(tag_type::strong)         # then: anything tagged "strong"
    - equip                               # then: any equip card
    - tag_value(tag_type::button_color, 2) # then: a specific button-row tier
    - pocket(pocket_type::player_table)    # then: prefer playing from the table
    - pocket(pocket_type::player_hand)     # then: then from hand
    - pocket(pocket_type::player_character) # then: then a character ability
    - tag_value_not(tag_type::pass_turn)   # then: anything that isn't just "pass"
```
Each entry is a tiny predicate struct (`game/bot_rules.h`/`.cpp`) over a candidate card — `pocket`, `pocket_not`, `equip`, `repeat`, `tag_value`, `tag_value_not`, `do_nothing` — matched against generic, already-existing properties of the card (which pocket it's in, whether it's an equip, whether it carries a given tag, optionally with a specific value). Nothing here is per-card: it's one short global list, generated once from this YAML into a single `bot_info` object (`config/parse_bots.py`), consulted for every bot decision in the game.

So for a typical new card, "driving" bot behavior means picking the right tag, not touching this list at all. If your new card is a big, important one-shot play — say a brown card, **Last Stand**, that should be prioritized the way other high-impact plays are — just tag it the same way they are:
```yaml
- name: LAST_STAND
  color: brown
  effects:
    - damage players notself
  tags:
    - strong
```
That alone routes it into the `tag_value(tag_type::strong)` tier — the second-highest priority in `in_play_rules` — ahead of ordinary hand/table plays, with zero AI code written. This is why the predicate list stays so short and generic: card authors steer bots through the same tags that already describe the card for other purposes (rules text, filters, UI), not through a parallel bot-specific configuration per card.

The list only needs a **new predicate** when the property you need genuinely isn't reducible to an existing tag/pocket/equip check. The existing set has a gap, for instance: nothing lets a rule ask about a card's *color*. Adding one follows the exact same shape as the built-in ones:
```cpp
// game/bot_rules.h — alongside the existing DEFINE_BOT_RULE lines
DEFINE_BOT_RULE(color, rule_color, card_color_type color)
```
```cpp
// game/bot_rules.cpp
bool rule_color::operator()(card_node node) const {
    return node && node->card->color == color;
}
```
which YAML can then reference exactly like any other rule, e.g. `color(card_color_type::brown)`, in either `in_play_rules` or `response_rules`. In practice this is the rare path — the seven existing predicates, especially `tag_value`, cover almost everything a new card needs, because tags themselves are freely extensible from YAML with no C++ at all.

**Why it works**: `get_selected_node` (`bot_ai.cpp`) doesn't score cards numerically — it filters the candidate pool by each rule *in order* and stops at the first rule that matches anything (`rv::filter(node_set, rule)`), then picks uniformly at random among just those matches (`random_element`). A card that matches an earlier rule is effectively in a higher priority tier than one that only matches a later rule or none at all (in which case the final fallback is a uniform pick over everything remaining). Tagging your card is enough because the rules test generic properties every card already has — you're not writing a special case, you're placing your card into a bucket that already has a priority.

---

## Case 11 — Discouraging a specific decision, invisibly to human players (bot prompts)

**Goal**: a card that both humans and bots can play on any player — but a bot should avoid pointing it at the "wrong" kind of target, without restricting what a human is allowed to do.

The concrete example is a new card, **Comrade's Aid** — "choose a player, heal them 1 HP." A human might deliberately heal an opponent for social reasons in a hidden-role game; that's a legitimate choice. A bot doing the same thing is almost always just wasting the effect. One rule worth stating up front, since it's easy to get backwards: **`get_error` is only ever for genuine rule violations — something that's illegal for anyone, bot or human — never for bot-only heuristics.** Bots and humans play by exactly the same rules; a bot must always be *able* to make the same play a human could, however unwise. Steering a bot away from a bad-but-legal choice belongs in `on_prompt` instead, using one of the ready-made helpers in `game/prompts.h`:

```cpp
// effects/base/comrades_aid.h
#ifndef __BASE_COMRADES_AID_H__
#define __BASE_COMRADES_AID_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_comrades_aid {
        prompt_string on_prompt(card_ptr origin_card, player_ptr origin, player_ptr target) {
            MAYBE_RETURN(prompts::bot_check_target_friend(origin, target));
            return {};
        }
        void on_play(card_ptr origin_card, player_ptr origin, player_ptr target) {
            target->heal(origin_card, origin, 1);
        }
    };

    DEFINE_EFFECT(comrades_aid, effect_comrades_aid)
}

#endif
```
```yaml
- name: COMRADES_AID
  color: brown
  effects:
    - comrades_aid player alive
```
`prompts::bot_check_target_friend` (`game/prompts.cpp`) is short and revealing:
```cpp
game_string bot_check_target_friend(player_ptr origin, player_ptr target) {
    if (origin->is_bot() && !bot_suggestion::is_target_friend(origin, target)) {
        return "BOT_TARGET_FRIEND";
    }
    return {};
}
```
This is one of a small family of similarly-shaped `bot_check_*` helpers in `game/prompts.h` (the exact set will keep growing, so it's not worth enumerating), every one of which starts with the same `origin->is_bot()` guard: for a human player it always returns an empty string, so `on_prompt` reports no prompt and `Comrade's Aid` behaves like any ordinary card with no bot-specific logic at all. `bot_suggestion::is_target_friend`/`is_target_enemy` (`game/bot_suggestion.h`) are the shared primitives behind the guess — they estimate alignment from known/suspected roles and from actions the bot has observed so far, so the same two functions back every heuristic in this family rather than each card reinventing "is this player probably on my side." Its `game_string` return converts implicitly into the `prompt_string` that `on_prompt` expects (`prompt_string`'s constructor accepts a plain `game_string`, defaulting to priority 0), which is what lets `MAYBE_RETURN` work the same way here as it does anywhere else.

**Why it works**: this piggybacks entirely on how `verify_and_play` already treats a non-empty `on_prompt` result — it's turned into a `play_verify_results::prompt`. The bot's own decision loop, `execute_random_play` (`bot_ai.cpp`), treats a `prompt` result as "this specific candidate didn't work, try a different card or target from the pool instead," never as "ask for confirmation." So a bot that draws Comrade's Aid with an enemy as the only candidate target will simply abandon that attempt and try something else in the same pass — it never sees a message, never "confirms" anything, it just doesn't pick that option. A human sending the exact same action never triggers the check at all, since `is_bot()` is false for them, and — critically — nothing here ever stops a human from making that same "unwise" choice on purpose. Two consequences worth keeping in mind:
- **It's a soft discouragement, not a hard rule.** If, after `bot_info.yml`'s `max_random_tries` retries, a bot genuinely has no other legal option, `execute_random_play` sets `bypass_prompt = true` and forces the play through anyway rather than stalling forever — so this steers a bot away from bad choices when better ones exist, it doesn't forbid the bad choice outright.
- **This is why `get_error` is off-limits for this.** A non-empty `get_error` makes a play flatly illegal — for everyone, since the client won't even offer it and the server would reject it outright if forced. That's correct for an actual rule (a filter, an immunity, a turn restriction); it's wrong for "a bot probably shouldn't do this," because it would also silently take the choice away from human players, who are allowed to make it. `on_prompt` is the only one of the two that's specifically about *discouraging without forbidding*.

---

## Case 12 — Defining a brand-new event type

**Goal**: every hook used in Cases 3, 4, and 7 — `count_range_mod`, `on_hit`, `apply_bang_modifier` — already existed before those cards were written. This case is the other side of that: what do you actually do when the event you need *doesn't* exist yet, because you're starting a new mechanic from scratch rather than plugging into `Bang!`?

The genuinely good news here is that **event types need no registration macro at all** — unlike effects, targets, modifiers, and MTH handlers, there's no `DEFINE_EVENT`. Any plain aggregate struct (public fields, no user-declared constructor — ordinary C++ aggregate rules) is automatically usable as an event:
```cpp
template<typename T>
concept event = requires (const T &value) { reflect::to<std::tuple>(value); };
```
`reflect::to<std::tuple>` (the vendored compile-time reflection library under `external/reflect`) is what makes this work: it turns any aggregate into a `std::tuple` of its fields, in declaration order, at compile time — no boilerplate, no name string, nothing to look up at runtime. `call_event`/`add_listener` are then just ordinary function templates parameterized on your struct's C++ type directly (`typeid(T)` is used as the internal key), not on a YAML-facing name — so there's no equivalent of `GET_EFFECT`/`TARGET_TYPE` to write.

Say you're adding a new mechanic — a brown card, **Buckshot**: "target discards a card at random from their hand (not damage)." You'd like other future cards to be able to increase how many cards it makes them discard, the same way *Aim*/*Slab the Killer* stack on `apply_bang_modifier`. Since this isn't a Bang!, there's no existing hook for it — you declare one, right alongside the effect that owns it:
```cpp
// effects/base/buckshot.h
#ifndef __BASE_BUCKSHOT_H__
#define __BASE_BUCKSHOT_H__

#include "cards/card_effect.h"

namespace banggame::event_type {
    struct apply_buckshot_modifier {
        player_ptr target;
        nullable_ref<int> ncards;
    };
}

namespace banggame {
    struct effect_buckshot {
        void on_play(card_ptr origin_card, player_ptr origin, player_ptr target);
    };
    DEFINE_EFFECT(buckshot, effect_buckshot)
}

#endif
```
```cpp
// effects/base/buckshot.cpp
#include "buckshot.h"
#include "game/game_table.h"

namespace banggame {
    void effect_buckshot::on_play(card_ptr origin_card, player_ptr origin, player_ptr target) {
        int ncards = 1;
        origin->m_game->call_event(event_type::apply_buckshot_modifier{ target, ncards });
        for (int i = 0; i < ncards && !target->m_hand.empty(); ++i) {
            target->discard_card(random_element(target->m_hand));
        }
    }
}
```
A future card can now stack onto this exactly the way *Aim* stacks onto `apply_bang_modifier`, with no changes to `buckshot.h`/`.cpp` at all:
```cpp
origin->m_game->add_listener<event_type::apply_buckshot_modifier>(origin_card,
    [=](player_ptr target, int &ncards) {
        if (/* whatever condition */) ++ncards;
    });
```
A few things worth being precise about:
- **Field order is the contract.** `reflect::to<std::tuple>` unpacks fields positionally, and `std::apply` binds them positionally to the listener's parameters — the listener's parameter list has to match the struct's field order and types (by value, reference, or const reference, whichever it actually declares), not the field names. Get the order wrong and it's a hard compile error at the `add_listener` call site (the `applicable_as_event` concept simply won't be satisfied), not a silent bug — which is a nice property, but still means the field order is doing real, load-bearing work.
- **`nullable_ref<T>`** (`utils/misc.h`) is why `ncards` is declared as `nullable_ref<int>` in the struct but received as a plain `int&` in every listener — it's a thin, default-constructible wrapper around a `T*` that converts to/from `T&` implicitly. It's a workaround for a limitation of the vendored `reflect` library (`external/reflect`, [qlibs/reflect](https://github.com/qlibs/reflect)) itself: its aggregate-reflection technique can't handle a struct that actually contains a genuine reference (`T&`) member, so event fields meant to be writable by listeners are declared as `nullable_ref<T>` instead, and it converts back and forth transparently at the call site. This is a library-specific workaround, not a fundamental language limitation — with C++26's native reflection this need goes away entirely, so `nullable_ref` is very likely a temporary fixture of this codebase rather than a permanent pattern to imitate elsewhere.
- **`result_type` is opt-in**, and only needed if you want listeners to *answer* something rather than just observe or accumulate. Omit it entirely (as `apply_buckshot_modifier` does) for a pure notification/accumulator event. Declare it — as `check_play_card`/`check_bang_target` do, with `result_type = game_string` — when you want the *first listener with a non-empty answer* to short-circuit the rest: any type convertible to `bool` works (`game_string`/`prompt_string` empty-is-falsy, plain `bool`, `std::optional<T>`), and that's exactly the same "truthy answer wins, empty keeps asking" convention already used everywhere else in this system (`get_error`, `on_prompt`), just generalized to an arbitrary event instead of a fixed vtable slot.
- **Where to declare it** matters for discoverability, not correctness: general-purpose events reused across many otherwise-unrelated cards belong in the shared `cards/game_events.h` (`on_hit`, `on_turn_end`...); an event that's conceptually owned by one specific mechanic belongs in that mechanic's own header, the way `apply_bang_modifier`/`on_missed`/`check_bang_target` all live in `bang.h` itself, or `check_equipped_green_card` lives in `dodgecity/ruleset.h`. Either way it's just a struct declaration — there's nothing to add to `vtable_build.h`, `effects.h`, or any collector header beyond what Buckshot's own `.h` file already needed for `DEFINE_EFFECT`.
- **Priority and ordering are inherited for free.** A brand-new event type goes through the exact same `add_listener`/`call_event` machinery as every existing one, so it automatically gets the same dispatch ordering already covered in the checklist (§4): higher `event_card_key.priority` fires earlier, `card->order` breaks ties. Nothing about defining a new event changes that — it's generic infrastructure, not something tied to any specific event.

---

## Case 13 — Letting a modifier reach into the card it precedes, via `effect_context`

**Goal**: `mth` (Case 9) combines several target groups *within one card's own effect list*. This case is the other direction: a **modifier** (Case 8) that needs to change how the **terminal card** behaves — two entirely separate cards, registered independently, that only interact because they happen to be played together in one chain. (The full mechanism behind `effect_context` — how context types are declared, and the `serialize_context` opt-in that controls whether one reaches the client — is covered in the server module doc; this is the worked example.)

The real card is *Bell Tower* (Armed & Dangerous): "ignore distances for the ranged card that follows." Its YAML:
```yaml
- name: BELL_TOWER
  color: orange
  modifier: belltower
  effects:
    - none self_cubes(1)
```
```cpp
// effects/armedanddangerous/belltower.h
struct modifier_belltower {
    bool valid_with_card(card_ptr origin_card, player_ptr origin, card_ptr playing_card) {
        return playing_card->has_tag(tag_type::ranged_effect);
    }
    void add_context(card_ptr origin_card, player_ptr origin, effect_context &ctx);
};
DEFINE_MODIFIER(belltower, modifier_belltower)

namespace contexts {
    struct ignore_distances {
        struct serialize_context{};
    };
}
```
```cpp
// belltower.cpp
void modifier_belltower::add_context(card_ptr origin_card, player_ptr origin, effect_context &ctx) {
    ctx.add(contexts::ignore_distances{});
}
```
That flag means nothing on its own — it only matters because the actual distance check, in `game/filters.cpp`, was written to look for it:
```cpp
static bool check_distance(const_player_ptr origin, const_player_ptr target, const effect_context &ctx, int range) {
    if (range == 0) return false;
    if (origin->check_player_flags(player_flag::ignore_distances)) return true;
    if (ctx.contains<contexts::ignore_distances>()) return true;
    ...
}
```
`filters.cpp` even `#include`s `belltower.h` directly, just for that one struct — worth being upfront about, since it's the important caveat to this pattern: defining a new context type costs nothing (a plain struct, no macro), but *making something else respect it* is a real, deliberate change to whatever shared code needs to care. Contrast this with the generic context types (`contexts::selected_card` and friends) that the base target types already populate for every card automatically — those are "free"; a brand-new cross-cutting flag like `ignore_distances` is not, and required editing `filters.cpp` itself to wire in.

---

## Summary: which case applies to my card?

| If the card... | You need to touch... |
|---|---|
| Reuses existing effects and targets, even combined | Only YAML (Cases 1-2) |
| Modifies a continuous numeric value (range, damage, distance) via an engine event | A new `effect`/`equip` in C++, no request (Case 3) |
| Reacts to an event with an action (drawing, discarding, gaining something) in a deferred way | A new `equip` with `queue_action` (Case 4) |
| Starts a multi-step exchange/interaction between players | A new `request_base` queued in `request_queue` (Case 5) |
| Requires a target shape not expressible with `player`/`players`/`card`/`cards` | A new `target_type` **+ frontend update** (Case 6) |
| Changes how *another* card's effect resolves (strength, damage, cancellation...) | Hook the target effect's existing modifier event, e.g. `apply_bang_modifier` (Case 7) |
| Can't be played on its own — it must be followed immediately by a specific kind of card | A new `modifier_vtable` via `DEFINE_MODIFIER` (Case 8) |
| Needs several differently-filtered targets delivered to *one* combined function, especially when one group's content affects what happens to another | A new `mth_vtable` via `DEFINE_MTH` (Case 9) |
| Should be prioritized (or deprioritized) by bots relative to other legal plays | The right `tags:` entry, matched against `config/bot_info.yml`'s existing rules (Case 10) |
| Is legal for anyone, but a bot should usually avoid a specific target/decision | A bot-only `on_prompt` check via `prompts::bot_check_*` (Case 11) |
| Starts a mechanic other cards should be able to hook into, and no existing event fits | A new plain aggregate struct under `event_type::`, no macro needed (Case 12) |
| Is a modifier that needs to change how the terminal card it precedes behaves | A new `effect_context` type, read back by whatever shared code needs to respect it (Case 13) |

In every case involving C++, the final step is always the same: **register the implementation with the appropriate macro** (`DEFINE_EFFECT`/`DEFINE_EQUIP`/`DEFINE_MODIFIER`/`DEFINE_MTH`/`DEFINE_TARGETING`), include the new file from its folder's collector header (`effects/<expansion>/effects.h` or `target_types/<expansion>/target_types.h`) so it's reachable from the generated `bang_cards.cpp`, reference it by name from the YAML, and — if anything user-visible changes (a new target, a new log message) — update the corresponding translations in `Locale/*/Cards.tsx` / `GameStrings.tsx` on the `bangweb` side. The one exception is Case 12 itself: a new event type is just a struct declaration, nothing to register anywhere.

---

## Checklist: everything a new card might need

Your list (YAML definition, effect struct, `.h`/`.cpp` file, `effects.h` + `CMakeLists.txt`, request class, bot checks, immunity checks, checking other cards that might interact with it) covers the core of it. Verified against the code, here's the full picture with the gaps filled in — mainly: **deciding the card's shape before writing anything**, **priority ordering against every other listener/request on the same event or queue**, and the frontend/i18n half that's easy to forget entirely since the server runs fine without it.

**1. Decide the card's shape** *(do this first — it determines most of what follows; see the summary table above for which case applies)*
- [ ] Which list does it belong to: `effects` (played on your turn), `responses` (played reacting to a request), `equip_effects` (equip on-attach), or more than one?

**2. YAML definition** (`config/sets/<expansion>.yml`)
- [ ] `name`, `signs` (or omit for characters/equip-once cards), `image`, `color`, correct deck section (`main_deck`/`character`/`hidden`/etc. — this also decides which "expansion strategy" `parse_bang_cards.py` applies, e.g. one entry per sign vs. a fixed `count`).
- [ ] `effects`/`responses`/`equip`/`equip_effects` referencing the effect name(s) you'll implement (or reusing existing ones — Cases 1-2).
- [ ] `modifier`/`modifier_response` if Case 8 applies; `mth_effect`/`mth_response` if Case 9 applies.
- [ ] `tags:` — see point 5 below; this is usually the most consequential line in the whole file, not an afterthought.

**3. Server C++ implementation**
- [ ] New `.h`/`.cpp` under `effects/<expansion>/` (or `target_types/<expansion>/` for a new target type), with the struct registered via `DEFINE_EFFECT`/`DEFINE_EQUIP`/`DEFINE_MODIFIER`/`DEFINE_MTH`/`DEFINE_TARGETING`/`DEFINE_RULESET` as appropriate.
- [ ] A `request_base` subclass (`game/request_queue.h`) only if the card starts a multi-step interaction rather than resolving immediately (Case 5) — most cards don't need one.
- [ ] For an equip card with an ongoing passive: `on_enable`, and `on_disable` if the default cleanup (automatic listener removal via `event_card_key` when the card leaves play) isn't enough — e.g. if `on_enable` did more than register a listener (changed some other persistent state) and that also needs explicit reverting.
- [ ] File added to the folder's `CMakeLists.txt` (`target_sources`), and included from that folder's collector header (`effects/<expansion>/effects.h` or `target_types/<expansion>/target_types.h`) — this second step is what actually gets the vtable *built*, not just declared (see the module doc's explanation of the `vtable_build.h` two-phase macro trick if a card ever compiles but links with an undefined-symbol error).

**4. Listener and request priority — check ordering against every other card reacting to the same thing**
If your new effect registers an event listener or queues a request, it's joining a pool of *other* cards' listeners/requests on the same event or queue — and there are two genuinely separate priority mechanisms here, with **opposite conventions**, both worth checking against what already exists before picking a number:
- [ ] **Event listeners** (`add_listener<event_type::X>({card, priority}, ...)`, `game/event_map.h`): for the same event type, a **higher** `priority` value fires **earlier**; ties are broken by `card->order` (the order cards actually entered play — not something you choose). This only needs a deliberate, non-default value when your listener must fire before or after a *specific* other card's listener on the same event regardless of table order. Real `on_hit` listeners alone span a wide, deliberately-chosen range — Bart Cassidy at 1, El Gringo at 2, Hawken/Shotgun at 3, Bounty at 4, Soundance Kid/Steve Tengo at 5, a Goldrush global rule at 6, a Stick of Dynamite global rule at 20, and several others left at the implicit default of 0 — precisely because each needed to slot into a specific position relative to the others. Grep every existing `add_listener<event_type::YourEvent>` before adding a new one on that same event, and pick a number that lands where you actually need it, not an arbitrary one.
- [ ] **Queued requests** (`queue_request<T>(..., priority)`, default 100 from `request_base`; `queue_action(fn, priority)`, default 0): here it's the opposite — a **higher** priority value resolves **earlier** (pops first from the `request_queue`), and ties are broken by insertion order (FIFO), not card order. *Paul Regret*, for example, deliberately queues its forced discard at priority 210 — specifically higher than the ambient Bang!'s default 100 — so it resolves before the Bang! response does; `queue_action`'s low default of 0 exists precisely so deferred bookkeeping (like Bart Cassidy's post-hit draw) naturally waits behind any genuinely pending request unless you raise it. If your new request needs to jump ahead of or wait behind something specific, check that thing's actual priority value rather than assuming the default ordering is fine.

**5. Immunity**
- [ ] Decide whether your card is meant to respect Bang!-avoidance-style immunity (Jourdonnais and similar) — this isn't automatic just because an effect deals damage or targets a player; it only applies where a card's own logic calls for it.
- [ ] If your card is meant to be dodgeable the way a Bang! is, add the check explicitly: a hard block via `target->immune_to(origin_card, origin, flags)` (as `request_bang::on_update` does), and/or a softer, prompt-based warning via `prompts::prompt_target_immunity(origin_card, origin, target, flags)` in `on_prompt` (which also doubles as a bot-only nudge, since it fires for bots even when the human-facing option is off).

**6. Bot behavior** (Cases 10-11)
- [ ] Tag it so it lands in the right `in_play_rules`/`response_rules` tier (Case 10) — only add a new `DEFINE_BOT_RULE` if the property genuinely isn't reducible to an existing tag/pocket/equip check.
- [ ] If it's a card any player *could* legally point at the wrong target, but a bot usually shouldn't, add a bot-only `on_prompt` check via `prompts::bot_check_*` (Case 11) rather than restricting human players — never `get_error` for this, since that would make the play illegal for humans too, not just discouraged for bots.

**7. Frontend / localization** (`bangweb`) — the server compiles and runs fine without any of this; it just won't be legible to a real client
- [ ] Card image asset in `public/cards/` matching the YAML's `image:` key.
- [ ] Name + description entry in `Locale/<every language>/Cards.tsx`.
- [ ] Any *new* string identifier your effect introduces — `ERROR_*`/`LOG_*`/`PROMPT_*`/`STATUS_*`/`BOT_*` passed to a `game_string`/`prompt_string` constructor — needs a matching entry in `Locale/<every language>/GameStrings.tsx`, or it'll render as a raw, untranslated key client-side. `scripts/find_game_string.sh` (a `clang-query`-based scan over the whole `src/` tree) exists specifically to enumerate every such identifier in use, which is useful for checking you haven't missed one.
- [ ] Only if Case 6 applied: the corresponding entry in `CardTarget.ts`'s `CardTargetTypes` and validation logic in `TargetDispatch.ts`.
- [ ] A new expansion-level tag/rule (not a single card) also needs the `ExpansionType` union and lobby checkbox described in the expansion-addition guide.

**8. Verification**
There's no automated test suite in this repository (no `tests/` directory) — verification is manual: build the server, start a local lobby with the relevant expansion enabled, and play the card through a real game (including against a bot, to exercise points 6-7 above). Getting your specific new card into hand without waiting to draw it naturally is one of the more tedious parts of that — for exactly this reason, starting the server with `--cheats` (`server_options::enable_cheats`) enables a `/give <card_name>` chat command in an active game (`game::get_game_commands`, `net/manager.cpp`), letting you hand yourself the exact card you're testing on your own turn. The `AllCards` gallery page (`bangweb`, served from `GET /cards/:deck`) is a fast way to sanity-check that the name, image, and description you added actually resolve correctly without needing to start a match.
