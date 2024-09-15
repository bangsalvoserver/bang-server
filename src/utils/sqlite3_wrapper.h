#ifndef __SQLITE3_WRAPPER_H__
#define __SQLITE3_WRAPPER_H__

#include <stdexcept>
#include <string>

#include <sqlite3.h>

namespace sql {
    
    static void throw_if_sqlite3_error(int result) {
        if (result != SQLITE_OK) {
            throw std::runtime_error(sqlite3_errstr(result));
        }
    }

    struct sqlite3_string {
        char *value = nullptr;

        ~sqlite3_string() {
            if (value) {
                sqlite3_free(value);
            }
        }
    };

    struct sqlite3_statement {
        sqlite3_stmt *value = nullptr;

        ~sqlite3_statement() {
            if (value) {
                sqlite3_finalize(value);
            }
        }

        void bind(int index, int num) {
            throw_if_sqlite3_error(sqlite3_bind_int(value, index, num));
        }

        int step() {
            return sqlite3_step(value);
        }
    };

    struct sqlite3_connection {
        sqlite3 *value = nullptr;

        ~sqlite3_connection() {
            if (value) {
                sqlite3_close(value);
            }
        }

        explicit operator bool() const {
            return value != nullptr;
        }

        void init(const std::string &filename) {
            throw_if_sqlite3_error(sqlite3_open(filename.c_str(), &value));
        }

        void exec_sql(const std::string &sql) {
            sqlite3_string errmsg;
            int result = sqlite3_exec(value, sql.c_str(), nullptr, nullptr, &errmsg.value);
            if (result != SQLITE_OK) {
                if (errmsg.value) {
                    throw std::runtime_error(errmsg.value);
                } else {
                    throw std::runtime_error(sqlite3_errstr(result));
                }
            }
        }

        sqlite3_statement prepare(const std::string &sql) {
            sqlite3_statement result;
            throw_if_sqlite3_error(sqlite3_prepare(value, sql.c_str(), sql.size(), &result.value, nullptr));
            return result;
        }
    };
}

#endif