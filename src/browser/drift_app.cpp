#include "drift_app.h"
#include "browser_window.h"
#include "include/wrapper/cef_helpers.h"

void DriftApp::OnBeforeCommandLineProcessing(const CefString& process_type,
                                              CefRefPtr<CefCommandLine> command_line) {
  if (process_type.empty()) {
    // ponytail: blanket sandbox disable for unsigned dev builds; replace with per-process
    // entitlements + code signing before any beta/release build
    command_line->AppendSwitch("no-sandbox");
    // ponytail: mock keychain suppresses "Chromium Safe Storage" prompt in unsigned dev builds;
    // remove before any signed release — production uses the real macOS Keychain
    command_line->AppendSwitch("use-mock-keychain");
  }
}

void DriftApp::OnContextInitialized() {
  CEF_REQUIRE_UI_THREAD();
  DriftCreateBrowserWindow("about:blank", 1280, 800);
}
