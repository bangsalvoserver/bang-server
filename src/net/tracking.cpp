#include "tracking.h"

#include "utils/sqlite3_wrapper.h"
#include "utils/parse_string.h"
#include "logging.h"

namespace tracking {

    static sql::sqlite3_connection s_connection;

    void init_tracking(const std::string &tracking_file) {
        try {
            s_connection.init(tracking_file);
            s_connection.exec_sql(R"SQL(
                CREATE TABLE IF NOT EXISTS client_count(
                    timestamp INT NOT NULL,
                    count INT NOT NULL
                );

                CREATE TABLE IF NOT EXISTS user_count(
                    timestamp INT NOT NULL,
                    count INT NOT NULL
                );

                CREATE TABLE IF NOT EXISTS lobby_count(
                    timestamp INT NOT NULL,
                    count INT NOT NULL
                );
            )SQL");
        } catch (const std::exception &error) {
            logging::error("SQL error: {}", error.what());
        }
    }

    void track_zero() {
        track_client_count(0);
        track_user_count(0);
        track_lobby_count(0);
    }

    static void track_simple(std::string_view table_name, size_t count) {
        if (s_connection) {
            try {
                s_connection.exec_sql(std::format(
                    "INSERT INTO {} (timestamp, count) VALUES (strftime('%s', 'now'), {})",
                    table_name, count
                ));
            } catch (const std::exception &error) {
                logging::error("SQL error: {}", error.what());
            }
        }
    }

    void track_client_count(size_t client_count) {
        track_simple("client_count", client_count);
    }

    void track_user_count(size_t user_count) {
        track_simple("user_count", user_count);
    }

    void track_lobby_count(size_t lobby_count) {
        track_simple("lobby_count", lobby_count);
    }

    static timestamp_counts read_tracking_simple(std::string_view table_name, timestamp start_date, duration max_diff) {
        timestamp_counts result;
        if (s_connection) {
            try {
                auto stmt = s_connection.prepare(std::format(
                    "SELECT timestamp, count FROM {} WHERE timestamp >= {}",
                    table_name, start_date.time_since_epoch().count()
                ));
                while (stmt.step()) {
                    timestamp time{std::chrono::seconds{stmt.column_int64(0)}};
                    size_t count = stmt.column_uint64(1);
                    if (time >= start_date) {
                        result.emplace_back(time, count);
                        start_date += max_diff;
                    }
                }
            } catch (const std::exception &error) {
                logging::error("SQL error: {}", error.what());
            }
        }
        return result;
    }

    std::expected<duration, std::string> parse_length(std::string_view length) {
        if (length.empty()) {
            return std::chrono::days{1};
        } else if (auto value = utils::parse_string<duration>(length)) {
            return *value;
        } else {
            return std::unexpected(std::format("Invalid length format: {}", length));
        }
    }

    tracking_response get_tracking_for(duration length, size_t max_count) {
        timestamp start_date = std::chrono::time_point_cast<duration>(clock::now() - length);
        auto max_diff = length / std::min(3000uz, max_count);
        
        return {
            read_tracking_simple("client_count", start_date, max_diff),
            read_tracking_simple("user_count", start_date, max_diff),
            read_tracking_simple("lobby_count", start_date, max_diff)
        };
    }

}