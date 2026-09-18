#pragma once

#include "browser-state/browser_state.h"

#include <cstdint>
#include <string>
#include <variant>
#include <vector>

namespace drift::browser {

enum class CodecErrorCode {
  invalid_format,
  unsupported_version,
  truncated,
  invalid_value,
};

struct CodecError {
  CodecErrorCode code;
  std::string message;
  bool operator==(const CodecError&) const = default;
};

using DecodeResult = std::variant<BrowserState, CodecError>;

[[nodiscard]] std::vector<std::uint8_t> encode_browser_state_v1(
    const BrowserState& state);

[[nodiscard]] DecodeResult decode_browser_state(
    const std::vector<std::uint8_t>& bytes);

}  // namespace drift::browser
