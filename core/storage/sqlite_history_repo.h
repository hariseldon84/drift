#pragma once
#include "history/history_repository.h"
#include <memory>
#include <string>

namespace drift::storage {

class SqliteHistoryRepo final : public history::IHistoryRepository {
public:
  explicit SqliteHistoryRepo(const std::string& db_path);
  ~SqliteHistoryRepo() override;

  std::vector<history::HistoryEntry> load_all() override;
  void save_all(const std::vector<history::HistoryEntry>&) override;

private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace drift::storage
