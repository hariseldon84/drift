#include "browser-state/browser_state_codec.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <string_view>
#include <unordered_set>
#include <utility>

namespace drift::browser {
namespace {

constexpr std::array<std::uint8_t, 8> kMagic{
    'D', 'R', 'I', 'F', 'T', 'B', 'S', '1'};
constexpr std::string_view kMagicPrefix{"DRIFTBS"};
constexpr std::uint32_t kProgressScale = 1'000'000;
constexpr std::uint32_t kMaxCollectionSize = 100'000;
constexpr std::uint32_t kMaxStringSize = 16 * 1024 * 1024;

class Writer {
 public:
  void u8(std::uint8_t value) { bytes_.push_back(value); }

  void u32(std::uint32_t value) {
    for (int shift = 0; shift < 32; shift += 8) {
      u8(static_cast<std::uint8_t>((value >> shift) & 0xff));
    }
  }

  void u64(std::uint64_t value) {
    for (int shift = 0; shift < 64; shift += 8) {
      u8(static_cast<std::uint8_t>((value >> shift) & 0xff));
    }
  }

  bool string(const std::string& value) {
    if (value.size() > std::numeric_limits<std::uint32_t>::max()) {
      return false;
    }
    u32(static_cast<std::uint32_t>(value.size()));
    bytes_.insert(bytes_.end(), value.begin(), value.end());
    return true;
  }

  void magic() { bytes_.insert(bytes_.end(), kMagic.begin(), kMagic.end()); }

  std::vector<std::uint8_t> finish() && { return std::move(bytes_); }

 private:
  std::vector<std::uint8_t> bytes_;
};

class Reader {
 public:
  explicit Reader(const std::vector<std::uint8_t>& bytes) : bytes_(bytes) {}

  bool u8(std::uint8_t& value) {
    if (remaining() < 1) {
      return false;
    }
    value = bytes_[offset_++];
    return true;
  }

  bool u32(std::uint32_t& value) {
    if (remaining() < 4) {
      return false;
    }
    value = 0;
    for (int shift = 0; shift < 32; shift += 8) {
      value |= static_cast<std::uint32_t>(bytes_[offset_++]) << shift;
    }
    return true;
  }

  bool u64(std::uint64_t& value) {
    if (remaining() < 8) {
      return false;
    }
    value = 0;
    for (int shift = 0; shift < 64; shift += 8) {
      value |= static_cast<std::uint64_t>(bytes_[offset_++]) << shift;
    }
    return true;
  }

  bool string(std::string& value) {
    std::uint32_t size = 0;
    if (!u32(size)) {
      return false;
    }
    if (size > kMaxStringSize) {
      invalid_value_ = true;
      return false;
    }
    if (remaining() < size) {
      return false;
    }
    value.assign(
        reinterpret_cast<const char*>(bytes_.data() + offset_),
        size);
    offset_ += size;
    return true;
  }

  [[nodiscard]] std::size_t remaining() const {
    return bytes_.size() - offset_;
  }

  void mark_invalid_value() { invalid_value_ = true; }
  [[nodiscard]] bool invalid_value() const { return invalid_value_; }

