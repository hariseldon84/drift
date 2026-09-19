#pragma once
#include "bookmarks/bookmark_repository.h"
#include <memory>
#include <string>

namespace drift::storage {

class SqliteBookmarkRepo final : public bookmarks::IBookmarkRepository {
public:
  explicit SqliteBookmarkRepo(const std::string& db_path);
  ~SqliteBookmarkRepo() override;

  std::vector<bookmarks::Bookmark> load_all() override;
  void save_all(const std::vector<bookmarks::Bookmark>&) override;

private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace drift::storage
