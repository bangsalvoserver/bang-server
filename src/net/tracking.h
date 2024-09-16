#ifndef __TRACKING_H__
#define __TRACKING_H__

#include <string>

namespace tracking {

    void init_tracking(const std::string &tracking_file);

    void track_zero();
    void track_client_count(size_t client_count);
    void track_user_count(size_t user_count);
    void track_lobby_count(size_t lobby_count);

}

#endif