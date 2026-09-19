#include "engine/cef/cef_engine_adapter.h"
#include "include/cef_frame.h"
#include "include/cef_string_visitor.h"
#include "intelligence/product_extractor.h"

// Bridge declared in browser_window.h — called on the CEF UI thread after extraction.
void DriftOnExtractionCompleted(int browser_id, const char* request_id, const char* result_json);

namespace {

class DriftSourceVisitor : public CefStringVisitor {
public:
    DriftSourceVisitor(int browser_id, std::string request_id, std::string url)
        : browser_id_(browser_id),
          request_id_(std::move(request_id)),
          url_(std::move(url)) {}

    void Visit(const CefString& string) override {
        auto result = drift::intelligence::extract_product(string.ToString(), url_);
        auto j      = drift::intelligence::to_json(result);
        DriftOnExtractionCompleted(browser_id_, request_id_.c_str(), j.c_str());
    }

private:
    int         browser_id_;
    std::string request_id_;
    std::string url_;
    IMPLEMENT_REFCOUNTING(DriftSourceVisitor);
};

}  // namespace

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

void CefEngineAdapter::request_extraction(const std::string& request_id) {
  if (!browser_) return;
  auto frame = browser_->GetMainFrame();
  if (!frame) return;
  std::string url = frame->GetURL().ToString();
  frame->GetSource(new DriftSourceVisitor(browser_->GetIdentifier(), request_id, url));
}

int CefEngineAdapter::browser_id() const {
  return browser_ ? browser_->GetIdentifier() : -1;
}

}  // namespace drift::engine
