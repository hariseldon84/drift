#include "history/history_state.h"

#include <algorithm>

namespace drift::history {

bool is_recordable_url(const std::string& url) {
  return url.starts_with("http://") ||
         url.starts_with("https://") ||
         url.starts_with("file://");
}

std::optional<std::string> add_entry(HistoryState& state, HistoryEntry entry) {
  if (!is_recordable_url(entry.url))
    return "URL not recordable: " + entry.url;
  state.entries.insert(state.entries.begin(), std::move(entry));
  if (state.entries.size() > 5000)
    state.entries.pop_back();
  return std::nullopt;
}

void clear_all(HistoryState& state) {
  state.entries.clear();
}

void remove_entries_for_url(HistoryState& state, const std::string& url) {
  std::erase_if(state.entries,
      [&url](const HistoryEntry& e) { return e.url == url; });
}

}  // namespace drift::history
