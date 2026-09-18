#pragma once

#include <compare>
#include <cstdint>
#include <optional>
#include <string>
#include <variant>

namespace drift::browser {

struct TabId {
  std::uint64_t value = 0;
  [[nodiscard]] constexpr bool valid() const noexcept { return value != 0; }
  auto operator<=>(const TabId&) const = default;
};

struct DownloadId {
  std::uint64_t value = 0;
  [[nodiscard]] constexpr bool valid() const noexcept { return value != 0; }
  auto operator<=>(const DownloadId&) const = default;
};

struct RequestId {
  std::uint64_t value = 0;
  [[nodiscard]] constexpr bool valid() const noexcept { return value != 0; }
  auto operator<=>(const RequestId&) const = default;
};

struct CreateTabCommand {
  RequestId request_id;
  std::string url;
  bool activate = true;
};
struct SelectTabCommand { TabId tab_id; };
struct CloseTabCommand { TabId tab_id; };
struct MoveTabCommand { TabId tab_id; std::uint32_t to_index = 0; };
struct RestoreClosedTabCommand { RequestId request_id; };
struct NavigateCommand { TabId tab_id; std::string url; };
struct GoBackCommand { TabId tab_id; };
struct GoForwardCommand { TabId tab_id; };
struct ReloadCommand { TabId tab_id; };
struct StopCommand { TabId tab_id; };
struct RequestExtractionCommand { RequestId request_id; TabId tab_id; };
struct CancelDownloadCommand { DownloadId download_id; };

using BrowserCommand = std::variant<
    CreateTabCommand,
    SelectTabCommand,
    CloseTabCommand,
    MoveTabCommand,
    RestoreClosedTabCommand,
    NavigateCommand,
    GoBackCommand,
    GoForwardCommand,
    ReloadCommand,
    StopCommand,
    RequestExtractionCommand,
    CancelDownloadCommand>;

enum class DownloadStatus : std::uint8_t {
  in_progress = 0,
  complete = 1,
  cancelled = 2,
  failed = 3,
};

enum class EngineErrorCode : std::uint16_t {
  unknown = 0,
  invalid_request = 1,
  navigation_failed = 2,
  renderer_terminated = 3,
  download_failed = 4,
  extraction_failed = 5,
};

struct TabCreatedEvent {
  TabId tab_id;
  std::string url;
  bool activate = true;
};
struct TabSelectedEvent { TabId tab_id; };
struct TabClosedEvent { TabId tab_id; };
struct TabMovedEvent { TabId tab_id; std::uint32_t to_index = 0; };
struct TabPinnedEvent { TabId tab_id; bool pinned = false; };
struct NavigationStateEvent {
  TabId tab_id;
  std::string url;
  std::string title;
  bool loading = false;
  bool can_go_back = false;
  bool can_go_forward = false;
};
struct LoadProgressEvent { TabId tab_id; double progress = 0.0; };
struct DownloadUpdatedEvent {
  DownloadId download_id;
  std::optional<TabId> tab_id;
  std::string file_name;
  std::uint64_t received_bytes = 0;
  std::uint64_t total_bytes = 0;
  DownloadStatus status = DownloadStatus::in_progress;
};
struct ExtractionCompletedEvent {
  RequestId request_id;
  TabId tab_id;
  std::string normalized_json;
};
struct EngineErrorEvent {
  std::optional<RequestId> request_id;
  EngineErrorCode code = EngineErrorCode::unknown;
  std::string message;
  bool recoverable = true;
};

using BrowserEvent = std::variant<
    TabCreatedEvent,
    TabSelectedEvent,
    TabClosedEvent,
    TabMovedEvent,
    TabPinnedEvent,
    NavigationStateEvent,
    LoadProgressEvent,
    DownloadUpdatedEvent,
    ExtractionCompletedEvent,
    EngineErrorEvent>;

}  // namespace drift::browser
