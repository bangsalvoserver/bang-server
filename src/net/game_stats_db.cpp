#include "game_stats_db.h"

#include "utils/sqlite3_wrapper.h"
#include "utils/enums.h"
#include "logging.h"

#include <format>

namespace game_stats {

    using namespace banggame;

    static sql::sqlite3_connection s_connection;

    // Schema versioning, tracked via SQLite's built-in `PRAGMA user_version`.
    // Bump current_schema_version and add a new `if (from_version < N)` branch
    // to migrate_schema() whenever the schema changes (e.g. a new stat column),
    // instead of editing the existing CREATE TABLE statements. This lets an
    // already-existing database file be upgraded in place via ALTER TABLE
    // instead of silently failing (or losing data) when the code expects
    // columns the on-disk schema doesn't have yet.
    static constexpr int current_schema_version = 1;

    static void migrate_schema(int from_version) {
        if (from_version < 1) {
            s_connection.exec_sql(R"SQL(
                CREATE TABLE IF NOT EXISTS games(
                    game_id INTEGER PRIMARY KEY,
                    lobby_id INTEGER NOT NULL,
                    started_at INTEGER NOT NULL,
                    ended_at INTEGER NOT NULL,
                    num_players INTEGER NOT NULL,
                    num_rounds INTEGER NOT NULL,
                    expansions TEXT NOT NULL,
                    options TEXT NOT NULL
                );

                CREATE TABLE IF NOT EXISTS game_players(
                    game_id INTEGER NOT NULL,
                    user_id INTEGER NOT NULL,
                    username TEXT NOT NULL,
                    is_bot INTEGER NOT NULL,
                    character TEXT NOT NULL,
                    role TEXT NOT NULL,
                    survived INTEGER NOT NULL,
                    won INTEGER NOT NULL,
                    elimination_order INTEGER NOT NULL,
                    died_on_round INTEGER NOT NULL,
                    bangs_played INTEGER NOT NULL,
                    ability_uses INTEGER NOT NULL,
                    dynamite_explosions INTEGER NOT NULL,
                    prison_turns_skipped INTEGER NOT NULL,
                    duels_lost INTEGER NOT NULL,
                    kills INTEGER NOT NULL,
                    cards_drawn INTEGER NOT NULL,
                    damage_dealt INTEGER NOT NULL,
                    hp_recovered INTEGER NOT NULL,
                    draw_checks_total INTEGER NOT NULL,
                    draw_checks_lucky INTEGER NOT NULL,
                    bonus_draws_used INTEGER NOT NULL,
                    volcanic_bangs_played INTEGER NOT NULL
                );

                CREATE TABLE IF NOT EXISTS next_game_id(
                    id INTEGER PRIMARY KEY CHECK (id = 0),
                    value INTEGER NOT NULL
                );
                INSERT OR IGNORE INTO next_game_id (id, value) VALUES (0, 0);

                CREATE INDEX IF NOT EXISTS idx_games_lobby_id ON games(lobby_id);
                CREATE INDEX IF NOT EXISTS idx_game_players_game_id ON game_players(game_id);
                CREATE INDEX IF NOT EXISTS idx_game_players_username ON game_players(username);
            )SQL");
        }

        // Example of a future migration, adding a new stat column without
        // touching existing rows (they get backfilled with the DEFAULT value):
        //
        // if (from_version < 2) {
        //     s_connection.exec_sql(
        //         "ALTER TABLE game_players ADD COLUMN new_stat INTEGER NOT NULL DEFAULT 0"
        //     );
        // }
    }

