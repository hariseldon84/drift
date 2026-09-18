#include "browser-state/browser_state.h"

#include <algorithm>
#include <cmath>
#include <utility>

namespace drift::browser {
namespace {

template <class... Visitors>
struct Overloaded : Visitors... {
  using Visitors::operator()...;
};

template <class... Visitors>
Overloaded(Visitors...) -> Overloaded<Visitors...>;

auto find_tab(BrowserState& state, TabId id) {
  return std::find_if(
      state.tabs.begin(),
      state.tabs.end(),
      [id](const TabState& tab) { return tab.id == id; });
}

auto find_tab(const BrowserState& state, TabId id) {
  return std::find_if(
      state.tabs.cbegin(),
      state.tabs.cend(),
      [id](const TabState& tab) { return tab.id == id; });
}

StateError error(StateErrorCode code, std::string message) {
  return StateError{code, std::move(message)};
}

std::optional<StateError> require_tab(const BrowserState& state, TabId id) {
  if (!id.valid()) {
    return error(StateErrorCode::invalid_identifier, "tab ID must be non-zero");
  }
  if (find_tab(state, id) == state.tabs.cend()) {
    return error(StateErrorCode::tab_not_found, "tab ID does not exist");
  }
  return std::nullopt;
}

}  // namespace

std::optional<StateError> apply_event(
    BrowserState& state,
    const BrowserEvent& event) {
  return std::visit(
      Overloaded{
          [&state](const TabCreatedEvent& value) -> std::optional<StateError> {
            if (!value.tab_id.valid()) {
              return error(
                  StateErrorCode::invalid_identifier,
                  "created tab ID must be non-zero");
            }
            if (find_tab(state, value.tab_id) != state.tabs.end()) {
              return error(
                  StateErrorCode::duplicate_identifier,
                  "created tab ID already exists");
            }

            state.tabs.push_back(TabState{
                .id = value.tab_id,
                .url = value.url,
                .title = {},
                .pinned = false,
                .loading = false,
                .load_progress = 0.0,
                .can_go_back = false,
                .can_go_forward = false});
            if (value.activate || !state.active_tab_id.has_value()) {
              state.active_tab_id = value.tab_id;
            }
            return std::nullopt;
          },
          [&state](const TabSelectedEvent& value) -> std::optional<StateError> {
            if (auto failure = require_tab(state, value.tab_id)) {
              return failure;
            }
            state.active_tab_id = value.tab_id;
            return std::nullopt;
          },
          [&state](const TabClosedEvent& value) -> std::optional<StateError> {
            if (auto failure = require_tab(state, value.tab_id)) {
              return failure;
            }

            const auto tab = find_tab(state, value.tab_id);
            const auto index = static_cast<std::size_t>(
                std::distance(state.tabs.begin(), tab));
            const bool was_active = state.active_tab_id == value.tab_id;
            state.closed_tabs.insert(
                state.closed_tabs.begin(),
                ClosedTabState{tab->url, tab->title, tab->pinned});
            state.tabs.erase(tab);

            if (state.tabs.empty()) {
              state.active_tab_id.reset();
            } else if (was_active) {
              const auto next_index = std::min(index, state.tabs.size() - 1);
              state.active_tab_id = state.tabs[next_index].id;
            }
            return std::nullopt;
          },
          [&state](const TabMovedEvent& value) -> std::optional<StateError> {
            if (auto failure = require_tab(state, value.tab_id)) {
              return failure;
            }
            if (value.to_index >= state.tabs.size()) {
              return error(
                  StateErrorCode::index_out_of_range,
                  "tab destination index is outside the tab list");
            }

            const auto source = find_tab(state, value.tab_id);
            TabState moving = std::move(*source);
            state.tabs.erase(source);
            state.tabs.insert(
                state.tabs.begin() + value.to_index,
                std::move(moving));
            return std::nullopt;
          },
          [&state](const TabPinnedEvent& value) -> std::optional<StateError> {
            if (auto failure = require_tab(state, value.tab_id)) {
              return failure;
            }
            find_tab(state, value.tab_id)->pinned = value.pinned;
            return std::nullopt;
          },
          [&state](const NavigationStateEvent& value)
              -> std::optional<StateError> {
            if (auto failure = require_tab(state, value.tab_id)) {
              return failure;
            }
            auto& tab = *find_tab(state, value.tab_id);
            tab.url = value.url;
            tab.title = value.title;
            tab.loading = value.loading;
            tab.can_go_back = value.can_go_back;
            tab.can_go_forward = value.can_go_forward;
            return std::nullopt;
          },
          [&state](const LoadProgressEvent& value)
              -> std::optional<StateError> {
            if (auto failure = require_tab(state, value.tab_id)) {
              return failure;
            }
            if (!std::isfinite(value.progress) ||
                value.progress < 0.0 ||
                value.progress > 1.0) {
              return error(
                  StateErrorCode::invalid_progress,
                  "load progress must be finite and between zero and one");
            }
            find_tab(state, value.tab_id)->load_progress = value.progress;
            return std::nullopt;
          },
          [&state](const DownloadUpdatedEvent& value)
              -> std::optional<StateError> {
            if (!value.download_id.valid() ||
                (value.tab_id.has_value() && !value.tab_id->valid())) {
              return error(
                  StateErrorCode::invalid_identifier,
                  "download and optional tab IDs must be non-zero");
            }
            if (value.total_bytes != 0 &&
                value.received_bytes > value.total_bytes) {
              return error(
                  StateErrorCode::invalid_download_progress,
                  "received bytes cannot exceed a known total");
            }
            if (static_cast<std::uint8_t>(value.status) >
                static_cast<std::uint8_t>(DownloadStatus::failed)) {
              return error(
                  StateErrorCode::invalid_download_status,
                  "download status is invalid");
            }

            const auto existing = std::find_if(
                state.downloads.begin(),
                state.downloads.end(),
                [&value](const DownloadState& download) {
                  return download.id == value.download_id;
                });
            const DownloadState replacement{
                value.download_id,
                value.tab_id,
                value.file_name,
                value.received_bytes,
                value.total_bytes,
                value.status};
            if (existing == state.downloads.end()) {
              state.downloads.push_back(replacement);
            } else {
              *existing = replacement;
            }
            return std::nullopt;
          },
          [&state](const ExtractionCompletedEvent& value)
              -> std::optional<StateError> {
            if (!value.request_id.valid()) {
              return error(
                  StateErrorCode::invalid_identifier,
                  "extraction request ID must be non-zero");
            }
            return require_tab(state, value.tab_id);
          },
          [](const EngineErrorEvent& value) -> std::optional<StateError> {
            if (value.request_id.has_value() && !value.request_id->valid()) {
              return error(
                  StateErrorCode::invalid_identifier,
                  "engine error request ID must be non-zero");
            }
            return std::nullopt;
          }},
      event);
}

}  // namespace drift::browser
