#include "drift_win_client.h"
#include "include/cef_app.h"

DriftWinClient::DriftWinClient(TitleCallback on_title)
    : on_title_(std::move(on_title)) {}

void DriftWinClient::OnAfterCreated(CefRefPtr<CefBrowser> browser) {
  engine_ = std::make_unique<drift::engine::CefEngineAdapter>(browser);
}

bool DriftWinClient::DoClose(CefRefPtr<CefBrowser>) {
  CefQuitMessageLoop();
  return false;
}

void DriftWinClient::OnLoadEnd(CefRefPtr<CefBrowser> browser,
                               CefRefPtr<CefFrame> frame,
                               int) {
  if (!frame->IsMain()) return;
  if (on_title_) on_title_(browser->GetHost()->GetWindowHandle()
    ? browser->GetMainFrame()->GetURL().ToString() : "");
}
