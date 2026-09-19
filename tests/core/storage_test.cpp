// Round-trip tests for SqliteBookmarkRepo and SqliteHistoryRepo.
// Synthetic data matches tests/fixtures/macos/bookmarks-v1.json and history-v1.json.
#include "storage/sqlite_bookmark_repo.h"
#include "storage/sqlite_history_repo.h"
#include <cassert>
#include <cstdio>
#include <string>
#include <unistd.h>

static std::string tmp_db() {
  char path[] = "/tmp/drift_storage_test_XXXXXX";
  int fd = mkstemp(path);
  close(fd);
  ::remove(path);  // let sqlite3_open create it fresh
  return path;
}

static void test_bookmarks() {
  auto path = tmp_db();
  {
    drift::storage::SqliteBookmarkRepo repo(path);
    assert(repo.load_all().empty());

    std::vector<drift::bookmarks::Bookmark> bks = {
      {"https://example.com/products/one", "Example Product",  "2026-09-18T10:00:00Z"},
      {"https://example.com/second",       "Another Bookmark", "2026-09-18T11:00:00Z"},
    };
    repo.save_all(bks);

    auto loaded = repo.load_all();
    assert(loaded.size() == 2);
    assert(loaded[0].url           == "https://example.com/products/one");
    assert(loaded[0].title         == "Example Product");
    assert(loaded[0].timestamp_utc == "2026-09-18T10:00:00Z");
    assert(loaded[1].url           == "https://example.com/second");
  }
  // Data must survive close/reopen
  {
    drift::storage::SqliteBookmarkRepo repo(path);
    auto loaded = repo.load_all();
    assert(loaded.size() == 2);
    assert(loaded[0].title == "Example Product");
  }
  // save_all replaces all existing rows
  {
    drift::storage::SqliteBookmarkRepo repo(path);
    repo.save_all({{"https://example.com/only", "Only", "2026-09-19T00:00:00Z"}});
    assert(repo.load_all().size() == 1);
  }
  ::remove(path.c_str());
}

static void test_history() {
  auto path = tmp_db();
  {
    drift::storage::SqliteHistoryRepo repo(path);
    assert(repo.load_all().empty());

    std::vector<drift::history::HistoryEntry> hist = {
      {"https://example.com/first",  "First Page",  "2026-09-18T10:04:00Z"},
      {"https://example.com/second", "Second Page", "2026-09-18T10:05:00Z"},
    };
    repo.save_all(hist);

    auto loaded = repo.load_all();
    assert(loaded.size() == 2);
    assert(loaded[0].url           == "https://example.com/first");
    assert(loaded[0].title         == "First Page");
    assert(loaded[0].timestamp_utc == "2026-09-18T10:04:00Z");
    assert(loaded[1].url           == "https://example.com/second");
  }
  // Reopen
  {
    drift::storage::SqliteHistoryRepo repo(path);
    auto loaded = repo.load_all();
    assert(loaded.size() == 2);
    assert(loaded[1].title == "Second Page");
  }
  ::remove(path.c_str());
}

int main() {
  test_bookmarks();
  test_history();
}
