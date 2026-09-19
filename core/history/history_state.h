#pragma once
#include "history/history_entry.h"

#include <optional>
#include <string>
#include <vector>

namespace drift::history {

struct HistoryState {
  std::vector<HistoryEntry> entries;  // most-recent first, capped at 5000
  bool operator==(const HistoryState&) const = default;
};

// Inserts at front. Returns an error message if the URL is not recordable.
[[nodiscard]] std::optional<std::string> add_entry(HistoryState&, HistoryEntry);

void clear_all(HistoryState&);
void remove_entries_for_url(HistoryState&, const std::string& url);

}  // namespace drift::history
