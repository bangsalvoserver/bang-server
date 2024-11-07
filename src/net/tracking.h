#ifndef __TRACKING_H__
#define __TRACKING_H__

#include <string>
#include <chrono>
#include <expected>

namespace tracking {

    void init_tracking(const std::string &tracking_file);

    void track_zero();
    void track_client_count(size_t client_count);
    void track_user_count(size_t user_count);
    void track_lobby_count(size_t lobby_count);

    using clock = std::chrono::system_clock;
    using duration = std::chrono::seconds;
    using timestamp = std::chrono::time_point<clock, duration>;
    using timestamp_counts = std::vector<std::pair<timestamp, size_t>>;

    struct tracking_response {
        timestamp_counts client_count;
        timestamp_counts user_count;
        timestamp_counts lobby_count;
    };

    std::expected<duration, std::string> parse_length(std::string_view length);
    tracking_response get_tracking_for(duration length);

}

#endif