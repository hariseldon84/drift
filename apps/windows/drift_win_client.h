#pragma once
#include "include/cef_client.h"
#include "include/cef_life_span_handler.h"
#include "include/cef_load_handler.h"
#include "engine/cef/cef_engine_adapter.h"
#include <functional>
#include <memory>

class DriftWinClient : public CefClient,
                       public CefLifeSpanHandler,
                       public CefLoadHandler {
public:
  using TitleCallback = std::function<void(const std::string&)>;
  explicit DriftWinClient(TitleCallback on_title);

  CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override { return this; }
  CefRefPtr<CefLoadHandler>     GetLoadHandler()     override { return this; }

  // CefLifeSpanHandler
  void OnAfterCreated(CefRefPtr<CefBrowser> browser) override;
  bool DoClose(CefRefPtr<CefBrowser> browser) override;

  // CefLoadHandler
  void OnLoadEnd(CefRefPtr<CefBrowser> browser,
                 CefRefPtr<CefFrame> frame,
                 int httpStatusCode) override;

  drift::engine::CefEngineAdapter* engine() { return engine_.get(); }

private:
  TitleCallback on_title_;
  std::unique_ptr<drift::engine::CefEngineAdapter> engine_;
  IMPLEMENT_REFCOUNTING(DriftWinClient);
};
