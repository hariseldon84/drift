#include "drift_win_window.h"
#include "include/cef_browser.h"
#include "include/cef_app.h"
#include <string>

static const wchar_t* kClassName = L"DriftWindow";
static const int      kUrlBarId  = 101;

DriftWinWindow::DriftWinWindow(HINSTANCE hInstance) : hInstance_(hInstance) {}

bool DriftWinWindow::Create(int nCmdShow) {
  WNDCLASSEX wc = {};
  wc.cbSize        = sizeof(wc);
  wc.lpfnWndProc   = WndProc;
  wc.hInstance     = hInstance_;
  wc.hCursor       = LoadCursor(nullptr, IDC_ARROW);
  wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
  wc.lpszClassName = kClassName;
  RegisterClassEx(&wc);

  hwnd_ = CreateWindowEx(0, kClassName, L"Drift",
    WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
    CW_USEDEFAULT, CW_USEDEFAULT, 1280, 800,
    nullptr, nullptr, hInstance_, this);
  if (!hwnd_) return false;

  // URL bar
  urlBar_ = CreateWindowEx(0, L"EDIT", L"https://example.com",
    WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
    0, 0, 1280, kUrlBarHeight, hwnd_,
    reinterpret_cast<HMENU>(kUrlBarId), hInstance_, nullptr);

  // CEF browser fills client area below the URL bar
  RECT cr;
  GetClientRect(hwnd_, &cr);

  client_ = new DriftWinClient([this](const std::string& url) {
    SetWindowTextA(urlBar_, url.c_str());
  });

  CefWindowInfo wi;
  RECT browserRect = {0, kUrlBarHeight, cr.right, cr.bottom};
  wi.SetAsChild(hwnd_, browserRect);

  CefBrowserSettings bs;
  CefBrowserHost::CreateBrowser(wi, client_, "https://example.com", bs, nullptr, nullptr);

  ShowWindow(hwnd_, nCmdShow);
  UpdateWindow(hwnd_);
  return true;
}

void DriftWinWindow::OnSize(int w, int h) {
  if (urlBar_)
    SetWindowPos(urlBar_, nullptr, 0, 0, w, kUrlBarHeight, SWP_NOZORDER);
  if (client_ && client_->engine()) {
    HWND browserHwnd = static_cast<HWND>(client_->engine()->native_view_handle());
    if (browserHwnd)
      SetWindowPos(browserHwnd, nullptr,
        0, kUrlBarHeight, w, h - kUrlBarHeight, SWP_NOZORDER);
  }
}

void DriftWinWindow::Navigate(const std::wstring& url) {
  if (client_ && client_->engine()) {
    int len = WideCharToMultiByte(CP_UTF8, 0, url.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::string narrow(len - 1, '\0');
    WideCharToMultiByte(CP_UTF8, 0, url.c_str(), -1, narrow.data(), len, nullptr, nullptr);
    client_->engine()->navigate(narrow);
  }
}

LRESULT CALLBACK DriftWinWindow::WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
  DriftWinWindow* self = nullptr;
  if (msg == WM_NCCREATE) {
    auto* cs = reinterpret_cast<CREATESTRUCT*>(lp);
    self = reinterpret_cast<DriftWinWindow*>(cs->lpCreateParams);
    SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
    self->hwnd_ = hwnd;
  } else {
    self = reinterpret_cast<DriftWinWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
  }

  switch (msg) {
    case WM_SIZE:
      if (self) self->OnSize(LOWORD(lp), HIWORD(lp));
      break;
    case WM_COMMAND:
      if (LOWORD(wp) == kUrlBarId && HIWORD(wp) == EN_CHANGE) break;
      if (LOWORD(wp) == kUrlBarId) {
        // Enter key in the edit control
        wchar_t buf[2048] = {};
        GetWindowTextW(self->urlBar_, buf, 2048);
        self->Navigate(buf);
      }
      break;
    case WM_DESTROY:
      PostQuitMessage(0);
      break;
    default:
      return DefWindowProc(hwnd, msg, wp, lp);
  }
  return 0;
}
