#pragma once
#include <string>
#include "include/cef_browser.h"

// C++ bridge — implemented in browser_window.mm (ObjC++)
// All functions run on the CEF UI / main thread.

// Call from DriftApp::OnContextInitialized to create the window and first tab.
void DriftCreateBrowserWindow(const char* initial_url, int width, int height);

// CEF browser lifecycle — call from DriftClient.
void DriftOnBrowserCreated(CefRefPtr<CefBrowser> browser);
void DriftOnBrowserClosed(int browser_id);

// Tab state updates — call from DriftClient display/load handlers.
void DriftOnAddressChange(int browser_id, const char* url);
void DriftOnTitleChange(int browser_id, const char* title);
void DriftOnFaviconURLChange(int browser_id, const char* first_icon_url);
void DriftOnLoadingStateChange(int browser_id, bool loading,
                               bool canGoBack, bool canGoForward);
void DriftOnLoadingProgressChange(int browser_id, double progress);

// Context menu actions — call from DriftClient context menu handler.
void DriftOpenURLInNewTab(const char* url);
void DriftCopyToClipboard(const char* text);

// Download events — call from DriftClient download handler.
void DriftOnDownloadStarted(int download_id, const char* filename,
                            int64_t total_bytes, const char* file_path);
void DriftOnDownloadUpdated(int download_id, int64_t received_bytes,
                            bool is_complete, bool is_canceled);

// Save-As flow: show NSSavePanel, then StartDownload with the chosen path.
void DriftSaveLinkAs(int browser_id, const char* url);
// Returns and clears the NSSavePanel-chosen path for a given URL.
std::string DriftConsumePendingSavePath(const std::string& url);

// Extraction — call from DriftClient::OnLoadEnd and DriftSourceVisitor::Visit.
void DriftOnLoadEnd(int browser_id, const char* url);
void DriftOnExtractionCompleted(int browser_id, const char* request_id, const char* result_json);