 private:
  const std::vector<std::uint8_t>& bytes_;
  std::size_t offset_ = kMagic.size();
  bool invalid_value_ = false;
};

CodecError codec_error(CodecErrorCode code, std::string message) {
  return CodecError{code, std::move(message)};
}

bool read_bool(Reader& reader, bool& value) {
  std::uint8_t raw = 0;
  if (!reader.u8(raw)) {
    return false;
  }
  if (raw > 1) {
    reader.mark_invalid_value();
    return false;
  }
  value = raw == 1;
  return true;
}

bool write_count(Writer& writer, std::size_t count) {
  if (count > kMaxCollectionSize) {
    return false;
  }
  writer.u32(static_cast<std::uint32_t>(count));
  return true;
}

bool can_encode(const BrowserState& state) {
  if (state.tabs.size() > kMaxCollectionSize ||
      state.closed_tabs.size() > kMaxCollectionSize ||
      state.downloads.size() > kMaxCollectionSize) {
    return false;
  }

  std::unordered_set<std::uint64_t> tab_ids;
  for (const auto& tab : state.tabs) {
    if (!tab.id.valid() || !tab_ids.insert(tab.id.value).second ||
        tab.url.size() > kMaxStringSize ||
        tab.title.size() > kMaxStringSize ||
        !std::isfinite(tab.load_progress) ||
        tab.load_progress < 0.0 ||
        tab.load_progress > 1.0) {
      return false;
    }
  }
  if (state.active_tab_id.has_value() &&
      !tab_ids.contains(state.active_tab_id->value)) {
    return false;
  }

  for (const auto& tab : state.closed_tabs) {
    if (tab.url.size() > kMaxStringSize || tab.title.size() > kMaxStringSize) {
      return false;
    }
  }

  std::unordered_set<std::uint64_t> download_ids;
  for (const auto& download : state.downloads) {
    if (!download.id.valid() ||
        !download_ids.insert(download.id.value).second ||
        (download.tab_id.has_value() && !download.tab_id->valid()) ||
        download.file_name.size() > kMaxStringSize ||
        static_cast<std::uint8_t>(download.status) >
            static_cast<std::uint8_t>(DownloadStatus::failed) ||
        (download.total_bytes != 0 &&
         download.received_bytes > download.total_bytes)) {
      return false;
    }
  }
  return true;
}

}  // namespace

std::vector<std::uint8_t> encode_browser_state_v1(
    const BrowserState& state) {
  if (!can_encode(state)) {
    return {};
  }

  Writer writer;
  writer.magic();
  writer.u64(state.active_tab_id.value_or(TabId{}).value);

  if (!write_count(writer, state.tabs.size())) {
    return {};
  }
  for (const auto& tab : state.tabs) {
    writer.u64(tab.id.value);
    if (!writer.string(tab.url) || !writer.string(tab.title)) {
      return {};
    }
    writer.u8(tab.pinned ? 1 : 0);
    writer.u8(tab.loading ? 1 : 0);
    writer.u32(static_cast<std::uint32_t>(
        std::llround(tab.load_progress * kProgressScale)));
    writer.u8(tab.can_go_back ? 1 : 0);
    writer.u8(tab.can_go_forward ? 1 : 0);
  }

  if (!write_count(writer, state.closed_tabs.size())) {
    return {};
  }
  for (const auto& tab : state.closed_tabs) {
    if (!writer.string(tab.url) || !writer.string(tab.title)) {
      return {};
    }
    writer.u8(tab.pinned ? 1 : 0);
  }

  if (!write_count(writer, state.downloads.size())) {
    return {};
  }
  for (const auto& download : state.downloads) {
    writer.u64(download.id.value);
    writer.u64(download.tab_id.value_or(TabId{}).value);
    if (!writer.string(download.file_name)) {
      return {};
    }
    writer.u64(download.received_bytes);
    writer.u64(download.total_bytes);
    writer.u8(static_cast<std::uint8_t>(download.status));
  }

  return std::move(writer).finish();
}

DecodeResult decode_browser_state(const std::vector<std::uint8_t>& bytes) {
  if (bytes.size() < kMagic.size()) {
    return codec_error(CodecErrorCode::truncated, "state header is truncated");
  }
  if (!std::equal(kMagicPrefix.begin(), kMagicPrefix.end(), bytes.begin())) {
    return codec_error(CodecErrorCode::invalid_format, "state magic is invalid");
  }
  if (bytes[7] != kMagic[7]) {
    return codec_error(
        CodecErrorCode::unsupported_version,
        "browser state version is not supported");
  }

  Reader reader(bytes);
  BrowserState state;
  std::uint64_t active_id = 0;
  if (!reader.u64(active_id)) {
    return codec_error(CodecErrorCode::truncated, "active tab ID is truncated");
  }
  if (active_id != 0) {
    state.active_tab_id = TabId{active_id};
  }

  std::uint32_t tab_count = 0;
  if (!reader.u32(tab_count)) {
    return codec_error(CodecErrorCode::truncated, "tab count is truncated");
  }
  if (tab_count > kMaxCollectionSize) {
    return codec_error(CodecErrorCode::invalid_value, "tab count is too large");
  }

  std::unordered_set<std::uint64_t> tab_ids;
  state.tabs.reserve(tab_count);
  for (std::uint32_t index = 0; index < tab_count; ++index) {
    TabState tab;
    std::uint32_t progress = 0;
    if (!reader.u64(tab.id.value) ||
        !reader.string(tab.url) ||
        !reader.string(tab.title) ||
        !read_bool(reader, tab.pinned) ||
        !read_bool(reader, tab.loading) ||
        !reader.u32(progress) ||
        !read_bool(reader, tab.can_go_back) ||
        !read_bool(reader, tab.can_go_forward)) {
      return codec_error(
          reader.invalid_value() ? CodecErrorCode::invalid_value
                                 : CodecErrorCode::truncated,
          reader.invalid_value() ? "tab record contains an invalid value"
                                 : "tab record is truncated");
    }
    if (!tab.id.valid() || !tab_ids.insert(tab.id.value).second ||
        progress > kProgressScale) {
      return codec_error(CodecErrorCode::invalid_value, "tab record is invalid");
    }
    tab.load_progress = static_cast<double>(progress) / kProgressScale;
    state.tabs.push_back(std::move(tab));
  }

  if (state.active_tab_id.has_value() &&
      !tab_ids.contains(state.active_tab_id->value)) {
    return codec_error(
        CodecErrorCode::invalid_value,
        "active tab ID is not present in the tab list");
  }

  std::uint32_t closed_count = 0;
  if (!reader.u32(closed_count)) {
    return codec_error(CodecErrorCode::truncated, "closed-tab count is truncated");
  }
  if (closed_count > kMaxCollectionSize) {
    return codec_error(
        CodecErrorCode::invalid_value,
        "closed-tab count is too large");
  }
  state.closed_tabs.reserve(closed_count);
  for (std::uint32_t index = 0; index < closed_count; ++index) {
    ClosedTabState tab;
    if (!reader.string(tab.url) ||
        !reader.string(tab.title) ||
        !read_bool(reader, tab.pinned)) {
      return codec_error(
          reader.invalid_value() ? CodecErrorCode::invalid_value
                                 : CodecErrorCode::truncated,
          reader.invalid_value()
              ? "closed-tab record contains an invalid value"
              : "closed-tab record is truncated");
    }
    state.closed_tabs.push_back(std::move(tab));
  }

  std::uint32_t download_count = 0;
  if (!reader.u32(download_count)) {
    return codec_error(CodecErrorCode::truncated, "download count is truncated");
  }
  if (download_count > kMaxCollectionSize) {
    return codec_error(
        CodecErrorCode::invalid_value,
        "download count is too large");
  }

  std::unordered_set<std::uint64_t> download_ids;
  state.downloads.reserve(download_count);
  for (std::uint32_t index = 0; index < download_count; ++index) {
    DownloadState download;
    std::uint64_t tab_id = 0;
    std::uint8_t status = 0;
    if (!reader.u64(download.id.value) ||
        !reader.u64(tab_id) ||
        !reader.string(download.file_name) ||
        !reader.u64(download.received_bytes) ||
        !reader.u64(download.total_bytes) ||
        !reader.u8(status)) {
      return codec_error(
          reader.invalid_value() ? CodecErrorCode::invalid_value
                                 : CodecErrorCode::truncated,
          reader.invalid_value() ? "download record contains an invalid value"
                                 : "download record is truncated");
    }
    if (!download.id.valid() ||
        !download_ids.insert(download.id.value).second ||
        status > static_cast<std::uint8_t>(DownloadStatus::failed) ||
        (download.total_bytes != 0 &&
         download.received_bytes > download.total_bytes)) {
      return codec_error(
          CodecErrorCode::invalid_value,
          "download record is invalid");
    }
    if (tab_id != 0) {
      download.tab_id = TabId{tab_id};
    }
    download.status = static_cast<DownloadStatus>(status);
    state.downloads.push_back(std::move(download));
  }

  if (reader.remaining() != 0) {
    return codec_error(
        CodecErrorCode::invalid_format,
        "browser state has trailing bytes");
  }
  return state;
}

}  // namespace drift::browser
