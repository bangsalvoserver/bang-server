#include "game_stats_db.h"

#include "utils/sqlite3_wrapper.h"
#include "utils/enums.h"
#include "logging.h"

#include <sstream>

namespace game_stats {

    using namespace banggame;

    static sql::sqlite3_connection s_connection;

    void init(const std::string &db_file) {
        try {
            s_connection.init(db_file);
            s_connection.exec_sql(R"SQL(
                CREATE TABLE IF NOT EXISTS games(
                    game_id TEXT PRIMARY KEY,
                    started_at INTEGER NOT NULL,
                    ended_at INTEGER NOT NULL,
                    num_players INTEGER NOT NULL,
                    num_rounds INTEGER NOT NULL,
                    expansions TEXT NOT NULL
                );

                CREATE TABLE IF NOT EXISTS game_players(
                    game_id TEXT NOT NULL,
                    user_id INTEGER NOT NULL,
                    username TEXT NOT NULL,
                    is_bot INTEGER NOT NULL,
                    character TEXT NOT NULL,
                    role TEXT NOT NULL,
                    survived INTEGER NOT NULL,
                    bangs_played INTEGER NOT NULL,
                    ability_uses INTEGER NOT NULL,
                    dynamite_explosions INTEGER NOT NULL,
                    prison_turns_skipped INTEGER NOT NULL,
                    duels_lost INTEGER NOT NULL,
                    kills INTEGER NOT NULL
                );

                CREATE INDEX IF NOT EXISTS idx_game_players_game_id ON game_players(game_id);
                CREATE INDEX IF NOT EXISTS idx_game_players_username ON game_players(username);
            )SQL");
        } catch (const sql::sql_error &error) {
            logging::error("SQL error: {}", error.what());
        }
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
        std::istringstream stream(value);
        std::string item;
        while (std::getline(stream, item, ',')) {
            if (!item.empty()) result.push_back(item);
        }
        return result;
    }

    void save_game(const game_report &report) {
        if (!s_connection) return;
        try {
            {
                auto stmt = s_connection.prepare(
                    "INSERT INTO games (game_id, started_at, ended_at, num_players, num_rounds, expansions) "
                    "VALUES (?1, ?2, ?3, ?4, ?5, ?6)"
                );
                stmt.bind(1, report.game_id);
                stmt.bind(2, report.started_at);
                stmt.bind(3, report.ended_at);
                stmt.bind(4, report.num_players);
                stmt.bind(5, report.num_rounds);
                stmt.bind(6, join_expansions(report.expansions));
                stmt.step();
            }

            for (const player_game_report &p : report.players) {
                auto stmt = s_connection.prepare(
                    "INSERT INTO game_players (game_id, user_id, username, is_bot, character, role, survived, "
                    "bangs_played, ability_uses, dynamite_explosions, prison_turns_skipped, duels_lost, kills) "
                    "VALUES (?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8, ?9, ?10, ?11, ?12, ?13)"
                );
                stmt.bind(1, report.game_id);
                stmt.bind(2, p.user_id);
                stmt.bind(3, p.username);
                stmt.bind(4, p.is_bot ? 1 : 0);
                stmt.bind(5, p.character);
                stmt.bind(6, std::string(enums::to_string(p.role)));
                stmt.bind(7, p.survived ? 1 : 0);
                stmt.bind(8, p.stats.bangs_played);
                stmt.bind(9, p.stats.ability_uses);
                stmt.bind(10, p.stats.dynamite_explosions);
                stmt.bind(11, p.stats.prison_turns_skipped);
                stmt.bind(12, p.stats.duels_lost);
                stmt.bind(13, p.stats.kills);
                stmt.step();
            }
        } catch (const sql::sql_error &error) {
            logging::error("SQL error: {}", error.what());
        }
    }

    std::optional<game_report> get_game(std::string_view game_id) {
        if (!s_connection) return std::nullopt;
        try {
            game_report report;
            {
                auto stmt = s_connection.prepare(
                    "SELECT game_id, started_at, ended_at, num_players, num_rounds, expansions "
                    "FROM games WHERE game_id = ?1"
                );
                stmt.bind(1, game_id);
                if (!stmt.step()) {
                    return std::nullopt;
                }
                report.game_id = stmt.column_text(0);
                report.started_at = stmt.column_int64(1);
                report.ended_at = stmt.column_int64(2);
                report.num_players = stmt.column_int(3);
                report.num_rounds = stmt.column_int(4);
                report.expansions = split_expansions(stmt.column_text(5));
            }

            auto stmt = s_connection.prepare(
                "SELECT user_id, username, is_bot, character, role, survived, "
                "bangs_played, ability_uses, dynamite_explosions, prison_turns_skipped, duels_lost, kills "
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
                p.stats.bangs_played = stmt.column_int(6);
                p.stats.ability_uses = stmt.column_int(7);
                p.stats.dynamite_explosions = stmt.column_int(8);
                p.stats.prison_turns_skipped = stmt.column_int(9);
                p.stats.duels_lost = stmt.column_int(10);
                p.stats.kills = stmt.column_int(11);
            }

            return report;
        } catch (const sql::sql_error &error) {
            logging::error("SQL error: {}", error.what());
            return std::nullopt;
        }
    }

    std::vector<game_report> search_games(std::string_view username, size_t limit, size_t offset) {
        std::vector<game_report> result;
        if (!s_connection) return result;
        try {
            std::vector<std::string> game_ids;
            {
                auto stmt = s_connection.prepare(
                    "SELECT game_id FROM games "
                    "WHERE (?1 = '' OR game_id IN (SELECT game_id FROM game_players WHERE username = ?1)) "
                    "ORDER BY started_at DESC LIMIT ?2 OFFSET ?3"
                );
                stmt.bind(1, username);
                stmt.bind(2, static_cast<int64_t>(limit));
                stmt.bind(3, static_cast<int64_t>(offset));
                while (stmt.step()) {
                    game_ids.push_back(stmt.column_text(0));
                }
            }

            for (const std::string &id : game_ids) {
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
