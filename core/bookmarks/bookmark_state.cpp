#include "bookmarks/bookmark_state.h"

#include <algorithm>

namespace drift::bookmarks {

bool is_bookmarkable_url(const std::string& url) {
  return url.starts_with("http://") ||
         url.starts_with("https://") ||
         url.starts_with("file://");
}

bool has_bookmark(const BookmarkState& state, const std::string& url) {
  return std::any_of(state.bookmarks.begin(), state.bookmarks.end(),
                     [&url](const Bookmark& b) { return b.url == url; });
}

std::optional<BookmarkError> add_bookmark(BookmarkState& state, Bookmark b) {
  if (!is_bookmarkable_url(b.url)) {
    return BookmarkError{BookmarkErrorCode::invalid_url,
                         "URL scheme not bookmarkable: " + b.url};
  }
  if (has_bookmark(state, b.url)) {
    return BookmarkError{BookmarkErrorCode::duplicate_url,
                         "already bookmarked: " + b.url};
  }
  state.bookmarks.push_back(std::move(b));
  return std::nullopt;
}

std::optional<BookmarkError> remove_bookmark(BookmarkState& state,
                                             const std::string& url) {
  auto it = std::find_if(state.bookmarks.begin(), state.bookmarks.end(),
                         [&url](const Bookmark& b) { return b.url == url; });
  if (it == state.bookmarks.end()) {
    return BookmarkError{BookmarkErrorCode::not_found,
                         "bookmark not found: " + url};
  }
  state.bookmarks.erase(it);
  return std::nullopt;
}

}  // namespace drift::bookmarks
