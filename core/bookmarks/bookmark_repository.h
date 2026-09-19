#pragma once
#include "bookmarks/bookmark.h"
#include <vector>

namespace drift::bookmarks {

class IBookmarkRepository {
public:
  virtual ~IBookmarkRepository() = default;
  virtual std::vector<Bookmark> load_all() = 0;
  virtual void save_all(const std::vector<Bookmark>&) = 0;
};

}  // namespace drift::bookmarks
