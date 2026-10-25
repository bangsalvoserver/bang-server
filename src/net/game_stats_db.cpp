#include "game_stats_db.h"

#include "utils/sqlite3_wrapper.h"
#include "utils/enums.h"
#include "logging.h"

#include <format>

namespace game_stats {

    using namespace banggame;

    static sql::sqlite3_connection s_connection;

    void init_stats(const std::string &db_file) {
        try {
            s_connection.init(db_file);

            s_connection.exec_sql(R"SQL(
                CREATE TABLE IF NOT EXISTS games(
                    game_id INTEGER PRIMARY KEY,
                    started_at INTEGER NOT NULL,
                    ended_at INTEGER NOT NULL,
                    num_players INTEGER NOT NULL,
                    num_rounds INTEGER NOT NULL,
                    options TEXT NOT NULL
                );

                CREATE TABLE IF NOT EXISTS game_players(
                    game_id INTEGER NOT NULL,
                    player_report TEXT NOT NULL
                );

                CREATE TABLE IF NOT EXISTS next_game_id(
                    id INTEGER PRIMARY KEY CHECK (id = 0),
                    value INTEGER NOT NULL
                );
                INSERT OR IGNORE INTO next_game_id (id, value) VALUES (0, 0);

                CREATE INDEX IF NOT EXISTS idx_games_game_id ON games(game_id);
                CREATE INDEX IF NOT EXISTS idx_game_players_game_id ON game_players(game_id);
            )SQL");
        } catch (const sql::sql_error &error) {
            logging::error("SQL error: {}", error.what());
        }
    }

    int get_next_game_id() {
        if (!s_connection) return 0;
        try {
            auto stmt = s_connection.prepare(
                "UPDATE next_game_id SET value = value + 1 WHERE id = 0 RETURNING value"
            );
            if (stmt.step()) {
                return stmt.column_int(0);
            }
        } catch (const sql::sql_error &error) {
            logging::error("SQL error: {}", error.what());
        }
        return 0;
    }

    void save_game(const game_report &report) {
        if (!s_connection) return;
        try {
            {
                auto stmt = s_connection.prepare(
                    "INSERT INTO games (game_id, started_at, ended_at, num_players, num_rounds, options) "
                    "VALUES (?1, ?2, ?3, ?4, ?5, ?6)"
                );
                stmt.bind(1, report.game_id);
                stmt.bind(2, report.started_at);
                stmt.bind(3, report.ended_at);
                stmt.bind(4, report.num_players);
                stmt.bind(5, report.num_rounds);
                stmt.bind(6, json::to_string(report.options));
                stmt.step();
            }

            for (const player_game_report &p : report.players) {
                auto stmt = s_connection.prepare("INSERT INTO game_players (game_id, player_report) VALUES (?1, ?2)");
                stmt.bind(1, report.game_id);
                stmt.bind(2, json::to_string(p));
                stmt.step();
            }
        } catch (const sql::sql_error &error) {
            logging::error("SQL error: {}", error.what());
        }
    }

    std::optional<game_report> get_game(int game_id) {
        if (!s_connection) return std::nullopt;
        try {
            game_report report;
            {
                auto stmt = s_connection.prepare(
                    "SELECT game_id, started_at, ended_at, num_players, num_rounds, options "
                    "FROM games WHERE game_id = ?1"
                );
                stmt.bind(1, game_id);
                if (!stmt.step()) {
                    return std::nullopt;
                }
                report.game_id = stmt.column_int(0);
                report.started_at = stmt.column_int64(1);
                report.ended_at = stmt.column_int64(2);
                report.num_players = stmt.column_int(3);
                report.num_rounds = stmt.column_int(4);
                report.options = json::parse_string<game_options>(stmt.column_text(5));
            }

            auto stmt = s_connection.prepare("SELECT player_report FROM game_players WHERE game_id = ?1");
            stmt.bind(1, game_id);
            while (stmt.step()) {
                auto player_report = stmt.column_text(0);
                report.players.push_back(json::parse_string<player_game_report>(player_report));
            }

            return report;
        } catch (const sql::sql_error &error) {
            logging::error("SQL error: {}", error.what());
        } catch (const json::deserialize_error &error) {
            logging::error("JSON error: {}", error.what());
        }
        return std::nullopt;
    }

}
