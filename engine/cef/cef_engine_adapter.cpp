#include "engine/cef/cef_engine_adapter.h"
#include "include/cef_frame.h"

namespace drift::engine {

CefEngineAdapter::CefEngineAdapter(CefRefPtr<CefBrowser> browser)
    : browser_(std::move(browser)) {}

void CefEngineAdapter::navigate(const std::string& url) {
  if (!browser_) return;
  auto frame = browser_->GetMainFrame();
  if (frame) frame->LoadURL(CefString(url));
}

void CefEngineAdapter::go_back()    { if (browser_) browser_->GoBack(); }
void CefEngineAdapter::go_forward() { if (browser_) browser_->GoForward(); }
void CefEngineAdapter::reload()     { if (browser_) browser_->Reload(); }
void CefEngineAdapter::stop()       { if (browser_) browser_->StopLoad(); }

void CefEngineAdapter::close() {
  if (browser_) browser_->GetHost()->CloseBrowser(false);
}

void CefEngineAdapter::start_download(const std::string& url) {
  if (browser_) browser_->GetHost()->StartDownload(CefString(url));
}

void* CefEngineAdapter::native_view_handle() {
  if (!browser_) return nullptr;
  return browser_->GetHost()->GetWindowHandle();
}

int CefEngineAdapter::browser_id() const {
  return browser_ ? browser_->GetIdentifier() : -1;
}

}  // namespace drift::engine
