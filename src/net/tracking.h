#ifndef __TRACKING_H__
#define __TRACKING_H__

#include <string>
#include <chrono>

namespace tracking {

    void init_tracking(const std::string &tracking_file);

    void track_zero();
    void track_client_count(size_t client_count);
    void track_user_count(size_t user_count);
    void track_lobby_count(size_t lobby_count);

    using clock = std::chrono::system_clock;
    using timestamp = std::chrono::time_point<clock, std::chrono::seconds>;
    using timestamp_counts = std::vector<std::pair<timestamp, size_t>>;

    struct tracking_response {
        timestamp_counts client_count;
        timestamp_counts user_count;
        timestamp_counts lobby_count;
    };

    timestamp parse_date(std::string_view date);
    tracking_response get_tracking_since(timestamp since_date);

}

#endif