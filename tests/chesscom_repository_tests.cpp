#include "test.hpp"

#include "pct/app/repository.hpp"
#include "pct/common/error.hpp"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

using namespace pct;

namespace {

struct TempRepository {
    std::filesystem::path directory;
    std::filesystem::path log_path;

    explicit TempRepository(std::string_view name)
        : directory(std::filesystem::temp_directory_path() /
                    ("pct-chesscom-" + std::string(name) + "-" +
                     std::to_string(::getpid()))),
          log_path(directory / "events.log") {
        std::filesystem::remove_all(directory);
        std::filesystem::create_directories(directory);
    }

    ~TempRepository() { std::filesystem::remove_all(directory); }
};

app::ChessComProfile profile() {
    return app::ChessComProfile{"Alice-Player", "", {"Rapid", "blitz", "rapid"},
                                "2026-06", 1234, "rate limited\nretry later"};
}

app::ChessComArchiveEntry archive_entry(std::size_t id, std::string month = "2026-06") {
    const std::string game_id = std::to_string(100000 + id);
    return app::ChessComArchiveEntry{
        game_id,
        "https://www.chess.com/game/live/" + game_id,
        "[Event \"Archive\"]\n\n1. e4 e5 *",
        "alice-player",
        std::move(month),
        id % 2 == 0 ? "rapid" : "blitz",
        static_cast<std::int64_t>(1000 + id),
        2000,
        "https://api.chess.com/pub/player/alice-player/games/2026/06"};
}

import::ImportedGame imported_game(std::size_t id) {
    const std::string pgn =
        "[Event \"Historical " + std::to_string(id) + "\"]\n"
        "[White \"Player" + std::to_string(id) + "\"]\n"
        "[Black \"Opponent\"]\n"
        "[Result \"1-0\"]\n\n"
        "1. e4 e5 2. Nf3 Nc6 1-0";
    return import::ImportedGame{chess::parse_pgn(pgn),
                                "https://www.chess.com/game/live/" + std::to_string(id), pgn,
                                import::ImportMethod::PublicApi};
}

json::Value read_json(const std::filesystem::path& path) {
    std::ifstream input(path);
    std::stringstream contents;
    contents << input.rdbuf();
    return json::parse(contents.str());
}

unsigned permissions(const std::filesystem::path& path) {
    struct stat status {};
    if (::stat(path.c_str(), &status) != 0)
        throw std::runtime_error("stat failed");
    return static_cast<unsigned>(status.st_mode & 0777);
}

} // namespace

TEST_CASE("Chess.com profile archive checkpoint and sync projections restart deterministically") {
    TempRepository files("restart");
    {
        storage::EventLog log(files.log_path);
        app::Repository repository(log);
        repository.save_chesscom_profile(profile());
        CHECK_EQ(repository.index_chesscom_archive_chunk({archive_entry(1), archive_entry(2)}),
                 2ULL);
        repository.checkpoint_chesscom_month(
            {"alice-player", "2026-06", "https://api.chess.com/archive", 2, 3000});
        repository.save_chesscom_sync_state(
            {"succeeded", "Alice-Player", "2026-06", "2026-06", 1, 2, 100, 3000, ""});
    }
    {
        storage::EventLog log(files.log_path);
        app::Repository repository(log);
        CHECK_EQ(repository.chesscom_profile()->normalized_username, "alice-player");
        CHECK_EQ(repository.chesscom_profile()->selected_time_controls.size(), 2ULL);
        CHECK_EQ(repository.chesscom_profile()->last_error, "rate limited retry later");
        CHECK_EQ(repository.chesscom_archive_entry("100001")->time_class, "blitz");
        CHECK_EQ(repository.chesscom_month_checkpoints("ALICE-PLAYER").size(), 1ULL);
        CHECK_EQ(repository.chesscom_sync_state().games_indexed, 2ULL);
    }
}