    void init(const std::string &db_file) {
        try {
            s_connection.init(db_file);

            int stored_version = 0;
            if (auto stmt = s_connection.prepare("PRAGMA user_version"); stmt.step()) {
                stored_version = stmt.column_int(0);
            }

            if (stored_version < current_schema_version) {
                migrate_schema(stored_version);
                s_connection.exec_sql(std::format("PRAGMA user_version = {}", current_schema_version));
            }
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

    static std::string join_expansions(const std::vector<std::string> &expansions) {
        std::string result;
        for (const std::string &value : expansions) {
            if (!result.empty()) result += ',';
            result += value;
        }
        return result;
    }

    static std::vector<std::string> split_expansions(const std::string &value) {
        std::vector<std::string> result;
        size_t start = 0;
        while (start <= value.size()) {
            size_t comma = value.find(',', start);
            size_t end = comma == std::string::npos ? value.size() : comma;
            if (end > start) {
                result.emplace_back(value.substr(start, end - start));
            }
            if (comma == std::string::npos) break;
            start = comma + 1;
        }
        return result;
    }

    void save_game(const game_report &report) {
        if (!s_connection) return;
        try {
            {
                auto stmt = s_connection.prepare(
                    "INSERT INTO games (game_id, lobby_id, started_at, ended_at, num_players, num_rounds, expansions, options) "
                    "VALUES (?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8)"
                );
                stmt.bind(1, report.game_id);
                stmt.bind(2, report.lobby_id);
                stmt.bind(3, report.started_at);
                stmt.bind(4, report.ended_at);
                stmt.bind(5, report.num_players);
                stmt.bind(6, report.num_rounds);
                stmt.bind(7, join_expansions(report.expansions));
                stmt.bind(8, std::string_view(report.options));
                stmt.step();
            }

            for (const player_game_report &p : report.players) {
                auto stmt = s_connection.prepare(
                    "INSERT INTO game_players (game_id, user_id, username, is_bot, character, role, survived, won, "
                    "elimination_order, died_on_round, "
                    "bangs_played, ability_uses, dynamite_explosions, prison_turns_skipped, duels_lost, kills, "
                    "cards_drawn, damage_dealt, hp_recovered, draw_checks_total, draw_checks_lucky, "
                    "bonus_draws_used, volcanic_bangs_played) "
                    "VALUES (?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8, ?9, ?10, ?11, ?12, ?13, ?14, ?15, ?16, "
                    "?17, ?18, ?19, ?20, ?21, ?22, ?23)"
                );
                stmt.bind(1, report.game_id);
                stmt.bind(2, p.user_id);
                stmt.bind(3, p.username);
                stmt.bind(4, p.is_bot ? 1 : 0);
                stmt.bind(5, p.character);
                stmt.bind(6, std::string(enums::to_string(p.role)));
                stmt.bind(7, p.survived ? 1 : 0);
                stmt.bind(8, p.won ? 1 : 0);
                stmt.bind(9, p.elimination_order);
                stmt.bind(10, p.died_on_round);
                stmt.bind(11, p.stats.bangs_played);
                stmt.bind(12, p.stats.ability_uses);
                stmt.bind(13, p.stats.dynamite_explosions);
                stmt.bind(14, p.stats.prison_turns_skipped);
                stmt.bind(15, p.stats.duels_lost);
                stmt.bind(16, p.stats.kills);
                stmt.bind(17, p.stats.cards_drawn);
                stmt.bind(18, p.stats.damage_dealt);
                stmt.bind(19, p.stats.hp_recovered);
                stmt.bind(20, p.stats.draw_checks_total);
                stmt.bind(21, p.stats.draw_checks_lucky);
                stmt.bind(22, p.stats.bonus_draws_used);
                stmt.bind(23, p.stats.volcanic_bangs_played);
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
                    "SELECT game_id, lobby_id, started_at, ended_at, num_players, num_rounds, expansions, options "
                    "FROM games WHERE game_id = ?1"
                );
                stmt.bind(1, game_id);
                if (!stmt.step()) {
                    return std::nullopt;
                }
                report.game_id = stmt.column_int(0);
                report.lobby_id = stmt.column_int(1);
                report.started_at = stmt.column_int64(2);
                report.ended_at = stmt.column_int64(3);
                report.num_players = stmt.column_int(4);
                report.num_rounds = stmt.column_int(5);
                report.expansions = split_expansions(stmt.column_text(6));
                report.options = stmt.column_text(7);
            }

            auto stmt = s_connection.prepare(
                "SELECT user_id, username, is_bot, character, role, survived, won, "
                "elimination_order, died_on_round, "
                "bangs_played, ability_uses, dynamite_explosions, prison_turns_skipped, duels_lost, kills, "
                "cards_drawn, damage_dealt, hp_recovered, draw_checks_total, draw_checks_lucky, "
                "bonus_draws_used, volcanic_bangs_played "
                "FROM game_players WHERE game_id = ?1"
            );
            stmt.bind(1, game_id);
            while (stmt.step()) {
                player_game_report &p = report.players.emplace_back();
                p.user_id = stmt.column_int(0);
                p.username = stmt.column_text(1);
                p.is_bot = stmt.column_int(2) != 0;
                p.character = stmt.column_text(3);
                p.role = enums::from_string<player_role>(stmt.column_text(4)).value_or(player_role::unknown);
                p.survived = stmt.column_int(5) != 0;
                p.won = stmt.column_int(6) != 0;
                p.elimination_order = stmt.column_int(7);
                p.died_on_round = stmt.column_int(8);
                p.stats.bangs_played = stmt.column_int(9);
                p.stats.ability_uses = stmt.column_int(10);
                p.stats.dynamite_explosions = stmt.column_int(11);
                p.stats.prison_turns_skipped = stmt.column_int(12);
                p.stats.duels_lost = stmt.column_int(13);
                p.stats.kills = stmt.column_int(14);
                p.stats.cards_drawn = stmt.column_int(15);
                p.stats.damage_dealt = stmt.column_int(16);
                p.stats.hp_recovered = stmt.column_int(17);
                p.stats.draw_checks_total = stmt.column_int(18);
                p.stats.draw_checks_lucky = stmt.column_int(19);
                p.stats.bonus_draws_used = stmt.column_int(20);
                p.stats.volcanic_bangs_played = stmt.column_int(21);
            }

            return report;
        } catch (const sql::sql_error &error) {
            logging::error("SQL error: {}", error.what());
            return std::nullopt;
        }
    }

    std::vector<game_report> search_games(std::string_view username, std::optional<int> lobby_id, size_t limit, size_t offset) {
        std::vector<game_report> result;
        if (!s_connection) return result;
        try {
            std::vector<int> game_ids;
            {
                auto stmt = s_connection.prepare(
                    "SELECT game_id FROM games "
                    "WHERE (?1 = '' OR game_id IN (SELECT game_id FROM game_players WHERE username = ?1)) "
                    "AND (?2 = -1 OR lobby_id = ?2) "
                    "ORDER BY started_at DESC LIMIT ?3 OFFSET ?4"
                );
                stmt.bind(1, username);
                stmt.bind(2, static_cast<int64_t>(lobby_id.value_or(-1)));
                stmt.bind(3, static_cast<int64_t>(limit));
                stmt.bind(4, static_cast<int64_t>(offset));
                while (stmt.step()) {
                    game_ids.push_back(stmt.column_int(0));
                }
            }

            for (int id : game_ids) {
                if (auto report = get_game(id)) {
                    result.push_back(std::move(*report));
                }
            }
        } catch (const sql::sql_error &error) {
            logging::error("SQL error: {}", error.what());
        }
        return result;
    }

}
