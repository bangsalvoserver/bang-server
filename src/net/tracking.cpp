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

}