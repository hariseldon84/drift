#include "storage/sqlite_bookmark_repo.h"
#include <sqlite3.h>
#include <stdexcept>

namespace drift::storage {

struct SqliteBookmarkRepo::Impl {
  sqlite3* db = nullptr;

  explicit Impl(const std::string& path) {
    if (sqlite3_open(path.c_str(), &db) != SQLITE_OK)
      throw std::runtime_error("SqliteBookmarkRepo: cannot open " + path);
    sqlite3_exec(db,
      "CREATE TABLE IF NOT EXISTS bookmarks("
        "url TEXT NOT NULL UNIQUE,"
        "title TEXT NOT NULL,"
        "timestamp_utc TEXT NOT NULL"
      ");",
      nullptr, nullptr, nullptr);
  }

  ~Impl() { if (db) sqlite3_close(db); }
};

SqliteBookmarkRepo::SqliteBookmarkRepo(const std::string& db_path)
    : impl_(std::make_unique<Impl>(db_path)) {}

SqliteBookmarkRepo::~SqliteBookmarkRepo() = default;

std::vector<bookmarks::Bookmark> SqliteBookmarkRepo::load_all() {
  std::vector<bookmarks::Bookmark> result;
  sqlite3_stmt* stmt = nullptr;
  sqlite3_prepare_v2(impl_->db,
    "SELECT url, title, timestamp_utc FROM bookmarks ORDER BY rowid;",
    -1, &stmt, nullptr);
  while (sqlite3_step(stmt) == SQLITE_ROW) {
    bookmarks::Bookmark b;
    b.url           = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
    b.title         = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
    b.timestamp_utc = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
    result.push_back(std::move(b));
  }
  sqlite3_finalize(stmt);
  return result;
}

void SqliteBookmarkRepo::save_all(const std::vector<bookmarks::Bookmark>& bks) {
  // ponytail: full replace on every save; fine at bookmark counts, use upsert if this shows in profiling
  sqlite3_exec(impl_->db, "BEGIN; DELETE FROM bookmarks;", nullptr, nullptr, nullptr);
  sqlite3_stmt* stmt = nullptr;
  sqlite3_prepare_v2(impl_->db,
    "INSERT INTO bookmarks(url, title, timestamp_utc) VALUES(?,?,?);",
    -1, &stmt, nullptr);
  for (const auto& b : bks) {
    sqlite3_bind_text(stmt, 1, b.url.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, b.title.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, b.timestamp_utc.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_reset(stmt);
  }
  sqlite3_finalize(stmt);
  sqlite3_exec(impl_->db, "COMMIT;", nullptr, nullptr, nullptr);
}

}  // namespace drift::storage
