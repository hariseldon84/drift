#include "history/history_state.h"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

namespace {

using namespace drift::history;

int failures = 0;

void check(bool condition, const std::string& message) {
  if (!condition) {
    std::cerr << "FAIL: " << message << '\n';
    ++failures;
  }
}

// ── Minimal JSON helpers (shared pattern from bookmark_test.cpp) ───────────

std::string extract_string_field(const std::string& obj, const std::string& key) {
  std::string needle = "\"" + key + "\"";
  auto pos = obj.find(needle);
  if (pos == std::string::npos) return "";
  pos = obj.find(':', pos + needle.size());
  if (pos == std::string::npos) return "";
  pos = obj.find('"', pos + 1);
  if (pos == std::string::npos) return "";
  auto end = obj.find('"', pos + 1);
  if (end == std::string::npos) return "";
  return obj.substr(pos + 1, end - pos - 1);
}

std::vector<std::string> split_json_objects(const std::string& json) {
  std::vector<std::string> objects;
  int depth = 0;
  std::size_t start = std::string::npos;
  for (std::size_t i = 0; i < json.size(); ++i) {
    if (json[i] == '{') {
      if (depth == 0) start = i;
      ++depth;
    } else if (json[i] == '}') {
      --depth;
      if (depth == 0 && start != std::string::npos) {
        objects.push_back(json.substr(start, i - start + 1));
        start = std::string::npos;
      }
    }
  }
  return objects;
}

// ── Tests ──────────────────────────────────────────────────────────────────

void test_url_validation() {
  check(is_recordable_url("http://example.com"),     "http:// valid");
  check(is_recordable_url("https://example.com"),    "https:// valid");
  check(is_recordable_url("file:///path/to/file"),   "file:// valid");
  check(!is_recordable_url("about:blank"),            "about:blank rejected");
  check(!is_recordable_url("chrome://settings"),      "chrome:// rejected");
  check(!is_recordable_url("data:text/html,x"),       "data: rejected");
  check(!is_recordable_url(""),                       "empty rejected");
}

void test_add_and_order() {
  HistoryState state;
  check(!add_entry(state, {"https://a.test", "A", "2026-01-01T00:00:00Z"}).has_value(), "add A ok");
  check(!add_entry(state, {"https://b.test", "B", "2026-01-01T00:01:00Z"}).has_value(), "add B ok");
  check(state.entries.size() == 2, "two entries");
  check(state.entries[0].url == "https://b.test", "most-recent first");
  check(state.entries[1].url == "https://a.test", "earlier entry second");
}

void test_clear_all() {
  HistoryState state;
  (void)add_entry(state, {"https://x.test", "X", ""});
  (void)add_entry(state, {"https://y.test", "Y", ""});
  clear_all(state);
  check(state.entries.empty(), "cleared");
}

void test_remove_by_url() {
  HistoryState state;
  (void)add_entry(state, {"https://a.test", "A", ""});
  (void)add_entry(state, {"https://a.test", "A again", ""});
  (void)add_entry(state, {"https://b.test", "B", ""});
  remove_entries_for_url(state, "https://a.test");
  check(state.entries.size() == 1, "both a.test entries removed");
  check(state.entries[0].url == "https://b.test", "b.test remains");
}

void test_invalid_url_rejected() {
  HistoryState state;
  auto err = add_entry(state, {"about:blank", "Bad", ""});
  check(err.has_value(), "about:blank returns error");
  check(state.entries.empty(), "state unchanged");
}

void test_cap_at_5000() {
  HistoryState state;
  for (int i = 0; i < 5001; ++i)
    (void)add_entry(state, {"https://x.test/" + std::to_string(i), "", ""});
  check(state.entries.size() == 5000, "capped at 5000");
  check(state.entries.front().url == "https://x.test/5000", "newest at front");
  check(state.entries.back().url == "https://x.test/1", "oldest kept is i=1");
}

void test_fixture_compat() {
  std::string path = std::string(DRIFT_FIXTURES_DIR) + "/macos/history-v1.json";
  std::ifstream f(path);
  check(f.is_open(), "fixture file opens: " + path);
  if (!f.is_open()) return;

  std::ostringstream ss;
  ss << f.rdbuf();
  std::string json = ss.str();

  auto objects = split_json_objects(json);
  check(objects.size() == 2, "fixture has 2 entries");

  // The macOS load path pushes entries to the back (preserving JSON order, which is
  // already most-recent-first). Simulate that here rather than calling add_entry,
  // which inserts at the front and would reverse the order.
  HistoryState state;
  for (const auto& obj : objects) {
    HistoryEntry e;
    e.url           = extract_string_field(obj, "url");
    e.title         = extract_string_field(obj, "title");
    e.timestamp_utc = extract_string_field(obj, "timestamp");
    if (is_recordable_url(e.url))
      state.entries.push_back(std::move(e));
  }

  check(state.entries.size() == 2, "2 entries loaded");
  check(state.entries[0].url == "https://example.com/second", "most-recent at front");
  check(state.entries[1].url == "https://example.com/first",  "older entry follows");
  check(state.entries[0].title == "Second Page", "title preserved");
}

}  // namespace

int main() {
  test_url_validation();
  test_add_and_order();
  test_clear_all();
  test_remove_by_url();
  test_invalid_url_rejected();
  test_cap_at_5000();
  test_fixture_compat();

  if (failures == 0)
    std::cout << "All history tests passed.\n";
  return failures != 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}