TEST_CASE("Chess.com archive chunks are idempotent by exact game id") {
    TempRepository files("duplicate");
    storage::EventLog log(files.log_path);
    app::Repository repository(log);
    CHECK_EQ(repository.index_chesscom_archive_chunk({archive_entry(1), archive_entry(1)}), 1ULL);
    CHECK_EQ(repository.index_chesscom_archive_chunk({archive_entry(1)}), 0ULL);
    CHECK(repository.chesscom_archive_entry("100001").has_value());
    CHECK(!repository.chesscom_archive_entry("1000010").has_value());
    CHECK_EQ(log.replay().events.size(), 1ULL);
}

TEST_CASE("Chess.com snapshot v2 restores projections and v1 remains loadable") {
    TempRepository files("snapshot-v2");
    std::filesystem::path snapshot;
    {
        storage::EventLog log(files.log_path);
        app::Repository repository(log);
        repository.save_chesscom_profile(profile());
        static_cast<void>(repository.index_chesscom_archive_chunk({archive_entry(3)}));
        snapshot = repository.create_snapshot();
    }
    CHECK_EQ(read_json(snapshot).at("snapshot_version").as_int(), 2);
    {
        storage::EventLog log(files.log_path);
        app::Repository repository(log);
        CHECK(repository.chesscom_profile().has_value());
        CHECK(repository.chesscom_archive_entry("100003").has_value());
    }

    TempRepository legacy("snapshot-v1");
    std::filesystem::create_directories(legacy.directory / "snapshots");
    storage::EventLog legacy_log(legacy.log_path);
    static_cast<void>(legacy_log.append(storage::EventType::GameParsed, "{}"));
    {
        std::ofstream output(legacy.directory / "snapshots" / "projection-1.json");
        output << R"json({"snapshot_version":1,"last_event_id":1,"games":[],"drills":[],"resource_completions":[],"analysis_jobs":[],"batches":[],"recommended_resources":[],"background_paused":false})json";
    }
    static_cast<void>(legacy_log.append(
        storage::EventType::ProfileSnapshotCreated,
        R"json({"path":"projection-1.json","last_event_id":1,"snapshot_version":1})json"));
    app::Repository legacy_repository(legacy_log);
    CHECK(!legacy_repository.chesscom_profile().has_value());
    CHECK(legacy_repository.search_chesscom_archive().entries.empty());
}

TEST_CASE("Chess.com indexes rebuild from authoritative events with private permissions") {
    TempRepository files("indexes");
    std::filesystem::permissions(
        files.directory,
        std::filesystem::perms::owner_all | std::filesystem::perms::group_read |
            std::filesystem::perms::group_exec,
        std::filesystem::perm_options::replace);
    {
        storage::EventLog log(files.log_path);
        app::Repository repository(log);
        repository.save_chesscom_profile(profile());
        static_cast<void>(repository.index_chesscom_archive_chunk({archive_entry(4)}));
    }
    const auto profile_index = files.directory / "chesscom-profile.idx";
    const auto archive_index = files.directory / "chesscom-archive.idx";
    std::filesystem::remove(profile_index);
    {
        std::ofstream corrupt(archive_index, std::ios::trunc);
        corrupt << "corrupt";
    }
    {
        storage::EventLog log(files.log_path);
        app::Repository repository(log);
        CHECK(repository.chesscom_archive_entry("100004").has_value());
    }
    CHECK_EQ(read_json(profile_index).at("profile").at("normalized_username").as_string(),
             "alice-player");
    CHECK_EQ(read_json(archive_index).at("entries").as_array().size(), 1ULL);
    CHECK_EQ(permissions(files.directory), 0750U);
    CHECK_EQ(permissions(files.log_path), 0600U);
    CHECK_EQ(permissions(profile_index), 0600U);
    CHECK_EQ(permissions(archive_index), 0600U);
}

