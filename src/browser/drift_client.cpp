#include "drift_client.h"
#include "browser_window.h"
#include "include/cef_context_menu_handler.h"
#include "include/cef_download_handler.h"
#include "include/wrapper/cef_helpers.h"

static const int kCmdOpenLinkNewTab  = MENU_ID_USER_FIRST;
static const int kCmdCopyLink        = MENU_ID_USER_FIRST + 1;
static const int kCmdOpenImageNewTab = MENU_ID_USER_FIRST + 2;
static const int kCmdCopyImageURL    = MENU_ID_USER_FIRST + 3;
static const int kCmdSaveLinkAs      = MENU_ID_USER_FIRST + 4;
static const int kCmdSaveImageAs     = MENU_ID_USER_FIRST + 5;

void DriftClient::OnAfterCreated(CefRefPtr<CefBrowser> browser) {
  CEF_REQUIRE_UI_THREAD();
  DriftOnBrowserCreated(browser);
}

bool DriftClient::DoClose(CefRefPtr<CefBrowser> /*browser*/) {
  CEF_REQUIRE_UI_THREAD();
  // ponytail: return true = we own the close; prevents CEF from sending performClose:
  // to our shared parent NSWindow, which would trigger windowShouldClose: on every tab close.
  return true;
}

void DriftClient::OnBeforeClose(CefRefPtr<CefBrowser> browser) {
  CEF_REQUIRE_UI_THREAD();
  DriftOnBrowserClosed(browser->GetIdentifier());
}

void DriftClient::OnAddressChange(CefRefPtr<CefBrowser> browser,
                                  CefRefPtr<CefFrame> frame,
                                  const CefString& url) {
  if (!frame->IsMain()) return;
  DriftOnAddressChange(browser->GetIdentifier(), url.ToString().c_str());
}

void DriftClient::OnTitleChange(CefRefPtr<CefBrowser> browser,
                                const CefString& title) {
  DriftOnTitleChange(browser->GetIdentifier(), title.ToString().c_str());
}

void DriftClient::OnFaviconURLChange(CefRefPtr<CefBrowser> browser,
                                     const std::vector<CefString>& icon_urls) {
  if (!icon_urls.empty())
    DriftOnFaviconURLChange(browser->GetIdentifier(),
                            icon_urls[0].ToString().c_str());
}

void DriftClient::OnLoadingStateChange(CefRefPtr<CefBrowser> browser,
                                       bool isLoading, bool canGoBack,
                                       bool canGoForward) {
  DriftOnLoadingStateChange(browser->GetIdentifier(), isLoading,
                            canGoBack, canGoForward);
}

void DriftClient::OnLoadingProgressChange(CefRefPtr<CefBrowser> browser,
                                          double progress) {
  DriftOnLoadingProgressChange(browser->GetIdentifier(), progress);
}

void DriftClient::OnBeforeContextMenu(CefRefPtr<CefBrowser> browser,
                                      CefRefPtr<CefFrame> /*frame*/,
                                      CefRefPtr<CefContextMenuParams> params,
                                      CefRefPtr<CefMenuModel> model) {
  model->Clear();

  CefString linkUrl = params->GetLinkUrl();
  CefString srcUrl  = params->GetSourceUrl();
  bool hasLink      = !linkUrl.empty();
  bool hasImage     = params->GetMediaType() == CM_MEDIATYPE_IMAGE && !srcUrl.empty();
  bool hasSelection = !params->GetSelectionText().empty();
  bool isEditable   = params->IsEditable();
  bool needSep      = false;

  if (hasLink) {
    model->AddItem(kCmdOpenLinkNewTab, "Open Link in New Tab");
    model->AddItem(kCmdCopyLink,       "Copy Link Address");
    model->AddItem(kCmdSaveLinkAs,     "Save Link As…");
    needSep = true;
  }

  if (hasImage && !hasLink) {
    if (needSep) model->AddSeparator();
    model->AddItem(kCmdOpenImageNewTab, "Open Image in New Tab");
    model->AddItem(kCmdCopyImageURL,    "Copy Image URL");
    model->AddItem(kCmdSaveImageAs,     "Save Image As…");
    needSep = true;
  }

  if (isEditable) {
    if (needSep) model->AddSeparator();
    model->AddItem(MENU_ID_CUT,        "Cut");
    model->AddItem(MENU_ID_COPY,       "Copy");
    model->AddItem(MENU_ID_PASTE,      "Paste");
    model->AddSeparator();
    model->AddItem(MENU_ID_SELECT_ALL, "Select All");
    needSep = true;
  } else if (hasSelection) {
    if (needSep) model->AddSeparator();
    model->AddItem(MENU_ID_COPY, "Copy");
    needSep = true;
  }

  if (needSep) model->AddSeparator();
  model->AddItem(MENU_ID_BACK,    "Back");
  model->AddItem(MENU_ID_FORWARD, "Forward");
  model->AddItem(MENU_ID_RELOAD,  "Reload");
  model->SetEnabled(MENU_ID_BACK,    browser->CanGoBack());
  model->SetEnabled(MENU_ID_FORWARD, browser->CanGoForward());

  if (!isEditable) {
    model->AddSeparator();
    model->AddItem(MENU_ID_SELECT_ALL, "Select All");
  }
}

