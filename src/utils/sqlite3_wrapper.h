#ifndef __SQLITE3_WRAPPER_H__
#define __SQLITE3_WRAPPER_H__

#include <stdexcept>
#include <string>
#include <utility>

#include <sqlite3.h>

namespace sql {
    
    static void throw_if_sqlite3_error(int result) {
        if (result != SQLITE_OK) {
            throw std::runtime_error(sqlite3_errstr(result));
        }
    }

    struct sqlite3_string {
        char *str = nullptr;

        sqlite3_string() = default;

        sqlite3_string(const sqlite3_string &) = delete;
        sqlite3_string &operator = (const sqlite3_string &) = delete;

        sqlite3_string(sqlite3_string &&other) noexcept : str{std::exchange(other.str, nullptr)} {}
        sqlite3_string &operator = (sqlite3_string &&other) noexcept {
            std::swap(str, other.str);
            return *this;
        }

        ~sqlite3_string() {
            if (str) {
                sqlite3_free(str);
            }
        }
    };

    struct sqlite3_statement {
        sqlite3 *db = nullptr;
        sqlite3_stmt *stmt = nullptr;

        sqlite3_statement(sqlite3 *db): db{db} {}

        sqlite3_statement(const sqlite3_statement &) = delete;
        sqlite3_statement &operator = (const sqlite3_statement &) = delete;

        sqlite3_statement(sqlite3_statement &&other) noexcept : stmt{std::exchange(other.stmt, nullptr)} {}
        sqlite3_statement &operator = (sqlite3_statement &&other) noexcept {
            std::swap(stmt, other.stmt);
            return *this;
        }

        ~sqlite3_statement() {
            if (stmt) {
                sqlite3_finalize(stmt);
            }
        }

        void bind(int index, std::string_view value) {
            throw_if_sqlite3_error(sqlite3_bind_text(stmt, index, value.data(), value.size(), nullptr));
        }

        void bind(int index, int value) {
            throw_if_sqlite3_error(sqlite3_bind_int(stmt, index, value));
        }

        void bind(int index, int64_t value) {
            throw_if_sqlite3_error(sqlite3_bind_int64(stmt, index, value));
        }

        void bind(int index, uint64_t value) {
            throw_if_sqlite3_error(sqlite3_bind_int64(stmt, index, value));
        }

        bool step() {
            int result = sqlite3_step(stmt);
            if (result != SQLITE_DONE && result != SQLITE_ROW) {
                throw std::runtime_error(sqlite3_errmsg(db));
            }
            return result != SQLITE_DONE;
        }

        int column_int(int index) {
            return sqlite3_column_int(stmt, index);
        }

        int64_t column_int64(int index) {
            return sqlite3_column_int64(stmt, index);
        }

        uint64_t column_uint64(int index) {
            return sqlite3_column_int64(stmt, index);
        }
    };

    struct sqlite3_connection {
        sqlite3 *db = nullptr;

        sqlite3_connection() = default;

        sqlite3_connection(const sqlite3_connection &) = delete;
        sqlite3_connection &operator = (const sqlite3_connection &) = delete;

        sqlite3_connection(sqlite3_connection &&other) noexcept : db{std::exchange(other.db, nullptr)} {}
        sqlite3_connection &operator = (sqlite3_connection &&other) noexcept {
            std::swap(db, other.db);
            return *this;
        }

        ~sqlite3_connection() {
            if (db) {
                sqlite3_close(db);
            }
        }

        explicit operator bool() const {
            return db != nullptr;
        }

        void init(const std::string &filename) {
            throw_if_sqlite3_error(sqlite3_open(filename.c_str(), &db));
        }

        void exec_sql(const std::string &sql) {
            sqlite3_string errmsg;
            int result = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errmsg.str);
            if (result != SQLITE_OK) {
                if (errmsg.str) {
                    throw std::runtime_error(errmsg.str);
                } else {
                    throw std::runtime_error(sqlite3_errstr(result));
                }
            }
        }

        sqlite3_statement prepare(std::string_view sql) {
            sqlite3_statement result{db};
            throw_if_sqlite3_error(sqlite3_prepare_v2(db, sql.data(), sql.size(), &result.stmt, nullptr));
            return result;
        }
    };
}

#endif