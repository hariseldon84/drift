#include "browser-state/browser_state.h"
#include "browser-state/browser_state_codec.h"

#include <cstdlib>
#include <cstdint>
#include <iostream>
#include <limits>
#include <string>
#include <variant>
#include <vector>

namespace {

using namespace drift::browser;

int failures = 0;

void check(bool condition, const std::string& message) {
  if (!condition) {
    std::cerr << "FAIL: " << message << '\n';
    ++failures;
  }
}

void require_applied(BrowserState& state, BrowserEvent event) {
  const auto error = apply_event(state, event);
  check(!error.has_value(), "expected event to apply successfully");
}

BrowserState populated_state() {
  BrowserState state;
  require_applied(state, TabCreatedEvent{TabId{1}, "https://one.test", true});
  require_applied(state, TabCreatedEvent{TabId{2}, "https://two.test", true});
  require_applied(state, TabCreatedEvent{TabId{3}, "https://three.test", false});
  require_applied(
      state,
      NavigationStateEvent{
          TabId{2},
          "https://two.test/item",
          "Two",
          true,
          true,
          false});
  require_applied(state, LoadProgressEvent{TabId{2}, 0.625});
  require_applied(state, TabPinnedEvent{TabId{1}, true});
  require_applied(
      state,
      DownloadUpdatedEvent{
          DownloadId{9},
          TabId{2},
          "manual.pdf",
          512,
          1024,
          DownloadStatus::in_progress});
  return state;
}

void test_tab_transitions() {
  BrowserState state = populated_state();

  check(state.tabs.size() == 3, "three tabs are present");
  check(state.active_tab_id == TabId{2}, "second tab is active");
  check(state.tabs[0].pinned, "pin state is retained");
  check(state.tabs[1].title == "Two", "navigation metadata is retained");
  check(state.tabs[1].load_progress == 0.625, "load progress is retained");

  require_applied(state, TabMovedEvent{TabId{3}, 0});
  check(state.tabs[0].id == TabId{3}, "tab move uses stable identity");
  check(state.active_tab_id == TabId{2}, "move does not change active identity");

  require_applied(state, TabClosedEvent{TabId{2}});
  check(state.active_tab_id == TabId{1}, "closing active tab selects the nearest surviving tab");
  check(state.closed_tabs.size() == 1, "closed tab is available for restore");
  check(state.closed_tabs[0].url == "https://two.test/item", "closed URL is retained");
}

void test_invalid_transitions_do_not_mutate() {
  BrowserState state = populated_state();
  const BrowserState before = state;

  const auto duplicate = apply_event(
      state,
      TabCreatedEvent{TabId{1}, "https://duplicate.test", true});
  check(
      duplicate && duplicate->code == StateErrorCode::duplicate_identifier,
      "duplicate tab IDs are rejected");
  check(state == before, "duplicate event leaves state unchanged");

  const auto invalid_progress = apply_event(
      state,
      LoadProgressEvent{TabId{2}, 1.5});
  check(
      invalid_progress &&
          invalid_progress->code == StateErrorCode::invalid_progress,
      "out-of-range progress is rejected");
  check(state == before, "invalid progress leaves state unchanged");

  const auto missing = apply_event(state, TabSelectedEvent{TabId{99}});
  check(
      missing && missing->code == StateErrorCode::tab_not_found,
      "unknown tab IDs are rejected");
  check(state == before, "missing-tab event leaves state unchanged");
}

void test_download_transitions() {
  BrowserState state = populated_state();
  require_applied(
      state,
      DownloadUpdatedEvent{
          DownloadId{9},
          TabId{2},
          "manual.pdf",
          1024,
          1024,
          DownloadStatus::complete});

  check(state.downloads.size() == 1, "download updates preserve identity");
  check(
      state.downloads[0].status == DownloadStatus::complete,
      "download status updates deterministically");

  const BrowserState before = state;
  const auto invalid = apply_event(
      state,
      DownloadUpdatedEvent{
          DownloadId{9},
          TabId{2},
          "manual.pdf",
          2048,
          1024,
          DownloadStatus::in_progress});
  check(
      invalid &&
          invalid->code == StateErrorCode::invalid_download_progress,
      "received bytes cannot exceed a known total");
  check(state == before, "invalid download leaves state unchanged");

  const auto invalid_status = apply_event(
      state,
      DownloadUpdatedEvent{
          DownloadId{9}, TabId{2}, "manual.pdf", 1, 2,
          static_cast<DownloadStatus>(255)});
  check(
      invalid_status &&
          invalid_status->code == StateErrorCode::invalid_download_status,
      "unknown download status is rejected");
  check(state == before, "invalid status leaves state unchanged");
}

void test_request_identifiers() {
  BrowserState state = populated_state();
  require_applied(
      state,
      ExtractionCompletedEvent{RequestId{7}, TabId{2}, "{\"ok\":true}"});
  require_applied(
      state,
      EngineErrorEvent{RequestId{8}, EngineErrorCode::extraction_failed,
                       "retry", true});

  const BrowserState before = state;
  const auto invalid_extraction = apply_event(
      state, ExtractionCompletedEvent{RequestId{}, TabId{2}, "{}"});
  check(
      invalid_extraction &&
          invalid_extraction->code == StateErrorCode::invalid_identifier,
      "zero extraction request ID is rejected");
  check(state == before, "invalid extraction request leaves state unchanged");

  const auto invalid_error = apply_event(
      state,
      EngineErrorEvent{RequestId{}, EngineErrorCode::unknown, "bad", false});
  check(
      invalid_error &&
          invalid_error->code == StateErrorCode::invalid_identifier,
      "zero engine-error request ID is rejected");
  check(state == before, "invalid engine error leaves state unchanged");
}

void test_codec_round_trip_and_versioning() {
  const BrowserState state = populated_state();
  const auto first = encode_browser_state_v1(state);
  const auto second = encode_browser_state_v1(state);

  check(!first.empty(), "encoded state is non-empty");
  check(first == second, "encoding is deterministic");

  const auto decoded = decode_browser_state(first);
  check(
      std::holds_alternative<BrowserState>(decoded),
      "valid state decodes");
  if (std::holds_alternative<BrowserState>(decoded)) {
    check(std::get<BrowserState>(decoded) == state, "codec round-trips state");
  }

  auto unsupported = first;
  if (unsupported.size() >= 8) {
    unsupported[7] = '2';
  }
  const auto unsupported_result = decode_browser_state(unsupported);
  check(
      std::holds_alternative<CodecError>(unsupported_result) &&
          std::get<CodecError>(unsupported_result).code ==
              CodecErrorCode::unsupported_version,
      "unknown versions are rejected explicitly");

  auto truncated = first;
  if (!truncated.empty()) {
    truncated.pop_back();
  }
  const auto truncated_result = decode_browser_state(truncated);
  check(
      std::holds_alternative<CodecError>(truncated_result) &&
          std::get<CodecError>(truncated_result).code ==
              CodecErrorCode::truncated,
      "truncated state is rejected explicitly");

  auto invalid_magic = first;
  if (!invalid_magic.empty()) {
    invalid_magic[0] = 'X';
  }
  const auto invalid_magic_result = decode_browser_state(invalid_magic);
  check(
      std::holds_alternative<CodecError>(invalid_magic_result) &&
          std::get<CodecError>(invalid_magic_result).code ==
              CodecErrorCode::invalid_format,
      "invalid magic is rejected explicitly");

  auto trailing = first;
  trailing.push_back(0);
  const auto trailing_result = decode_browser_state(trailing);
  check(
      std::holds_alternative<CodecError>(trailing_result) &&
          std::get<CodecError>(trailing_result).code ==
              CodecErrorCode::invalid_format,
      "trailing bytes are rejected explicitly");

  BrowserState invalid_state = state;
  invalid_state.tabs[0].load_progress =
      std::numeric_limits<double>::quiet_NaN();
  check(
      encode_browser_state_v1(invalid_state).empty(),
      "invalid in-memory state is not serialized");
}

void test_codec_empty_state_golden_bytes() {
  const std::vector<std::uint8_t> expected{
      'D', 'R', 'I', 'F', 'T', 'B', 'S', '1',
      0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0,
      0, 0, 0, 0,
      0, 0, 0, 0};
  check(
      encode_browser_state_v1(BrowserState{}) == expected,
      "empty-state bytes lock the DRIFTBS1 wire layout");
}

void test_codec_populated_state_golden_bytes() {
  BrowserState state;
  state.tabs.push_back(
      TabState{TabId{1}, "u", "t", true, false, 0.5, true, false});
  state.active_tab_id = TabId{1};
  state.closed_tabs.push_back(ClosedTabState{"c", "d", false});
  state.downloads.push_back(
      DownloadState{DownloadId{2}, TabId{1}, "f", 3, 4,
                    DownloadStatus::complete});

  const std::vector<std::uint8_t> expected{
      'D', 'R', 'I', 'F', 'T', 'B', 'S', '1',
      1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,
      1, 0, 0, 0, 0, 0, 0, 0,
      1, 0, 0, 0, 'u', 1, 0, 0, 0, 't',
      1, 0, 0x20, 0xA1, 0x07, 0, 1, 0,
      1, 0, 0, 0, 1, 0, 0, 0, 'c', 1, 0, 0, 0, 'd', 0,
      1, 0, 0, 0,
      2, 0, 0, 0, 0, 0, 0, 0,
      1, 0, 0, 0, 0, 0, 0, 0,
      1, 0, 0, 0, 'f',
      3, 0, 0, 0, 0, 0, 0, 0,
      4, 0, 0, 0, 0, 0, 0, 0, 1};
  const auto encoded = encode_browser_state_v1(state);
  check(encoded == expected, "populated-state bytes lock every DRIFTBS1 record");

  auto invalid_bool = expected;
  invalid_bool[38] = 2;
  const auto result = decode_browser_state(invalid_bool);
  check(
      std::holds_alternative<CodecError>(result) &&
          std::get<CodecError>(result).code == CodecErrorCode::invalid_value,
      "invalid boolean is distinguished from truncation");

  auto oversized_string = expected;
  oversized_string[28] = 1;
  oversized_string[29] = 0;
  oversized_string[30] = 0;
  oversized_string[31] = 1;
  const auto oversized_result = decode_browser_state(oversized_string);
  check(
      std::holds_alternative<CodecError>(oversized_result) &&
          std::get<CodecError>(oversized_result).code ==
              CodecErrorCode::invalid_value,
      "oversized string is distinguished from truncation");
}

}  // namespace

int main() {
  test_tab_transitions();
  test_invalid_transitions_do_not_mutate();
  test_download_transitions();
  test_request_identifiers();
  test_codec_round_trip_and_versioning();
  test_codec_empty_state_golden_bytes();
  test_codec_populated_state_golden_bytes();

  if (failures != 0) {
    std::cerr << failures << " test assertion(s) failed\n";
    return EXIT_FAILURE;
  }

  std::cout << "browser_state_tests: all assertions passed\n";
  return EXIT_SUCCESS;
}
