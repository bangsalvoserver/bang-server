# Il server puo' mandare i seguenti messaggi

### La connessione e' stata accettata
    {"client_accepted": {
        "id": 1   // id utente
    }}

### In caso di errore
    {"lobby_error": "ERROR_MESSAGE"} // vedi src/locales/locale_it.h, src/locales/locale_en.h

### Mostra la riga della lobby nella schermata delle lobby
    {"lobby_update": {
        "lobby_id": 1,
        "name": "Ciao",
        "num_players": 4,
        "lobby_state": "waiting" // puo' essere waiting, playing o finished
    }}

### Informa il client che sei entrato in una lobby
    {"lobby_entered": {
        "lobby_id": 098543,
        "name": "Kasplode",
        "options": { /* oggetto game_options */ }
    }}

### Informa il client che le informazioni della lobby sono cambiate
    {"lobby_edited": { /* oggetto lobby_info */ }}

### Rimuove una lobby dalla lista
    {"lobby_removed": {
        "lobby_id": 1 // id lobby
    }}

### Informa al client del proprietario della lobby
    {"lobby_owner": {
        "id": 1 // id proprietario della lobby in cui ti trovi
    }}

### Aggiunge un utente alla lobby
    {"lobby_add_user": {
        "id": 1, // id utente
        "user": { /* oggetto user_info */ }
    }}

### Rimuove un utente dalla lobby
    {"lobby_remove_user": {
        "id": 1, // id utente
    }}

### Ricevi un messaggio in chat
    {"lobby_chat": {
        "user_id": 1,
        "message": "Ciao"
    }}

### Update di gioco
    {"game_update": { /* oggetto game_update */ }}

### La partita e' iniziata
    {"game_started": {}}

# Il client puo' mandare i seguenti messaggi

### Bisogna mandare questo come primo messaggio per entrare nel server
    {"connect": {
        "user": { /* oggetto user_info */ }
        "commit_hash": "..." // deve essere uguale all'hash di commit della versione deployata di banggameserver
    }}

### Permette all'utente di cambiare nome e immagine di profilo dopo che si e' collegato
    {"user_edit": { /* oggetto user_info */ }}

### Richiede la lista delle lobby, ritorna una serie di lobby_update
    {"lobby_list":{}}

### Crea una lobby
    {"lobby_make": { /* oggetto lobby_info */ }}

### Modifica la lobby in cui sei (solo da proprietario)
    {"lobby_edit": { /* oggetto lobby_info */ }}

### Entra in una lobby
    {"lobby_join": {
        "lobby_id": 1
    }}

### Esci dalla lobby
    {"lobby_leave": {}}

### Invia un messaggio in chat
    {"lobby_chat": {
        "message": "Ciao"
    }}

### Ritorna alla lobby dopo che la partita e' finita
    {"lobby_return": {}}

### Avvia la partita (solo da proprietario)
    {"game_start": {}}

### Azione di gioco
    {"game_action": { /* oggetto game_action */ }}

---
### oggetto **user_info**
    {
        "name": "Salvo",
        "profile_image": { /* oggetto image_pixels */ }
    }
    
### oggetto **lobby_info**
    {
        "name": "Ciao",
        "options": { /* oggetto game_options */ }
    }
### oggetto **image_pixels**
    {
        "width": 50,
        "height": 50,
        "pixels": "...." // codifica base64 dei pixel dell'immagine
    }
### oggetto **game_options**
    {
        "expansions": "valleyofshadows armedanddangerous", // stringa che contiene tutte le espansioni della partita
        "enable_ghost_cards": false,
        "character_choice":" false,
        "allow_beer_in_duel": false,
        "quick_discard_all": false,
        "scenario_deck_size": 12,
        "num_bots": 0,
        "damage_timer": 1500, // ms
        "escape_timer": 3000, // ms
        "bot_play_timer": 500, // ms
        "tumbleweed_timer": 3000, // ms
    }
