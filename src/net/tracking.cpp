#include "tracking.h"

#include <sqlite3.h>
#include <stdexcept>

namespace tracking {

    struct sqlite3_connection {
        sqlite3 *value = nullptr;

        ~sqlite3_connection() {
            if (value) {
                sqlite3_close(value);
            }
        }
    };

    static sqlite3_connection s_connection;

    static void throw_if_sqlite3_error(int result) {
        if (result != SQLITE_OK) {
            throw std::runtime_error(sqlite3_errstr(result));
        }
    }

    void init_tracking(const std::string &tracking_file) {
        throw_if_sqlite3_error(sqlite3_open(tracking_file.c_str(), &s_connection.value));
        // TODO create tables
    }

    void track_client_count(int client_count) {
        if (s_connection.value) {
            // TODO insert into client_count (timestamp, client_count)
        }
    }

}