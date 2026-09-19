#pragma once
#include <string>

namespace drift::engine {

// Platform-neutral interface for a single browser tab's rendering engine.
// macOS: backed by CefEngineAdapter (CEF 152)
// Windows: backed by CefEngineAdapter (same CEF build)
// Android: backed by WebViewAdapter (EP-00-T12)
// iPhone: backed by WKWebViewAdapter (EP-00-T13)
class IBrowserEngine {
public:
  virtual ~IBrowserEngine() = default;

  virtual void navigate(const std::string& url) = 0;
  virtual void go_back() = 0;
  virtual void go_forward() = 0;
  virtual void reload() = 0;
  virtual void stop() = 0;
  virtual void close() = 0;
  virtual void start_download(const std::string& url) = 0;

  // Trigger async extraction of the current page. Result delivered via
  // DriftOnExtractionCompleted bridge (browser process UI thread).
  virtual void request_extraction(const std::string& request_id) = 0;

  // Platform-native view handle for embedding.
  // macOS/Windows: NSView* / HWND cast via the caller's platform bridge.
  // Returns nullptr if the engine is not yet initialised.
  [[nodiscard]] virtual void* native_view_handle() = 0;
};

}  // namespace drift::engine
