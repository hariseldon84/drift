#pragma once
#include <string>

namespace drift::history {

struct HistoryEntry {
  std::string url;
  std::string title;
  std::string timestamp_utc;  // ISO 8601
  bool operator==(const HistoryEntry&) const = default;
};

[[nodiscard]] bool is_recordable_url(const std::string& url);

}  // namespace drift::history
