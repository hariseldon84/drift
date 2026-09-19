#include "drift_win_app.h"
#include "drift_win_window.h"
#include "include/cef_sandbox_win.h"
#include <windows.h>

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
  CefMainArgs main_args(hInstance);
  CefRefPtr<DriftWinApp> app(new DriftWinApp());

  // Subprocess entry point — returns early for renderer/GPU/utility processes.
  int exit_code = CefExecuteProcess(main_args, app, nullptr);
  if (exit_code >= 0) return exit_code;

  CefSettings settings;
  settings.no_sandbox = true;  // proof build; enable sandbox before beta

  if (!CefInitialize(main_args, settings, app, nullptr)) return 1;

  DriftWinWindow window(hInstance);
  if (!window.Create(nCmdShow)) return 1;

  CefRunMessageLoop();
  CefShutdown();
  return 0;
}