TEST_CASE("Chess.com checkpoints reject invalid and stale values without fault leakage") {
    TempRepository files("checkpoint");
    storage::EventLog log(files.log_path);
    app::Repository repository(log);
    CHECK(app::valid_chesscom_month("2026-12"));
    CHECK(!app::valid_chesscom_month("2026-13"));
    CHECK_THROWS(repository.checkpoint_chesscom_month(
        {"alice-player", "2026-13", "https://api.chess.com/archive", 0, 1}));
    repository.checkpoint_chesscom_month(
        {"alice-player", "2026-06", "https://api.chess.com/archive", 1, 20});
    CHECK_THROWS(repository.checkpoint_chesscom_month(
        {"alice-player", "2026-06", "https://api.chess.com/archive", 2, 19}));

    log.set_append_fault_hook([](storage::AppendStage stage) {
        if (stage == storage::AppendStage::BeforeWrite)
            throw Error(ErrorCode::IoError, "injected fault");
    });
    CHECK_THROWS(repository.checkpoint_chesscom_month(
        {"alice-player", "2026-07", "https://api.chess.com/archive", 1, 30}));
    CHECK(!repository.chesscom_month_checkpoint("alice-player", "2026-07").has_value());
}

TEST_CASE("Chess.com archive search is deterministic bounded filtered and paginated") {
    TempRepository files("search");
    storage::EventLog log(files.log_path);
    app::Repository repository(log);
    std::vector<app::ChessComArchiveEntry> entries;
    for (std::size_t id = 0; id < 250; ++id)
        entries.push_back(archive_entry(id, id < 225 ? "2026-06" : "2026-05"));
    CHECK_EQ(repository.index_chesscom_archive_chunk(std::vector<app::ChessComArchiveEntry>(
                 entries.begin(), entries.begin() + 100)),
             100ULL);
    CHECK_EQ(repository.index_chesscom_archive_chunk(std::vector<app::ChessComArchiveEntry>(
                 entries.begin() + 100, entries.begin() + 200)),
             100ULL);
    CHECK_EQ(repository.index_chesscom_archive_chunk(std::vector<app::ChessComArchiveEntry>(
                 entries.begin() + 200, entries.end())),
             50ULL);

    app::ChessComArchiveSearch search;
    search.username = "Alice-Player";
    search.month = "2026-06";
    search.limit = 1000;
    const auto first = repository.search_chesscom_archive(search);
    CHECK_EQ(first.entries.size(), app::chesscom_archive_search_limit);
    CHECK(first.has_more);
    CHECK_EQ(first.entries.front().game_id, "100224");
    CHECK(first.entries.front().pgn.empty());
    CHECK(!repository.chesscom_archive_entry("100224")->pgn.empty());
    search.offset = first.next_offset;
    const auto second = repository.search_chesscom_archive(search);
    CHECK_EQ(second.entries.size(), 25ULL);
    CHECK(!second.has_more);

    search = {};
    search.time_class = "rapid";
    search.limit = 10;
    CHECK_EQ(repository.search_chesscom_archive(search).entries.size(), 10ULL);
}

TEST_CASE("Chess.com projections survive event-log compaction") {
    TempRepository files("compaction");
    {
        storage::EventLog log(files.log_path);
        app::Repository repository(log);
        repository.save_chesscom_profile(profile());
        static_cast<void>(repository.index_chesscom_archive_chunk({archive_entry(8)}));
        repository.checkpoint_chesscom_month(
            {"alice-player", "2026-06", "https://api.chess.com/archive", 1, 20});
        repository.save_chesscom_sync_state(
            {"running", "alice-player", "", "2026-06", 0, 1, 10, 20, ""});
        repository.save_chesscom_sync_state(
            {"succeeded", "alice-player", "done", "2026-06", 1, 1, 10, 30, ""});
        static_cast<void>(repository.compact_storage());
    }
    storage::EventLog log(files.log_path);
    app::Repository repository(log);
    CHECK(repository.chesscom_profile().has_value());
    CHECK(repository.chesscom_archive_entry("100008").has_value());
    CHECK(repository.chesscom_month_checkpoint("alice-player", "2026-06").has_value());
    CHECK_EQ(repository.chesscom_sync_state().status, "succeeded");
}

