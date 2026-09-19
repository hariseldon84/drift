#include "bookmarks/bookmark_state.h"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

namespace {

using namespace drift::bookmarks;

int failures = 0;

void check(bool condition, const std::string& message) {
  if (!condition) {
    std::cerr << "FAIL: " << message << '\n';
    ++failures;
  }
}

// ── Minimal JSON helpers ───────────────────────────────────────────────────

// Extract the value of a JSON string field from a single object string.
// Returns "" if not found. Handles only simple (non-escaped) string values.
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

// Split a JSON array string into individual object strings.
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
  check(is_bookmarkable_url("http://example.com"),  "http:// valid");
  check(is_bookmarkable_url("https://example.com"), "https:// valid");
  check(is_bookmarkable_url("file:///path/to/file"), "file:// valid");
  check(!is_bookmarkable_url("about:blank"),   "about:blank rejected");
  check(!is_bookmarkable_url("chrome://settings"), "chrome:// rejected");
  check(!is_bookmarkable_url("data:text/html,x"),  "data: rejected");
  check(!is_bookmarkable_url(""),                   "empty rejected");
}

void test_add_remove() {
  BookmarkState state;
  check(!add_bookmark(state, {"https://a.test", "A", "2026-01-01T00:00:00Z"}).has_value(), "add A ok");
  check(!add_bookmark(state, {"https://b.test", "B", "2026-01-01T00:00:00Z"}).has_value(), "add B ok");
  check(has_bookmark(state, "https://a.test"), "has A");
  check(has_bookmark(state, "https://b.test"), "has B");
  check(!remove_bookmark(state, "https://a.test").has_value(), "remove A ok");
  check(!has_bookmark(state, "https://a.test"), "A gone");
  check(has_bookmark(state, "https://b.test"),  "B still there");
  check(state.bookmarks.size() == 1, "one remaining");
}

void test_duplicates() {
  BookmarkState state;
  check(!add_bookmark(state, {"https://dup.test", "D", ""}).has_value(), "first add ok");
  auto err = add_bookmark(state, {"https://dup.test", "D2", ""});
  check(err.has_value(), "second add returns error");
  check(err && err->code == BookmarkErrorCode::duplicate_url, "duplicate_url code");
  check(state.bookmarks.size() == 1, "still one entry");
}

void test_not_found() {
  BookmarkState state;
  auto err = remove_bookmark(state, "https://missing.test");
  check(err.has_value(), "remove missing returns error");
  check(err && err->code == BookmarkErrorCode::not_found, "not_found code");
}

void test_invalid_url_rejected() {
  BookmarkState state;
  auto err = add_bookmark(state, {"about:blank", "Bad", ""});
  check(err.has_value(), "about:blank returns error");
  check(err && err->code == BookmarkErrorCode::invalid_url, "invalid_url code");
  check(state.bookmarks.empty(), "state unchanged");
}

void test_fixture_compat() {
  // DRIFT_FIXTURES_DIR is set by CMake.
  std::string path = std::string(DRIFT_FIXTURES_DIR) + "/macos/bookmarks-v1.json";
  std::ifstream f(path);
  check(f.is_open(), "fixture file opens: " + path);
  if (!f.is_open()) return;

  std::ostringstream ss;
  ss << f.rdbuf();
  std::string json = ss.str();

  auto objects = split_json_objects(json);
  check(objects.size() == 3, "fixture has 3 raw entries");

  BookmarkState state;
  for (const auto& obj : objects) {
    Bookmark b;
    b.url           = extract_string_field(obj, "url");
    b.title         = extract_string_field(obj, "title");
    b.timestamp_utc = extract_string_field(obj, "timestamp");
    (void)add_bookmark(state, std::move(b));  // silently rejects invalid/duplicate
  }

  check(state.bookmarks.size() == 2, "2 valid bookmarks loaded (about:blank rejected)");
  check(has_bookmark(state, "https://example.com/products/one"), "https entry present");
  check(has_bookmark(state, "file:///Users/example/reference.html"), "file entry present");
  check(!has_bookmark(state, "about:blank"), "about:blank not loaded");
}

}  // namespace

int main() {
  test_url_validation();
  test_add_remove();
  test_duplicates();
  test_not_found();
  test_invalid_url_rejected();
  test_fixture_compat();

  if (failures == 0) {
    std::cout << "All bookmark tests passed.\n";
  }
  return failures != 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}
