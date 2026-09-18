#pragma once
#include "include/cef_app.h"

class DriftApp : public CefApp, public CefBrowserProcessHandler {
 public:
  // CefApp
  CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() override {
    return this;
  }

  // ponytail: --disable-gpu-sandbox only; remove once the app is code-signed with GPU entitlements
  void OnBeforeCommandLineProcessing(const CefString& process_type,
                                     CefRefPtr<CefCommandLine> command_line) override;

  // CefBrowserProcessHandler — called once CEF is ready to create windows
  void OnContextInitialized() override;

 private:
  IMPLEMENT_REFCOUNTING(DriftApp);
};
