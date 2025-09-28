#ifndef __SERVER_OPTIONS_H__
#define __SERVER_OPTIONS_H__

#include "options.h"

namespace banggame {
    
    struct server_options {
        bool disable_pings = false;
        bool enable_cheats = false;
        int max_session_id_count = 10;
        
        ticks lobby_lifetime = 5min;
        ticks user_lifetime = 10s;

        ticks client_accept_timer = 30s;
        ticks ping_interval = 10s;
        ticks inactivity_timeout = 2min;
    };
}

#endif