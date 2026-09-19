#pragma once
#include "include/cef_app.h"

class DriftWinApp : public CefApp, public CefBrowserProcessHandler {
public:
  CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() override {
    return this;
  }
  IMPLEMENT_REFCOUNTING(DriftWinApp);
};
