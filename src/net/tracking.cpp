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

                CREATE TABLE IF NOT EXISTS lobby_count(
                    timestamp INT NOT NULL,
                    count INT NOT NULL
                );
            )SQL");
        } catch (const std::exception &error) {
            logging::error("SQL error: {}", error.what());
        }
    }

    void track_client_count(int client_count) {
        if (s_connection) {
            try {
                auto stmt = s_connection.prepare("INSERT INTO client_count (timestamp, count) VALUES (strftime('%s', 'now'), ?)");
                stmt.bind(1, client_count);
                stmt.step();
            } catch (const std::exception &error) {
                logging::error("SQL error: {}", error.what());
            }
        }
    }

    void track_lobby_count(int lobby_count) {
        if (s_connection) {
            try {
                auto stmt = s_connection.prepare("INSERT INTO lobby_count (timestamp, count) VALUES (strftime('%s', 'now'), ?)");
                stmt.bind(1, lobby_count);
                stmt.step();
            } catch (const std::exception &error) {
                logging::error("SQL error: {}", error.what());
            }
        }
    }

}