bool DriftClient::OnContextMenuCommand(CefRefPtr<CefBrowser> browser,
                                       CefRefPtr<CefFrame> /*frame*/,
                                       CefRefPtr<CefContextMenuParams> params,
                                       int command_id,
                                       EventFlags /*event_flags*/) {
  if (command_id == kCmdOpenLinkNewTab) {
    DriftOpenURLInNewTab(params->GetLinkUrl().ToString().c_str());
    return true;
  }
  if (command_id == kCmdCopyLink) {
    DriftCopyToClipboard(params->GetLinkUrl().ToString().c_str());
    return true;
  }
  if (command_id == kCmdOpenImageNewTab) {
    DriftOpenURLInNewTab(params->GetSourceUrl().ToString().c_str());
    return true;
  }
  if (command_id == kCmdCopyImageURL) {
    DriftCopyToClipboard(params->GetSourceUrl().ToString().c_str());
    return true;
  }
  if (command_id == kCmdSaveLinkAs) {
    // Show NSSavePanel first; completion handler calls StartDownload with chosen path
    DriftSaveLinkAs(browser->GetIdentifier(), params->GetLinkUrl().ToString().c_str());
    return true;
  }
  if (command_id == kCmdSaveImageAs) {
    // Show NSSavePanel first; completion handler calls StartDownload with chosen path
    DriftSaveLinkAs(browser->GetIdentifier(), params->GetSourceUrl().ToString().c_str());
    return true;
  }
  return false;  // let CEF handle built-in commands (copy, paste, back, etc.)
}

bool DriftClient::OnBeforeDownload(CefRefPtr<CefBrowser> /*browser*/,
                                   CefRefPtr<CefDownloadItem> download_item,
                                   const CefString& suggested_name,
                                   CefRefPtr<CefBeforeDownloadCallback> callback) {
  // Check if user already chose a path via NSSavePanel (DriftSaveLinkAs flow)
  std::string path = DriftConsumePendingSavePath(download_item->GetURL().ToString());
  if (path.empty()) {
    std::string home = getenv("HOME") ? getenv("HOME") : "";
    path = home + "/Downloads/" + suggested_name.ToString();
  }
  DriftOnDownloadStarted(download_item->GetId(),
                         suggested_name.ToString().c_str(),
                         download_item->GetTotalBytes(),
                         path.c_str());
  callback->Continue(CefString(path), false);
  return true;
}

void DriftClient::OnDownloadUpdated(CefRefPtr<CefBrowser> /*browser*/,
                                    CefRefPtr<CefDownloadItem> download_item,
                                    CefRefPtr<CefDownloadItemCallback> /*callback*/) {
  DriftOnDownloadUpdated(download_item->GetId(),
                         download_item->GetReceivedBytes(),
                         download_item->IsComplete(),
                         download_item->IsCanceled());
}
