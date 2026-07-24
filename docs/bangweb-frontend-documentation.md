# Module documentation — bangweb (frontend)

Repository: [bangsalvoserver/bangweb](https://github.com/bangsalvoserver/bangweb)
Language: **TypeScript + React 18**, build tool **Vite**, styling **Tailwind CSS**
Communication with the backend: **WebSocket** (JSON protocol) + a few auxiliary **HTTP REST** endpoints

This document describes the architecture of Bang!'s web client and, above all, **how it interfaces with the** [`bang-server`](https://github.com/bangsalvoserver/bang-server) **server** described in the previous document. The two projects share the same implicit "contract": every network-protocol structure on the server side (`net/messages.h`) and every card/target type (`cards/`, `target_types/`) has an **almost identical TypeScript counterpart** on the client side — it's worth reading the two documents side by side.

---

## 1. Overview

```
bangweb/
├── index.html / tracking.html / all_cards.html   Vite entry points (multi-page app)
├── src/
│   ├── App.tsx                 Root component: selects the current "scene"
│   ├── index.tsx                React bootstrap
│   ├── tracking.tsx              Bootstrap for the statistics page (/tracking.html)
│   ├── all_cards.tsx              Bootstrap for the card-gallery page (/all_cards.html)
│   ├── Model/                   Global application state and network protocol
│   ├── Scenes/                  The app's "screens" (Home, Lobby, Game, etc.)
│   │   └── Game/                 The game screen — by far the largest and most involved part
│   │       └── Model/             The client "engine": table state, target-selection state machine
│   ├── Components/                Reusable UI components
│   ├── Locale/                    Internationalization system (5 languages)
│   └── Utils/                     Generic hooks and utilities
└── public/                      Static assets (card images, sounds, fonts)
```

The organizing idea behind the whole client, and the thread running through every section below, is that it **mirrors the server as closely as possible and pushes decision-making server-side wherever it can**, rather than re-implementing game logic independently. Concretely: the server precomputes entire modifier chains and attaches them to each playable card rather than making the client work out compatibility itself (§4.3); `mth` handlers have no client-side equivalent at all, because the server never offers a combination the handler would reject; even the client's own target-validation logic (`TargetDispatch.ts`) is explicitly "optimistic" — a UX convenience for greying out invalid clicks, never the actual authority, since `play_verify.cpp` can and does reject anything regardless of what the client thought was valid. The payoff is that most new server-side content (a new card reusing existing effects and targets) needs no client change at all, and the client stays thin enough that its own state machine (`TargetSelector.ts`) is close to the only genuinely independent logic in the whole app.

The app is a **finite-state SPA**: `App.tsx` doesn't use a traditional router, but a single reducer (`sceneReducer`) that represents the entire navigation as a state machine (`home → loading → waiting_area → lobby → game`), driven by messages received from the server over WebSocket.

Besides the main web app, the project builds **two satellite pages** (served from `tracking.html` and `all_cards.html`, with their respective `tracking.tsx`/`all_cards.tsx` entry points): a server-statistics dashboard and a browsable gallery of all cards, both fed by HTTP endpoints on the server (see §5).

---

## 2. Module `Model/` — Global state and network protocol

This is the layer that **directly mirrors** the server's `net/` module.

| File | Role | Server counterpart |
|---|---|---|
| `ClientMessage.ts` | The discriminated union `ClientMessage` with all messages the client can send: `connect`, `user_set_name`, `user_set_propic`, `lobby_make`, `lobby_game_options`, `lobby_join`, `lobby_leave`, `lobby_chat`, `lobby_return`, `user_spectate`, `game_start`, `game_rejoin`, `game_action`. | `net/messages.h` → `client_message` (`std::variant<client_messages::*>`) |
| `ServerMessage.ts` | The discriminated union `ServerMessage`: `ping`, `client_accepted`, `lobby_error`, `lobby_update`, `lobby_entered`, `lobby_game_options`, `lobby_removed`, `lobby_user_update`, `lobby_kick`, `lobby_chat`, `game_update`, `game_started`. Each variant has the same fields as the corresponding C++ struct (e.g. `LobbyValue` ≡ `server_messages::lobby_update`). | `net/messages.h` → `server_message` |
| `UseWebSocket.ts` | A low-level hook that opens/closes the browser's native `WebSocket`, JSON-serializes (`JSON.stringify`) outgoing `ClientMessage`s and deserializes (`JSON.parse`) incoming `ServerMessage`s, exposing a connection state (`initial`/`connected`/`disconnected` with a code and optional reason). | `net/wsserver.cpp` (the `ws("/")` endpoint) |
| `UseBangConnection.ts` | The main "orchestrator" hook: opens the connection, handles the rehandshake (`connect` with the saved `session_id` — see rejoin), acts as a **central dispatcher** routing each `ServerMessage` type to either the scene reducer (`sceneDispatch`) or the game channel (`gameChannel`), automatically handles `ping`→`pong`, and implements **reconnection retry** (1s timeout if the disconnection wasn't "clean", code ≠ 1000). |
| `SceneState.ts` | The navigation reducer (`sceneReducer`) and the `SceneState` type (`home`/`loading`/`waiting_area`/`lobby`/`game`, with an optional `error`). Receives `SceneUpdate`s generated by `UseBangConnection` in response to server messages. |
| `AppSettings.ts` | Persistence of user preferences in `localStorage`/`sessionStorage` (username, avatar, language, default game options, `session_id` for rejoin, etc.), via the generic `UseLocalStorage.ts` hook. |
| `Env.ts` | Reads Vite environment variables (`VITE_BANG_SERVER_URL`, `VITE_LANGUAGE`, `VITE_DISCORD_LINK`) and derives the auxiliary HTTP URLs (`.../tracking`, `.../cards`, `.../image`) from the WebSocket URL. |

### The message cycle in detail (`UseBangConnection.ts`)

```
WS opened ──► client: connect{username, propic, session_id}
                        │
    server: client_accepted{session_id} ──► save session_id, go to "waiting_area"
    server: lobby_update / lobby_removed ──► update lobby list
    server: lobby_entered ──► clear gameChannel, go to "lobby"
    server: lobby_user_update / lobby_chat ──► update lobby state
    server: game_started ──► go to "game"
    server: game_update ──► forwarded to gameChannel (consumed by the game scene)
    server: ping ──► client: pong  (keep-alive)
```

If the connection drops abnormally while the user has an active session (`sessionId` present in `AppSettings`), the client shows a "disconnected" error and **automatically attempts reconnection** after 1 second, reusing the same `session_id` — this is what enables **rejoin** on the server side (`client_messages::connect` with a non-null `session_id` causes the existing `game_session` in `m_sessions` to be found again, and, if applicable, the associated lobby/match via `game_rejoin`).

---

## 3. Module `Scenes/` — The application's screens

| Scene | Main files | Corresponds to... (server side) |
|---|---|---|
| **Home** | `Home/Home.tsx` | The initial screen: entering username/avatar, connect button. No network traffic beyond the eventual `connect`. |
| **Loading** | `Loading/Loading.tsx` | The waiting screen shown during the initial connection or a reconnection attempt. |
| **WaitingArea** | `WaitingArea/WaitingArea.tsx`, `LobbyElement.tsx` | The list of available lobbies (`lobby_update`/`lobby_removed`), with `lobby_make`/`lobby_join` actions. |
| **Lobby** | `Lobby/Lobby.tsx`, `GameOptionsEditor.tsx`, `LobbyUser.tsx` | The pre-game waiting room: user list (`lobby_user_update`), chat (`lobby_chat`), the `GameOptions` editor (mirroring `game/game_options.h`, sent via `lobby_game_options`), the `game_start` button. |
| **Game** | `Game/GameScene.tsx` + `Game/Model/*` + `Game/Pockets/*` + `Game/Animations/*` | The heart of the app: rendering the game table and handling all interactions. See §4. |
| **AllCards** | `AllCards/AllCards.tsx` | A browsable gallery: downloads (`GET /cards/:deck`) an entire expansion's deck over HTTP and shows its localized illustrations/descriptions — useful for looking up cards outside a match. |
| **Tracking** | `Tracking/Tracking.tsx` | A dashboard with charts (`chart.js`/`react-chartjs-2`) of the server's historical statistics (player/lobby counts over time), read from `GET /tracking` — the same table populated server-side by `net/tracking.cpp` via SQLite. |

`Header.tsx`, `OverlayButtons.tsx`, `ErrorPopup.tsx`, `LanguageMenu.tsx`, `UserMenu.tsx` (in `Components/`) are frame elements present on every scene (top bar, error popups, language/settings selection).

---

## 4. Module `Scenes/Game/` — The client-side state engine

This is the largest and conceptually most interesting part: it **replicates the server's validation logic** client-side (`play_verify.cpp`/`possible_to_play.cpp`/`target_types/`) to offer a responsive interface without having to query the server on every click.

### 4.1 State representation (`Model/GameTable.ts`, `Model/GameUpdate.ts`)

- **`GameTable`** — the TypeScript mirror of the server's `game_table`: `players`/`cards` maps indexed by id, `pockets` (the same piles as on the server: `main_deck`, `discard_pile`, `shop_deck`, `train`, `stations`, `feats`, etc.), the current animation state, `train_position`, game flags (`Set<GameFlag>`).
- **`Card`/`KnownCard`** — a card can be "known" (with a full `CardData`: name, image, effects) or "unknown" (only its owning deck is known, e.g. a face-down card in an opponent's hand) — this mirrors the partial information visibility that the server sends selectively (`update_target`/`player_set` in `game_net.h`).
- **`GameUpdate`** (in `Model/GameUpdate.ts`) — a discriminated union **virtually identical** to the server's `game_update.h`: `add_cards`, `remove_cards`, `move_card`, `player_hp`, `player_show_role`, `switch_turn`, `add_tokens`, `move_train`, `deck_shuffled`, `show_card`/`hide_card`, `tap_card`, `flash_card`, `request_status`, `status_ready`, `game_log`, `game_prompt`, `play_sound`, etc. Many variants include a `duration` field (`Milliseconds`), computed by the server from `game_options` (e.g. `damage_timer`, `duration_coefficient`) to sync the pace of animations with the time the game engine expects to elapse before the next update.
- **`GameString`/`FormatArg`** — an exact mirror of `cards/game_string.h`: a message is `{format_str, format_args}`, where each argument is typed (`integer`/`card`/`player`), resolved client-side through the localization system (§4.4) rather than being an already-formatted string — so the same server response can be shown in 5 different languages.

### 4.2 The "update engine" (`Model/UseGameState.ts` + `Model/GameTableReducer.ts`)

The server sends an **ordered sequence** of `game_update`s for every (even minor) change in game state. The client applies them **one at a time**, respecting the included `duration`s, via a **delayed queue** mechanism (`delayDispatch` in `UseGameState.ts`):

1. Every `GameUpdate` arriving from the `gameChannel` is enqueued into `gameUpdates`.
2. `handleNextUpdate` processes the queue until it hits an update with `duration > 0`: in that case it applies the update immediately (e.g. `move_card`) and schedules, via `setTimeout`, the corresponding "animation-end" update (e.g. `move_card_end`, which clears the transition state), then resumes processing the queue.
3. This reproduces **exactly the same pauses/animations perceived on the server** (card movement, flipping, damage, deck shuffling...), because the durations are dictated by the server itself (`game_options`), not by hard-coded constants in the client.
4. `GameTableReducer.ts` contains the pure logic (immutable functions) that, given the current state and a single update, produces the new `GameTable` — each case (`add_cards`, `move_card`, `show_card`, `exchange_card`, `add_tokens`, `move_train`...) manipulates the piles (`pockets`) and maps (`cards`/`players`) declaratively.
5. "Synthetic" updates (`*_end`, not present in the server protocol but generated locally by `UseGameState`) close the animation transition started by the real update.

### 4.3 Target selection (`Model/TargetSelector.ts`, `Model/TargetDispatch.ts`, `Model/CardTarget.ts`)

This is the subsystem that **mirrors the backend's `target_types/` module**, one to one:

- **`CardTarget.ts`** defines `CardTargetTypes`, a map from target-type name to `{value, target, effect}`, using **exactly the same names** as the folders/implementations in the server's `target_types/`: `none`, `player`, `conditional_player`, `adjacent_players`, `player_per_cube`, `card`, `random_if_hand_card`, `extra_card`, `players`, `cards`, `max_cards`, `bang_or_cards`, `card_per_player`, `missed_and_same_suit`, `cube_slot`, `move_cube_slot`, `select_cubes*`, `self_cubes`. Each entry specifies: the type of the "value" shown in the UI (e.g. a list of candidate cards), the serialized type to send to the server (`target`, e.g. a `CardId` or an array of them), and the effect's arguments (`effect`, e.g. `player_filter`/`card_filter`, analogous to C++ `target_args`).
- **`TargetDispatch.ts`** (the largest file in the Model module) — see the dedicated breakdown right after this list for how it's actually built and what to add when introducing a new target type.
- **`TargetSelector.ts`** implements the **finite-state machine** for game interaction (documented with an ASCII diagram in the source): modes `start → preselect → modifier → middle → target → finish`. It manages: the selected main card (`selection`), any modifier cards played beforehand (`modifiers`, e.g. *Aim*, which must precede a Bang!), any server-imposed "preselection" (requests tagged `preselect`, e.g. the "Pick" card that must be chosen automatically), and confirmation prompts (`GamePrompt`: `yesno` or `playpick`). See below for a full explanation of the `modifier`, `preselect`, and `playpick` states.

#### `TargetDispatch.ts` in depth, and what a new target type needs here

Every target type needs up to ten small functions — `isCardSelected`, `isValidCardTarget`, `appendCardTarget`, `isPlayerSelected`, `isValidPlayerTarget`, `appendPlayerTarget`, `isValidCubeTarget`, `getCubesSelected`, `isSelectionFinished`, `isSelectionConfirmable`, `confirmSelection`, `buildAutoTarget`, `generateTarget` — and the file is structured so that **each target type only ever writes the handful it actually needs**, rather than every target type implementing the same fixed interface in full. That's the one thing worth understanding before touching this file at all.

**The dispatch-table pattern.** `buildDispatch` (top of the file) takes a `DispatchMap` — one entry per `TargetType`, each a `PartialDispatch` (every function optional) — and returns a single `TargetDispatch` object with all ten functions *always* present, each one a thin wrapper that looks up the target's own `type`, finds that type's entry in the map, and calls whichever function is actually defined there (or applies a sensible default when it's missing: `isCardSelected`/`isPlayerSelected` default to `false`, `isSelectionFinished` defaults to `true`, `isValidCubeTarget`/`getCubesSelected` default to `false`/`0`). So the rest of the app — `TargetSelector.ts`, `SelectorConfirm.tsx` — always calls a single, uniform `targetDispatch.isValidCardTarget(...)`/etc., and never needs to know which specific target type it's dealing with; the branching happens once, here.

**Two ways to register a target type**, depending on whether it needs its own local working state:
- A **plain object literal** (e.g. `player`, `card`, `players`, `cards`) — used when the "value" being built up *is* the target itself, with no extra bookkeeping. `player`'s whole entry is four lines: `isPlayerSelected: checkId`, `appendPlayerTarget: (table, selector, target, effect, player) => player`, `isValidPlayerTarget` (the shared helper checking `effect.player_filter`), `generateTarget: target => target.id`.
- **`reservedDispatch(...)`** — a thin identity-cast helper used whenever the target's `value` type is some richer object (e.g. `{ cards: Card[], max_cards: number }` for `max_cards`, or `{ players: Player[], finished: boolean }` for `adjacent_players`) rather than the raw selected value(s). It exists purely to make the TypeScript types line up (the "target" and "value" types differ), not to change behavior.

**What each function actually does**, using the real, simplest cases as reference:
- **`isValidCardTarget`/`isValidPlayerTarget`** — can this specific card/player be added to the target *right now*, given what's already selected? The shared helpers (`isValidCardTarget`/`isValidPlayerTarget`, used directly by `card`/`player` and many others) just call `checkCardFilter`/`checkPlayerFilter` (`Filters.ts`) against the effect's `card_filter`/`player_filter` — the client-side mirror of the server's `filters.cpp`. Richer target types layer more onto this: `adjacent_players`'s version only allows a second player within `max_distance` of the first once one is already selected; `card_per_player`'s version also rejects a card if that player already has a card selected (one per player, hence the name).
- **`appendCardTarget`/`appendPlayerTarget`** — given the current value and a newly-clicked card/player, produce the *next* value. For `player` this is trivial (`player` becomes the whole value, since there's only ever one). For accumulating types like `cards` it's `(target ?? []).concat(card)`; for `adjacent_players` it also decides whether the selection is `finished` after this pick (true once two players are chosen, or immediately after one if no valid second player exists at all).
- **`isSelectionFinished`** — has enough been picked to stop asking? `cards` compares `target.length === effect.ncards`; `max_cards` compares against its own computed `max_cards` (see `buildAutoTarget` below); the default (`true` when unset) is right for anything that's always complete the instant it has one value, like `player`.
- **`isSelectionConfirmable`/`confirmSelection`** — for target types where the player can choose to stop *early* rather than at a fixed count (`max_cards`'s "discard up to N, but you can stop sooner," gated by its own `confirmable` flag from the server; `select_cubes_optional`'s "you may select 0"). `confirmSelection` freezes the current partial value's length in as the new `max_cards`/`max_cubes`, effectively making "finished" retroactively true.
- **`buildAutoTarget`** — computed once, when a card is first selected, *before* the player clicks anything: this is what pre-fills a target type that doesn't actually need a manual pick. `none` returns `null` outright (nothing to select, ever). `players` (target *every* matching player, not a choice) computes the full filtered list immediately. `max_cards` and `card_per_player` precompute their own `max_cards` bound by scanning `getValidCardTargets`/`table.visible_players` against the filters — this is the client doing its own small enumeration, parallel to (but independent from) the server's `possible_to_play.cpp`. Returning `undefined` (as `conditional_player` does when no valid target exists at all) means "there's nothing to target here," used to skip a step entirely.
- **`generateTarget`** — the client-side mirror of the server's `deserialize_target`, run in the opposite direction: turns the internal `value` shape back into the flat `CardId`/`PlayerId`/array/tuple the server actually expects on the wire (matching whatever shape `target_types/<expansion>/*.cpp`'s own serialization expects). `player` → `target.id`; `cards`/`select_cubes*` → `mapIds(target)`; `player_per_cube`/`select_cubes_player` → a `[CardId[], PlayerId[]]`/`[CardId[], PlayerId]` tuple; `none`/`players`/`self_cubes` → `null` (nothing to send beyond which effect was chosen — `players` targets *everyone matching the filter*, computed identically server-side, so there's nothing player-specific to transmit).
- **`isValidCubeTarget`/`getCubesSelected`** — the cube-token-specific pair (Armed & Dangerous), parallel to the card/player pair but for cube slots rather than cards or players; `isValidCubeTarget` (a shared helper reused by most cube-based types) checks the slot belongs to `table.self_player` and still has an unselected cube; `getCubesSelected` reports back, per cube slot, how many of its cubes are currently part of the selection (used to render the "N selected" count on that specific slot).

All of this validation is purely "optimistic"/UX — it exists so the UI can grey out invalid clicks and know when to stop asking for more targets, not to enforce the rules. **Authoritative** validation always stays server-side (`play_verify.cpp`), which can and does reject the resulting action with a `game_error` regardless of what the client thought was valid.

#### What a "modifier" is, and why the client needs a state for it

A modifier is a card that isn't a complete action on its own — it has to be played *together with* another card that follows it, changing how that card behaves. The classic case is *Aim*: "play this before a Bang!, and that Bang! deals +1 damage." Aim by itself doesn't do anything meaningful; the server treats a modifier-then-real-card play as a single combined action rather than two separate ones (full mechanism in the server module doc — the client doesn't need to replicate any of it).

That matters client-side because `GameAction` isn't just "one card + targets," it's `{ card, effect_list, targets, modifiers: [{card, effect_list, targets}, ...] }` — an ordered chain of zero or more modifier plays followed by one terminal card (with its own `effect_list`, exactly like each modifier has its own). The server already precomputes, for every playable card, the exact chain of modifiers required before it: each entry in `respond_cards`/`play_cards` (`PlayableCardInfo`) carries its own `modifiers` array. So `getAllPlayableCards` (`TargetSelector.ts`) never has to work out compatibility itself — its job is reduced to sequence-matching: given how many modifiers the player has already selected (`selector.modifiers`), it filters down to entries whose own `modifiers` array starts with the same sequence, and yields either the *next* required modifier (`isModifier: true`, if `selector.modifiers.length < modifiers.length` for that entry) or the terminal card itself (`isModifier: false`, once the full chain is satisfied). This is exactly what drives the `start --(2)--> modifier` transition in the state diagram: clicking a card that `newTargetSelection` resolves with `isModifier: true` enters `modifier` mode instead of `target` mode; once its own targets are picked, `(4)` moves to `middle`, from which clicking another modifier loops back to `modifier` (stacking), or clicking the terminal card moves on to `target` `(1)`. `getModifierContext` reads any extra precomputed context a chain entry carries (e.g. a `forced_play` context, used elsewhere by cards that lock in a specific next card automatically) — again information the server already derived, not something the client has to compute.

**One modifier scenario gets a bespoke visual treatment: Gold Rush's *Bottle*/*Pardner*** (a single card playable as any one of three hidden effect-cards — see the server module doc's modifiers section for the full `card_choice` mechanism). Rather than falling through to the ordinary hand/target rendering, `CardChoiceView.tsx` watches for this specific case: once *Bottle* has been played as a modifier, `getModifierContext(selector, 'card_choice')` reads the shared context value the server attaches to all three hidden follow-up cards, resolves it back to the physical *Bottle* card on the table, and calls the same generic `getAllPlayableCards({ ...selector, selection: null })` used everywhere else in the modifier chain — which, given *Bottle* was just played, naturally resolves to exactly its three hidden options. Those get rendered as ordinary `<CardView>`s, but absolutely positioned in a small row centered on *Bottle*'s current on-screen position (via `CardTracker`, the same live position-tracking used for movement animations), nudged down slightly, so a little strip of "Panic! / Beer / Bang!" pops up right under the card the player just played. The layout is bespoke; everything deciding *which* cards to show and *when* is the same generic modifier-chain state every other modifier already uses.

#### Multi-target handlers (`mth`): the mechanism with no client-side counterpart at all

The server also has a second, related mechanism — `mth` (multi-target handler), covered in full in the server module doc — for cards like *Doc Holyday* (discard 2 cards, deal 1 damage to a chosen player, as one combined action) or *Switch* (swap one of your table cards with one of someone else's). Unlike modifiers, this one is worth calling out specifically **because it has no footprint on the client at all**, which is easy to miss given how much of the rest of this pipeline is mirrored line-for-line between the two codebases.

`CardData.mth_effect`/`mth_response` exist in the TypeScript model (`{ type: MthType | null }`, with `MthType = string`, exactly like `ModifierType`) purely because they're part of the same `card_data` struct that gets sent over the wire wholesale — nothing in `TargetSelector.ts`, `TargetDispatch.ts`, or `TargetSelectorReducer.ts` ever reads either field. A card with an `mth_effect` still declares each of its target groups as perfectly ordinary `effect_holder` entries in `effects`/`responses` (e.g. Doc Holyday's own "discard 2 cards" and "choose a player" are each just a normal `cards`/`player` target type, identical in every way to the ones any other multi-effect card uses), so the client walks through them exactly the way it walks through *Sid Ketchum*'s "discard then heal" — one slot at a time, through the same generic `target`/`middle` machinery, with no awareness that a combined handler exists behind them at all.

That's possible because of where the server enforces the `mth` handler's joint logic: `possible_to_play.cpp` only ever offers the client combinations that have *already* passed the handler's `get_error` check (see the server doc for the exact recursive-backtracking mechanism). Whatever the player is allowed to click through the ordinary per-slot flow is therefore guaranteed, by construction, to also satisfy whatever cross-group constraint the handler enforces — so the client can stay completely oblivious to `mth`'s existence and still never produce an action the server would reject for that reason.

#### The `preselect` state, explained

Every other mode in the state machine starts from the player clicking something: a card in their hand (`start` → `target`/`modifier`), or a further target. `preselect` is the one mode that starts from **the server**, not from a click. It shows up for choices that aren't really "play a card from your hand" — like picking one out of several face-up cards, or choosing a target for an effect that isn't tied to playing anything at all.

On the client, `TargetSelectorReducer.ts`'s `handlePreselect` is what detects this: whenever a new request arrives, it scans `respond_cards` for one flagged this way. If it finds one, there's nothing meaningful for the player to click to "start" a selection — so the reducer skips straight past `start`, sets `mode: 'preselect'`, and treats that card as already selected (`preselection`), just as if the player had clicked it themselves. From there, target collection proceeds exactly like the `target`/`modifier` modes (same `TargetDispatch.ts` validation, same confirm button once selection is complete) — the only difference is what put the card in that role. One consequence worth noting: `selectorCanUndo` explicitly returns `false` whenever `preselection !== null` — the player can't "undo" a choice the game itself imposed through the ordinary Undo button.

Because a preselected card can conceptually stand in for either a modifier or the actual played card depending on the situation, once its own targets are fully chosen, `handleEndPreselection` **transfers** control into the ordinary state machine: it moves to `middle` (if it was acting like a modifier) or to `target` (if it was the real play) — transitions (6)/(7) in the ASCII diagram. From that point on, the rest of the flow is indistinguishable from a normal player-initiated selection.

Being preselected is a *default*, not a lock-in. Being in `preselect` mode doesn't mean the player must go through with that particular pick — at any point, clicking a different, independently-playable card is a perfectly normal thing to do, and the reducer honors it. `SelectorConfirm.tsx`'s `getClickCardUpdate` handles the `preselect` mode with exactly this in mind:

```ts
case 'preselect': {
    const canPlay = selectorCanPlayCard(selector, card);
    const canPick = isValidCardTarget(table, selector, card);
    if (canPlay && canPick) {
        return { setPrompt: { type: 'playpick', card }};
    } else if (canPlay) {
        return { selectPlayingCard: card };
    } else if (canPick) {
        return { addCardTarget: card };
    }
    break;
}
```

If the clicked card is only a valid *pick* target, it's added to the preselection as normal (`addCardTarget`). If it's only a valid *independent play* (not a legal pick target at all), the reducer routes it straight to `selectPlayingCard` — the same action taken from `start` — which wipes the forced pick out of state entirely first. In other words: clicking any ordinary playable card while a pick is pending silently discards the preselection and starts a brand-new selection with that card, exactly as if the player had pressed Undo on the pick first. No prompt, no extra step — this is the common case.

#### The `playpick` prompt: resolving the one ambiguous case

The `playpick` branch above (`canPlay && canPick` both true) is the situation this prompt exists for: the player clicked a card that is *simultaneously* a legal target for the pending pick **and** a legitimately playable card in its own right. This isn't a corner case invented for safety — it happens whenever a pick request's target set overlaps with cards the player could otherwise choose to play.

The clearest real trigger is a player hit by **Bandidos** who also holds an **Escape** card. Bandidos forces every other player to either discard up to 2 cards or take damage; playing Escape instead cancels Bandidos for them entirely, avoiding both the discard and the damage. Since Escape is sitting right there in that player's hand, it's simultaneously a valid card to discard for the Bandidos response *and* a legitimate card to play in its own right — a single click can't tell those apart, which is exactly the situation this prompt exists for. Concretely, **Pick** means "play the Bandidos response, using Escape as one of the discarded cards," while **Play** means "play the Escape card itself" — same card clicked, two different cards actually being played.

Those are two different `GameAction`s with materially different consequences, and nothing about a single click distinguishes them — so the reducer can't silently guess. Instead it sets `prompt: { type: 'playpick', card }`, which does two things: it renders three explicit buttons in `PromptView.tsx` (`BUTTON_PLAY` / `BUTTON_PICK` / `BUTTON_UNDO`), and — via the `selector.prompt.type === 'none'` guard in `SelectorConfirmProvider` — it suspends ordinary clicking on the table until the player disambiguates.

The three buttons map directly onto the two paths already described, plus a third:
- **Play** → dispatches `selectPlayingCard: card` — plays Escape for its own effect (cancelling Bandidos), the same "discard the preselection and start fresh with this card" path taken automatically in the unambiguous case.
- **Pick** → dispatches `addCardTarget: card` — discards Escape as one of the Bandidos picks, transferring into the normal flow exactly as described above for `preselect`.
- **Undo** → dispatches `undoSelection: {}`, which calls `handlePreselect` on the original request again — re-deriving a clean `preselect` state from scratch, without committing to either interpretation. This is the one place the player *can* explicitly cancel a pick, which is otherwise not offered as a standalone button while in `preselect` mode (the generic corner "Undo" button, gated by `selectorCanUndo`, is deliberately disabled for as long as `preselection !== null`, since — outside of this ambiguous case — abandoning a pick is done implicitly by clicking whatever the player actually wants to play instead, not through a separate button).

So the two mechanisms fit together: the *implicit* path (click something else, preselection silently disappears) covers the common case; the `playpick` prompt exists only for the narrower case where the engine genuinely cannot tell, from the click alone, which of the two valid actions the player meant.
- **`Model/SelectorConfirm.tsx`** closes the loop: once the state machine reaches `finish`, it builds the `GameAction` object (identical to `game::handle_game_action` on the server side: `card`, `effect_list`, `targets[]`, `modifiers[]`, `bypass_prompt`, `timer_id`) and sends it with `connection.sendMessage({ game_action: action })`. The `handleClickCard`/`handleClickPlayer`/`handleConfirm`/`handleUndo` functions, exposed via a `React.Context` (`SelectorConfirmContext`) to the entire game UI, translate user clicks into state-machine transitions.
- **`Model/TargetSelectorReducer.ts`** is the reducer applying the `SelectorUpdate`s (e.g. `selectPlayingCard`, `addCardTarget`, `addPlayerTarget`, `confirmSelection`, `undoSelection`, `setRequest`, `setPrompt`) produced by the handlers above, updating `TargetSelector`.
- **`Model/Filters.ts`** contains pure utility functions for querying state (e.g. `getCardColor`, `getCardOwner`, `getCardPocket`), used both by the target selector and by rendering components.
- **`Model/UseCardOverlay.ts`** manages the detail overlay of a card (zoom/description) when the user selects it for inspection.
- **`Model/CardTracker.ts`** tracks the on-screen positions of cards (to compute movement animations between pockets).

### 4.4 Rendering system (React components)

| File | Role |
|---|---|
| `GameScene.tsx` | The root component of the game screen: provides the various React Contexts (`GameStateContext`, `LobbyContext`, `SelectorConfirmContext`), orchestrates `useGameState`, lays out the general layout (table, hand, log). |
| `PlayerView.tsx` / `PlayerSlotView.tsx` | Renders a player at the table: HP, role (if revealed), characters, cards on the table, tokens. |
| `CardView.tsx` / `CardSignView.tsx` / `CardOverlayView.tsx` | Renders a single card (front/back, suit, zoom overlay). |
| `Pockets/*` (`PocketView.tsx`, `CardSlot.tsx`, `StackPocket.tsx`, `CardChoiceView.tsx`, `StationsView.tsx`, `TrainView.tsx`, `FeatsPocket.tsx`) | Renders the various game piles/areas (deck, discard, shop, stations, train, feats...), each specialized for its own visual layout. |
| `Animations/*` (`AnimationView.tsx`, `MoveCardAnimation.tsx`, `MovePlayersAnimation.tsx`, `MoveTokensAnimation.tsx`, `DeckShuffleAnimation.tsx`) | Components interpolating CSS/JS transitions for the "animated" updates described in §4.2. |
| `GameLogView.tsx` / `GameStringComponent.tsx` | Renders the game log (`game_log`) and, in general, any `GameString` coming from the server, resolving `format_str`/`format_args` through the localization registry (card names, player names, numbers) and producing JSX with clickable links/highlights. |
| `PromptView.tsx` | Confirmation UI for the `yesno`/`playpick` prompts generated by the state machine. |
| `RoleView.tsx`, `StatusBar.tsx`, `GameUsersView.tsx`, `TimerWidget.tsx` (in `Components/`) | Displays role, the general status of the current request (`status_text` from `RequestStatus`), the user/spectator list, and the visual timer for timed requests (`request_timer` server-side). |

---

## 5. Auxiliary HTTP interface

Besides the WebSocket channel (which handles **everything** real-time: lobby, chat, matches), the server exposes a few simple **HTTP REST** endpoints, served by the same `uWebSockets` process (`net/wsserver.cpp`), used by the client for "static" content not tied to a session:

| Server endpoint | Consumed by (client) | Purpose |
|---|---|---|
| `GET /image/:hash` | `Scenes/Lobby/LobbyUser.tsx` (`Env.bangImageUrl`) | Retrieves a user's avatar image (PNG) from its hash, out of the server's `image_registry`. Used to show other users' avatars without having to retransmit them repeatedly over WebSocket. |
| `GET /cards/:deck` | `Scenes/AllCards/AllCards.tsx` (`Env.bangCardsUrl`) | Returns, as JSON, the entire requested deck (`card_deck_type`) — the same `card_data` generated by `config/parse_bang_cards.py` — to populate the card gallery without starting a match. |
| `GET /tracking?length=...&max_count=...` | `Scenes/Tracking/Tracking.tsx` (`Env.bangTrackingUrl`) | Returns the historical statistics series (player/lobby counts) accumulated by the server in SQLite (`net/tracking.cpp`), to populate the charts (`chart.js`). |

`Model/Env.ts` derives these URLs automatically from `VITE_BANG_SERVER_URL` (replacing `wss://`→`https://`/`ws://`→`http://`), so client and server stay in sync using a single configuration environment variable.

---

## 6. Module `Locale/` — Internationalization

The client supports **5 languages** (Italian, English, Spanish, Czech, Hungarian — folders `Italian/`, `English/`, `Spanish/`, `Czech/`, `Hungarian/`), loaded **on demand** via dynamic `import()` (per-language code-splitting, see `loadRegistries` in `Registry.tsx`) to avoid bloating the initial bundle.

Each language exposes three registries:
- **`Cards.tsx`** — the localized name + description (JSX, so it can include icons/formatting) for every card, indexed by the same identifier (`name`) used in the server's YAML (e.g. `BANG`, `MISSED`, `CALAMITY_JANET`).
- **`Labels.ts`** — generic UI strings (buttons, error messages, options), organized by "group" (e.g. `ui.UNKNOWN_CARD`).
- **`GameStrings.tsx`** — the most delicate part: maps every possible `format_str` the server can send (e.g. the message "player X played BANG! on player Y") to a function that, given the arguments already resolved into JSX (localized card/player names), produces the final text in the current language. This **completely** decouples the game logic (which on the server only ever produces a string identifier + typed arguments, never free-form text) from linguistic presentation.

The language is chosen, in priority order, from: a build-time environment variable (`VITE_LANGUAGE`, for deployments dedicated to a single language), the `?language=` query string, the user's saved preference, the browser's language (`navigator.languages`), falling back to `en`.

---

## 7. Module `Utils/` — Generic hooks and utilities

| File | Purpose |
|---|---|
| `UseWebSocket.ts` | See §2 — wrapper around the native WebSocket. |
| `UseChannel.ts` | A minimal "pub/sub" primitive (`subscribe`/`update`/`clear`) used both for raw WebSocket messages and for the `gameChannel` dedicated to `GameUpdate`s — decouples asynchronous reception from consumption in React components. |
| `UseFetch.ts` | A generic hook for HTTP GET calls (used by the `AllCards`/`Tracking` pages against the §5 endpoints). |
| `UseLocalStorage.ts` | A generic hook to persist values in `localStorage`/`sessionStorage` with typed (de)serialization (`stringConverter`, `intConverter`, `boolConverter`, `jsonConverter`) — the basis of `AppSettings.ts`. |
| `UseAssets.ts` | Preloading of images/sounds (`preloadAssets`, consumed by the `preload_assets` update — the server signals which assets will be needed before sending the updates that use them, avoiding visual "pop-in") and sound playback (`usePlaySound`, mapping `SoundId` → audio file, mirroring the server's `cards/game_events.h`/`sound_id`). |
| `ImageSerial.ts` | Serializes a local image (the user-chosen avatar) into a resized PNG data-URL (`PROPIC_SIZE = 512`), ready to be sent in the `connect`/`user_set_propic` message — the counterpart of `net/image_pixels.h` on the server. |
| `Base64Utils.ts` | Base64 encoding/decoding used in image serialization. |
| `MapCache.ts` | `makeMapCache`, a generic memoizing wrapper around a `Map` — used in `UseAssets.ts` to build `loadCardImage`/`loadGameSound`, so a given card image or sound file is only ever fetched/decoded once and reused after that. |
| `ArrayUtils.ts` | Array utilities, including the `Container`/`parseContainer` type — the TypeScript counterpart of C++ bitsets/sets serialized as JSON arrays (e.g. `game_user_flags`, `player_flags`), converted into a client-side `Set<T>` for more natural access (`.has(flag)`). |
| `RecordUtils.ts` | Utilities for immutably manipulating `Record<Key, Value>` (heavily used by `GameTableReducer.ts` to update `players`/`cards` maps). |
| `UnionUtils.ts` | Generic infrastructure for Rust/F#-style discriminated unions: `createUnionDispatch` (exhaustive pattern matching for handling an incoming message, used everywhere for `ServerMessage`/`GameUpdate`/`TableUpdate`) and `createUnionReducer` (a reducer based on the same pattern, used for `sceneReducer`/`gameTableReducer`/`targetSelectorReducer`). This is the architectural "glue" that makes it natural to translate the server's C++ `std::variant`s into exhaustively-handled, type-safe TypeScript unions. |
| `Rect.ts` | 2D geometry for computing card/player movement animations on screen. |
| `MobileCheck.ts` | Mobile-device detection (to adapt the layout, cf. `Style/PlayerGridMobile.css` vs `PlayerGridDesktop.css`). |
| `UseCloseOnLoseFocus.ts` | A UI hook for closing menus/popups on outside click. |
| `UsePrevious.ts` | The classic hook for accessing the previous value of a state/prop between renders. |
| `UseUpdateEveryFrame.ts` | A hook for animations requiring recomputation on every frame (`requestAnimationFrame`). |
| `UseMapRef.ts` | A variant of `useRef` for keeping a mutable map persistent across renders (e.g. DOM references for tracking card positions). |
| `FileUtils.ts` | Utilities for loading files/images from user input (used by `ImageSerial.ts`). |

---

## 8. How the two repositories stay in sync

`bang-server` and `bangweb` are two separate projects that **share no code**, only an implicit JSON protocol. Correspondence is maintained "by hand," with a very strict naming discipline that makes side-by-side comparison possible:

| Concept | Server (C++) | Client (TypeScript) |
|---|---|---|
| Network messages | `net::client_message` / `net::server_message` (`std::variant`) | `ClientMessage` / `ServerMessage` (discriminated unions) |
| Match state updates | `game_update` (`game/game_update.h`) | `GameUpdate` (`Scenes/Game/Model/GameUpdate.ts`) |
| Proposed game action | `game_action` (deserialized in `play_verify.cpp`) | `GameAction` (`Scenes/Game/Model/GameAction.ts`) |
| Card data | `card_data` (generated from YAML) | `CardData` (`Scenes/Game/Model/CardData.ts`) |
| Target types | folders in `target_types/<expansion>` | entries of `CardTargetTypes` in `CardTarget.ts` + logic in `TargetDispatch.ts` |
| Localizable game strings | `game_string`/`prompt_string` (typed `format_str` + `format_args`) | `GameString`/`FormatArg` + `Locale/*/GameStrings.tsx` registries |
| Match options | `game_options.h` | `GameOptions` (`GameUpdate.ts`) |
| Bitset flags (user, player, game) | `enums::bitset<T>` serialized as a JSON array | `Container<'array', T>` → `Set<T>` (`ArrayUtils.ts`) |

Since **there is no shared automatic code generation** between the two repositories (unlike the YAML→C++ link internal to `bang-server` alone), every time a new expansion or a new effect/target type is added on the server, `bangweb` must be **manually updated**: the `CardTargetTypes` union, any logic in `TargetDispatch.ts`, translations in `Locale/*/Cards.tsx` and `GameStrings.tsx`, and graphic/audio assets in `public/`.
