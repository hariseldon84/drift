#pragma once
#include <string>

namespace drift::bookmarks {

struct Bookmark {
  std::string url;
  std::string title;
  std::string timestamp_utc;  // ISO 8601
};

[[nodiscard]] bool is_bookmarkable_url(const std::string& url);

}  // namespace drift::bookmarks