TEST_CASE("snapshots require a matching marker after log truncation and id reuse") {
    TempRepository files("snapshot-truncation");
    {
        storage::EventLog log(files.log_path);
        app::Repository repository(log);
        repository.save_chesscom_profile(profile());
        static_cast<void>(repository.create_snapshot());
    }
    std::filesystem::resize_file(files.log_path, 0);
    {
        storage::EventLog log(files.log_path);
        app::Repository repository(log);
        auto replacement = profile();
        replacement.original_username = "Replacement";
        repository.save_chesscom_profile(std::move(replacement));
    }
    {
        storage::EventLog log(files.log_path);
        app::Repository repository(log);
        CHECK_EQ(repository.chesscom_profile()->normalized_username, "replacement");
    }
}

TEST_CASE("a stale shared repository cannot snapshot an unseen authoritative tail") {
    TempRepository files("stale-snapshot");
    storage::EventLog log(files.log_path);
    app::Repository stale(log);
    app::Repository current(log);
    current.save_chesscom_profile(profile());
    CHECK_THROWS(stale.create_snapshot());
    CHECK(!std::filesystem::exists(files.directory / "snapshots"));
}

TEST_CASE("event log secures only directories it creates") {
    const auto root = std::filesystem::temp_directory_path() /
                      ("pct-private-parent-" + std::to_string(::getpid()));
    std::filesystem::remove_all(root);
    {
        storage::EventLog log(root / "owned" / "events.log");
        CHECK_EQ(permissions(root), 0700U);
        CHECK_EQ(permissions(root / "owned"), 0700U);
        CHECK_EQ(permissions(root / "owned" / "events.log"), 0600U);
    }
    std::filesystem::permissions(
        root / "owned", std::filesystem::perms::owner_all |
                            std::filesystem::perms::group_read,
        std::filesystem::perm_options::replace);
    {
        storage::EventLog log(root / "owned" / "second.log");
        CHECK_EQ(permissions(root / "owned"), 0740U);
        CHECK_EQ(permissions(root / "owned" / "second.log"), 0600U);
    }
    std::filesystem::remove_all(root);
}

TEST_CASE("Chess.com archive chunks enforce count and encoded-byte bounds") {
    TempRepository files("chunk-bounds");
    storage::EventLog log(files.log_path);
    app::Repository repository(log);
    std::vector<app::ChessComArchiveEntry> too_many;
    for (std::size_t id = 0; id <= app::chesscom_archive_chunk_limit; ++id)
        too_many.push_back(archive_entry(id));
    CHECK_THROWS(repository.index_chesscom_archive_chunk(std::move(too_many)));

    auto oversized = archive_entry(500);
    oversized.pgn.assign(app::chesscom_archive_chunk_encoded_byte_limit, 'P');
    CHECK_THROWS(repository.index_chesscom_archive_chunk({std::move(oversized)}));
    CHECK_EQ(log.replay().events.size(), 0ULL);
}

TEST_CASE("Chess.com profile fields are trimmed bounded validated and secrets are redacted") {
    TempRepository files("profile-validation");
    storage::EventLog log(files.log_path);
    app::Repository repository(log);
    auto value = profile();
    value.original_username = "  Alice-Player  ";
    value.archive_cursor = "  2026-06:next  ";
    value.last_error =
        "Authorization: Bearer top-secret Cookie=session-value "
        "https://example.test/fail?code=private&token=hidden";
    repository.save_chesscom_profile(std::move(value));
    const auto stored = repository.chesscom_profile();
    CHECK_EQ(stored->original_username, "Alice-Player");
    CHECK_EQ(stored->archive_cursor, "2026-06:next");
    CHECK(stored->last_error.find("top-secret") == std::string::npos);
    CHECK(stored->last_error.find("session-value") == std::string::npos);
    CHECK(stored->last_error.find("private") == std::string::npos);
    CHECK(stored->last_error.find("hidden") == std::string::npos);
    CHECK(stored->last_error.find("[redacted]") != std::string::npos);

    auto invalid = profile();
    invalid.archive_cursor.assign(app::chesscom_profile_cursor_limit + 1, 'x');
    CHECK_THROWS(repository.save_chesscom_profile(std::move(invalid)));
    invalid = profile();
    invalid.selected_time_controls = {"rapid", "unknown"};
    CHECK_THROWS(repository.save_chesscom_profile(std::move(invalid)));
}

