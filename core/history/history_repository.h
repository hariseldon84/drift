#pragma once
#include "history/history_entry.h"
#include <vector>

namespace drift::history {

class IHistoryRepository {
public:
  virtual ~IHistoryRepository() = default;
  virtual std::vector<HistoryEntry> load_all() = 0;
  virtual void save_all(const std::vector<HistoryEntry>&) = 0;
};

}  // namespace drift::history
