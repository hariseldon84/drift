#pragma once
#include "bookmarks/bookmark.h"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace drift::bookmarks {

struct BookmarkState {
  std::vector<Bookmark> bookmarks;
  bool operator==(const BookmarkState&) const = default;
};

enum class BookmarkErrorCode : std::uint8_t {
  invalid_url   = 0,
  duplicate_url = 1,
  not_found     = 2,
};

struct BookmarkError {
  BookmarkErrorCode code;
  std::string       message;
  bool operator==(const BookmarkError&) const = default;
};

[[nodiscard]] bool has_bookmark(const BookmarkState&, const std::string& url);
[[nodiscard]] std::optional<BookmarkError> add_bookmark(BookmarkState&, Bookmark);
[[nodiscard]] std::optional<BookmarkError> remove_bookmark(BookmarkState&, const std::string& url);

}  // namespace drift::bookmarks
