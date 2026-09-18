#pragma once

#include "browser-state/browser_contract.h"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace drift::browser {

struct TabState {
  TabId id;
  std::string url;
  std::string title;
  bool pinned = false;
  bool loading = false;
  double load_progress = 0.0;
  bool can_go_back = false;
  bool can_go_forward = false;
  bool operator==(const TabState&) const = default;
};

struct ClosedTabState {
  std::string url;
  std::string title;
  bool pinned = false;
  bool operator==(const ClosedTabState&) const = default;
};

struct DownloadState {
  DownloadId id;
  std::optional<TabId> tab_id;
  std::string file_name;
  std::uint64_t received_bytes = 0;
  std::uint64_t total_bytes = 0;
  DownloadStatus status = DownloadStatus::in_progress;
  bool operator==(const DownloadState&) const = default;
};

struct BrowserState {
  std::vector<TabState> tabs;
  std::optional<TabId> active_tab_id;
  std::vector<ClosedTabState> closed_tabs;
  std::vector<DownloadState> downloads;
  bool operator==(const BrowserState&) const = default;
};

enum class StateErrorCode {
  invalid_identifier,
  duplicate_identifier,
  tab_not_found,
  index_out_of_range,
  invalid_progress,
  invalid_download_progress,
  invalid_download_status,
};

struct StateError {
  StateErrorCode code;
  std::string message;
  bool operator==(const StateError&) const = default;
};

[[nodiscard]] std::optional<StateError> apply_event(
    BrowserState& state,
    const BrowserEvent& event);

}  // namespace drift::browser
