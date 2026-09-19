#pragma once
#include "drift_win_client.h"
#include <windows.h>
#include <string>

class DriftWinWindow {
public:
  explicit DriftWinWindow(HINSTANCE hInstance);
  bool Create(int nCmdShow);

private:
  static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
  void OnSize(int w, int h);
  void Navigate(const std::wstring& url);

  HINSTANCE          hInstance_;
  HWND               hwnd_      = nullptr;
  HWND               urlBar_    = nullptr;
  CefRefPtr<DriftWinClient> client_;

  static constexpr int kUrlBarHeight = 36;
};
