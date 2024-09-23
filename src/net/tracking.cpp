#include "tracking.h"

#include "utils/sqlite3_wrapper.h"
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

    static timestamp_counts read_tracking_simple(std::string_view table_name, timestamp since_date) {
        timestamp_counts result;
        if (s_connection) {
            try {
                auto stmt = s_connection.prepare(std::format(
                    "SELECT timestamp, count FROM {} WHERE timestamp >= {}",
                    table_name, since_date.time_since_epoch().count()
                ));
                while (stmt.step()) {
                    timestamp time{std::chrono::seconds{stmt.column_int64(0)}};
                    size_t count = stmt.column_uint64(1);
                    result.emplace_back(time, count);
                }
            } catch (const std::exception &error) {
                logging::error("SQL error: {}", error.what());
            }
        }
        return result;
    }

    static std::optional<tracking::timestamp> parse_date(std::string_view date) {
        if (date.empty()) {
            return tracking::timestamp{};
        }
        std::tm tm = {};
        std::stringstream ss{std::string(date)};
        ss >> std::get_time(&tm, "%Y-%m-%d");
        if (ss.fail()) {
            return std::nullopt;
        }
        auto timestamp = tracking::clock::from_time_t(std::mktime(&tm));
        return tracking::timestamp{ std::chrono::duration_cast<std::chrono::seconds>(timestamp.time_since_epoch()) };
    }

    tracking_response get_tracking_since(std::string_view since_date) {
        if (auto timestamp = parse_date(since_date)) {
            return {
                read_tracking_simple("client_count", *timestamp),
                read_tracking_simple("user_count", *timestamp),
                read_tracking_simple("lobby_count", *timestamp)
            };
        } else {
            return {};
        }
    }

}