TEST_CASE("bulk historical game import mixes exact duplicates and new identities") {
    TempRepository files("bulk-mixed");
    storage::EventLog log(files.log_path);
    app::Repository repository(log);
    const auto first = imported_game(1);
    const auto second = imported_game(2);
    const auto third = imported_game(3);
    CHECK(repository.add(first) == app::AddResult::Added);

    const auto result = repository.bulk_add({first, second, second, third});
    CHECK_EQ(result.added, 2ULL);
    CHECK_EQ(result.duplicates, 2ULL);
    CHECK_EQ(result.added_game_ids.size(), 2ULL);
    CHECK_EQ(result.duplicate_game_ids.size(), 2ULL);
    CHECK_EQ(result.added_game_ids[0], second.game.identity);
    CHECK_EQ(result.added_game_ids[1], third.game.identity);
    CHECK_EQ(repository.size(), 3ULL);

    const auto events = log.replay().events;
    CHECK_EQ(events.size(), 6ULL);
    CHECK(events[2].type == storage::EventType::GameImported);
    CHECK(events[3].type == storage::EventType::GameParsed);
    CHECK(events[4].type == storage::EventType::GameImported);
    CHECK(events[5].type == storage::EventType::GameParsed);
    CHECK_EQ(json::parse(events[2].payload).at("game_id").as_string(), second.game.identity);
    CHECK_EQ(json::parse(events[4].payload).at("game_id").as_string(), third.game.identity);
}

TEST_CASE("bulk historical game import projection is restart-equivalent") {
    TempRepository files("bulk-restart");
    std::map<std::string, app::StoredGame> before;
    {
        storage::EventLog log(files.log_path);
        app::Repository repository(log);
        const auto result =
            repository.bulk_add({imported_game(10), imported_game(11), imported_game(12)});
        CHECK_EQ(result.added, 3ULL);
        for (const auto& stored : repository.list())
            before.emplace(stored.imported.game.identity, stored);
    }
    {
        storage::EventLog log(files.log_path);
        app::Repository repository(log);
        CHECK_EQ(repository.size(), before.size());
        for (const auto& [game_id, expected] : before) {
            const auto restored = repository.get(game_id);
            CHECK(restored.has_value());
            CHECK_EQ(restored->imported.pgn, expected.imported.pgn);
            CHECK_EQ(restored->imported.source_url, expected.imported.source_url);
            CHECK_EQ(restored->imported_at_ms, expected.imported_at_ms);
        }
    }
}

TEST_CASE("bulk historical game import rebuilds derived indexes only after event writes") {
    TempRepository files("bulk-index-rebuild");
    storage::EventLog log(files.log_path);
    app::Repository repository(log);
    const auto games_index = files.directory / "games.idx";
    std::filesystem::remove(games_index);
    std::vector<bool> index_seen_during_append;
    log.set_append_fault_hook([&](storage::AppendStage stage) {
        if (stage == storage::AppendStage::BeforeWrite)
            index_seen_during_append.push_back(std::filesystem::exists(games_index));
    });
    const auto result = repository.bulk_add({imported_game(20), imported_game(21)});
    log.set_append_fault_hook({});

    CHECK_EQ(result.added, 2ULL);
    CHECK_EQ(index_seen_during_append.size(), 4ULL);
    for (const bool existed : index_seen_during_append)
        CHECK(!existed);
    CHECK(std::filesystem::exists(games_index));
    CHECK_EQ(read_json(games_index).at("games").as_array().size(), 2ULL);
}

TEST_CASE("bulk historical game import rejects oversized inputs before appending") {
    TempRepository files("bulk-bounds");
    storage::EventLog log(files.log_path);
    app::Repository repository(log);
    std::vector<import::ImportedGame> too_many(app::bulk_game_import_limit + 1,
                                               imported_game(30));
    CHECK_THROWS(repository.bulk_add(std::move(too_many)));
    auto too_large = imported_game(31);
    too_large.pgn.assign(app::bulk_game_import_pgn_byte_limit + 1, 'P');
    CHECK_THROWS(repository.bulk_add({std::move(too_large)}));
    CHECK_EQ(log.replay().events.size(), 0ULL);
}
