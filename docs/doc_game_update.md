# Oggetto game_update

### In caso di errore
    {"game_error": { /* oggetto game_string */ }}

### In caso di messaggio di log di gioco
    {"game_log": { /* oggetto game_string */ }}

### In caso di messaggio di prompt
    {"game_prompt" { /* oggetto game_string */ }}

### Aggiunge le carte in un pocket
    {"add_cards": {
        "card_ids": [
            {
                "id": 1, // id della carta
                "deck": "main_deck" // tipo del mazzo della carta
            }
        ],
        "pocket": "main_deck" // in quale pocket aggiunge la carta, vedi pockets
        "player": null
    }}

### Rimuove carte
    {"remove_cards": {
        "cards": [ 1, 2, 3 ... ] // id delle carte da rimuovere
    }}

### Muove una carta
    {"move_card": {
        "card": 123, // id della carta da muovere
        "pocket": "player_hand", // verso quale pocket muoverla
        "player": 2,
        "duration": 333 // durata dell'animazione in ms
    }}

### Aggiunge i cubetti
    {"add_cubes": {
        "num_cubes": 40,
        "target_card": 123, // se null indica i cubetti sul tavolo
    }}

### Muove i cubetti
    {"move_cubes": {
        "num_cubes": 2, // numero di cubetti da muovere
        "origin_card": null,
        "target_card": 123,
        "duration": 250
    }}

### Sposta il mazzetto delle carte scenario
    {"move_scenario_deck": {
        "pocket": "scenario_deck", // scenario_deck o wws_scenario_deck
        "player": 2, // id di destinazione
        "duration": 333
    }}

### Sposta il treno
    {"move_train": {
        "position": 2, // posizione del treno
        "duration": 500
    }}

### Mescola il mazzo
    {"deck_shuffled": {
        "pocket": "main_deck", // main_deck o shop_deck
        "duration": 1333
    }}

### Mostra la carta
    {"show_card": {
        "card": 123,
        "info": { /* oggetto card_info */ },
        "duration": 167
    }}

### Copre la carta
    {"hide_card": {
        "card": 123,
        "duration": 167
    }}

### Gira la carta orizzontalmente
    {"tap_card": {
        "card": 123,
        "inactive": true, // true se orizzontale, false se verticale
        "duration": 167
    }}

### Flasha la carta
    {"flash_card": {
        "card": 123,
        "duration": 167
    }}

### Pausa
    {"short_pause": {
        "card": 123, // puo' essere null
        "duration": 333
    }}

### Aggiunge un player
    {"player_add": {
        [
            {
                "player_id": 1,
                "user_id": 123478264
            },
            {
                "player_id": 2,
                "user_id": 68713561
            },
            {
                "player_id": 3,
                "user_id": 5456465
            }
        ]
    }}

### Cambia l'ordine dei giocatori nel tavolo
    {"player_order": {
        "players": [1,2,3,4,5,6],
        "duration": 1000
    }}

### Imposta gli hp di un giocatore
    {"player_hp": {
        "player": 2,
        "hp": 3,
        "duration": 333
    }}

### Imposta il numero di pepite di un giocatore
    {"player_gold": {
        "player": 2,
        "gold": 5
    }}

### Rivela il ruolo di un giocatore
    {"player_show_role": {
        "player": 2,
        "role": "outlaw",
        "duration": 167
    }}

### Setta le flag del giocatore
    {"player_status": {
        "player": 2,
        "flags": "",
        "range_mod": 0,
        "weapon_range": 1,
        "distance_mod": 0
    }}

### Cambia l'indicatore di turno
    {"switch_turn": 2} // id di player

### Informazioni di request (risposta)
    {"request_status": {
        "origin_card": 123, // carta che ha attivato l'effetto
        "origin": 4, // giocatore che ha attivato l'effetto
        "target": 3, // giocatore contro cui e' attivato l'effetto
        "status_text": { /* oggetto game_string */ },
        "auto_select": false, // se true il client auto clicca sull'unica carta giocabile di risposta
        "respond_cards": [ /* oggetto card_modifier_tree */ ],
        "pick_card": [ 123, 124, 125 ], // lista di carte che si possono 'selezionare'
        "highlight_cards": [] // lista di carte da evidenziare
    }}

### Informazioni di request (di turno)
    {"status_ready": {
        "play_cards": [ /* oggetto card_modifier_tree */ ]
    }}

### Setta le flag di gioco
    {"game_flags": ""}

### Riproduce un suono
    {"play_sound": "bang"}

### Rimuove le informazioni di request
    {"status_clear": {}}

---
### Oggetto game_string
    {
        "format_str": "LOG_DISCARDED_CARD_FOR", // vedere src/locales/locale_it.h, src/locales/locale_en.h
        "format_args": [
            {"card": 125},
            {"player": 4},
            {"integer": 10}
        ]
    }

### Oggetto card_modifier_tree
    [
        {
            "card": 123,
            "branches": [
                {
                    "card": 124,
                    "branches": []
                }
            ]
        },
        {
            "card": 124,
            "branches": [
                {
                    "card": 125,
                    "branches": []
                }
            ]
        }
    ]

