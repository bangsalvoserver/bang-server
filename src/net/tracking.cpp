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

                CREATE INDEX IF NOT EXISTS client_count_ts ON client_count(timestamp);

                CREATE TABLE IF NOT EXISTS user_count(
                    timestamp INT NOT NULL,
                    count INT NOT NULL
                );

                CREATE INDEX IF NOT EXISTS user_count_ts   ON user_count(timestamp);

                CREATE TABLE IF NOT EXISTS lobby_count(
                    timestamp INT NOT NULL,
                    count INT NOT NULL
                );

                CREATE INDEX IF NOT EXISTS lobby_count_ts  ON lobby_count(timestamp);
            )SQL");
        } catch (const sql::sql_error &error) {
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
            } catch (const sql::sql_error &error) {
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
                const int64_t start = start_date.time_since_epoch().count();
                const int64_t step = std::max<int64_t>(1, max_diff.count());

                {
                    auto stmt = s_connection.prepare(std::format(
                        "SELECT count FROM {} WHERE timestamp < {} ORDER BY timestamp DESC, rowid DESC LIMIT 1",
                        table_name, start
                    ));
                    if (stmt.step()) {
                        result.emplace_back(start_date, stmt.column_uint64(0));
                    }
                }

                auto stmt = s_connection.prepare(std::format(
                    "SELECT timestamp, count FROM {} WHERE timestamp >= {} ORDER BY timestamp, rowid",
                    table_name, start
                ));

                int64_t current_bucket = -1;
                timestamp_count_entry lo, hi;

                auto flush = [&] {
                    if (current_bucket < 0) return;
                    if (lo.second == hi.second) {
                        result.push_back(lo);
                    } else if (hi.first < lo.first) {
                        result.push_back(hi);
                        result.push_back(lo);
                    } else {
                        result.push_back(lo);
                        result.push_back(hi);
                    }
                };

                while (stmt.step()) {
                    int64_t seconds = stmt.column_int64(0);
                    timestamp_count_entry e{timestamp{std::chrono::seconds{seconds}}, static_cast<size_t>(stmt.column_uint64(1))};
                    int64_t bucket = (seconds - start) / step;

                    if (bucket != current_bucket) {
                        flush();
                        current_bucket = bucket;
                        lo = hi = e;
                    } else {
                        if (e.second < lo.second) lo = e;
                        if (e.second > hi.second) hi = e;
                    }
                }
                flush();
            } catch (const sql::sql_error &error) {
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
        auto max_diff = length / std::clamp(max_count, 500uz, 2000uz);
        
        return {
            read_tracking_simple("client_count", start_date, max_diff),
            read_tracking_simple("user_count", start_date, max_diff),
            read_tracking_simple("lobby_count", start_date, max_diff)
        };
    }

}