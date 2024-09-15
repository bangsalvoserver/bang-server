#ifndef __TRACKING_H__
#define __TRACKING_H__

#include <string>

namespace tracking {

    void init_tracking(const std::string &tracking_file);

    void track_client_count(int client_count);
    void track_lobby_count(int lobby_count);

}

#endif