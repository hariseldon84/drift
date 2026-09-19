#include "storage/sqlite_history_repo.h"
#include <sqlite3.h>
#include <stdexcept>

namespace drift::storage {

struct SqliteHistoryRepo::Impl {
  sqlite3* db = nullptr;

  explicit Impl(const std::string& path) {
    if (sqlite3_open(path.c_str(), &db) != SQLITE_OK)
      throw std::runtime_error("SqliteHistoryRepo: cannot open " + path);
    sqlite3_exec(db,
      "CREATE TABLE IF NOT EXISTS history("
        "url TEXT NOT NULL,"
        "title TEXT NOT NULL,"
        "timestamp_utc TEXT NOT NULL"
      ");",
      nullptr, nullptr, nullptr);
  }

  ~Impl() { if (db) sqlite3_close(db); }
};

SqliteHistoryRepo::SqliteHistoryRepo(const std::string& db_path)
    : impl_(std::make_unique<Impl>(db_path)) {}

SqliteHistoryRepo::~SqliteHistoryRepo() = default;

std::vector<history::HistoryEntry> SqliteHistoryRepo::load_all() {
  std::vector<history::HistoryEntry> result;
  sqlite3_stmt* stmt = nullptr;
  sqlite3_prepare_v2(impl_->db,
    "SELECT url, title, timestamp_utc FROM history ORDER BY rowid;",
    -1, &stmt, nullptr);
  while (sqlite3_step(stmt) == SQLITE_ROW) {
    history::HistoryEntry e;
    e.url           = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
    e.title         = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
    e.timestamp_utc = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
    result.push_back(std::move(e));
  }
  sqlite3_finalize(stmt);
  return result;
}

void SqliteHistoryRepo::save_all(const std::vector<history::HistoryEntry>& entries) {
  // ponytail: full replace; fine at typical history sizes, add incremental append if needed
  sqlite3_exec(impl_->db, "BEGIN; DELETE FROM history;", nullptr, nullptr, nullptr);
  sqlite3_stmt* stmt = nullptr;
  sqlite3_prepare_v2(impl_->db,
    "INSERT INTO history(url, title, timestamp_utc) VALUES(?,?,?);",
    -1, &stmt, nullptr);
  for (const auto& e : entries) {
    sqlite3_bind_text(stmt, 1, e.url.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, e.title.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, e.timestamp_utc.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_reset(stmt);
  }
  sqlite3_finalize(stmt);
  sqlite3_exec(impl_->db, "COMMIT;", nullptr, nullptr, nullptr);
}

}  // namespace drift::storage
