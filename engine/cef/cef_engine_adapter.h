#pragma once
#include "engine/api/browser_engine.h"
#include "include/cef_browser.h"

namespace drift::engine {

class CefEngineAdapter final : public IBrowserEngine {
public:
  explicit CefEngineAdapter(CefRefPtr<CefBrowser> browser);

  void navigate(const std::string& url) override;
  void go_back() override;
  void go_forward() override;
  void reload() override;
  void stop() override;
  void close() override;
  void start_download(const std::string& url) override;
  void* native_view_handle() override;

  // CEF-specific — used only in bridge functions, not on the interface.
  int browser_id() const;

private:
  CefRefPtr<CefBrowser> browser_;
};

}  // namespace drift::engine
