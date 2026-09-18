# Drift — Bug Log

> One entry per bug. Never deleted. Format: BUG-NN | Task | Status | Description.

---

## BUG-01 | EP-01-T03 | FIXED 2026-09-16

**Address bar Enter navigates to the current URL instead of what the user typed.**

Root cause: `urlFieldAction:` read `_urlField.stringValue`, which returns the NSTextField's stored attributed display string (`example.com/`) — not the field editor's live content (`google.com`). NSTextField only syncs the field editor back to its stored value on end-editing, but the action fires before that sync.

Secondary symptoms (all resolved by the fix):
- After Enter, display showed old URL selected (attributed string re-applied while field editor was still active)
- Cursor position visually misaligned from editor position (display showed `example.com/`, editor had `https://example.com/`, click positions did not match)
- ⌘A appeared not to work (editor had more content than the visible display suggested)
- Backspace deleted wrong character for the same alignment reason

Fix: read from `[_urlField currentEditor].string` in `urlFieldAction:`. Guard `refreshNavBar` with `if ([_urlField currentEditor]) return` to prevent clobbering the field editor display while the user is typing.

---

## BUG-02 | EP-01-T03 | FIXED 2026-09-16

**Typing inserts at cursor instead of replacing; ⌘A appears broken.**

Root cause: `[editor selectAll:nil]` called synchronously inside `controlTextDidBeginEditing:` silently fails — the field editor is not yet fully installed as first responder at that point in the delegate callback. Typing then inserts at the click position inside the existing URL text instead of replacing it.

Secondary cause: `attributedStringValue` used for display left formatting state in the text system that diverged from field editor content, causing overlap artefacts (`https://examgoogle.com/`).

Fix: (1) Switch `refreshNavBar` to plain `stringValue` — no attributed string. (2) Defer `selectAll:` via `dispatch_async(dispatch_get_main_queue(), ...)` so it runs after the field editor is fully established as first responder.

---

## BUG-03 | EP-01-T03 | FIXED 2026-09-16

**Red close button hangs — window never closes.**

Root cause: `windowShouldClose:` moved ALL tabs to `closingTabs`, including newly opened tabs whose CEF browser was not yet assigned (`browserId = -1`). `CloseBrowser` was only called for browser-backed tabs, so browser-less tabs sat in `closingTabs` forever with no `DriftOnBrowserClosed` ever firing. The exit condition (`closingTabs.count == 0`) was never satisfied → `CefQuitMessageLoop` was never called.

Fix: only add a tab to `closingTabs` if it has a live browser. Browser-less tabs need no async cleanup — drop them immediately. If no browser-backed tabs remain after the sweep, call `CefQuitMessageLoop` directly.

---

## BUG-04 | EP-01-T06 | FIXED 2026-09-17

**⌘⇧T does nothing after closing a blank tab.**

Root cause: `tabStrip:didCloseTabAtIndex:` filtered out `about:blank` URLs before pushing to `_closedTabURLs`. New tabs opened via ⌘T have `about:blank` as their URL, so closing them pushed nothing to the stack. `reopenLastClosedTab` then found an empty array and returned silently.

Fix: remove the `about:blank` guard — push all non-empty URLs including `about:blank` to `_closedTabURLs`.

---

## BUG-05 | EP-01-T06 | FIXED 2026-09-17

**Closing any tab closes the entire browser.**

Root cause: `DoClose` returned `false`, which instructs CEF to send a native OS close event to the browser's parent `NSWindow`. Because all embedded tab browsers share the same parent window (our main `NSWindow`), every tab close fired `windowShouldClose:` → `[NSApp terminate:nil]`.

Fix: return `true` from `DoClose`. This tells CEF the app owns the close sequence and suppresses the `performClose:` call to the parent window. `OnBeforeClose` still fires as part of CEF's internal browser destruction, so `DriftOnBrowserClosed` continues to work correctly.
---

## BUG-06 | EP-01-T06 | FIXED 2026-09-17

**⌘⇧T monitor check fails — event passes through to CEF which ignores it.**

Root cause: `NSEvent.charactersIgnoringModifiers` returns `"T"` (uppercase) when Shift is held, because Shift alters the base character mapping even though it is treated as a modifier flag. Our check `[ch isEqualToString:@"t"]` failed for Shift+T, so the monitor returned the event unmodified and CEF processed it (doing nothing).

Fix: `[ch.lowercaseString isEqualToString:@"t"]` so both `"t"` and `"T"` match.

---

## BUG-07 | EP-01-T01 | FIXED 2026-09-17

**"Drift wants to use Chromium Safe Storage" keychain prompt on every launch.**

Root cause: CEF requests its `Chromium Safe Storage` encryption key from macOS Keychain on every start. Unsigned dev builds have a changing code signature, so macOS treats Drift as a new app each time and re-prompts.

Fix: `--use-mock-keychain` in `OnBeforeCommandLineProcessing` → CEF uses in-memory mock keychain, no prompt. Remove before any signed release build.

---

## BUG-08 | EP-01-T07 | FIXED 2026-09-17

**First character typed in URL bar gets selected; second character replaces it ("techcrunch" → "echcrunch").**

Root cause: `dispatch_async(dispatch_get_main_queue(), selectAll:)` in `controlTextDidBeginEditing:` fires AFTER the user types their first character. The deferred `selectAll:` then selects the typed "t"; the next keystroke replaces it.

Fix: `[editor setSelectedRange:NSMakeRange(0, editor.string.length)]` synchronously in the delegate callback. `setSelectedRange:` works without first-responder status; no deferred dispatch needed.

---

## BUG-09 | EP-01-T07 | FIXED 2026-09-17

**URL bar goes blank after navigation — text reappears only after clicking the bar.**

Root cause: `refreshNavBar` set `_urlField.textColor = clearColor` to hide field text behind the display label overlay. If `_urlDisplayLabel` is ever left hidden (e.g. a rapid tab-switch sequence), the URL bar shows nothing.

Fix: Use `--drift-text-secondary` instead of `clearColor`. Display label still renders styled text on top normally. If the label is ever missing, field text is visible as a readable fallback.

---

## BUG-10 | EP-01-T06 | FIXED 2026-09-17

**⌃Tab / ⌃⇧Tab does not cycle tabs.**

Root cause: Not in the keyboard monitor.

Fix: Add keyCode 48 (Tab) + `NSEventModifierFlagControl` to the monitor. ⌃Tab → next tab (wraps); ⌃⇧Tab → previous tab (wraps).

---

## BUG-11 | EP-01-T02 | FIXED 2026-09-18

**Tab strip X on last tab and ⌘W do nothing when only one tab is open. Only ⌘Q closes Drift.**

Decision (Anand, 2026-09-18): Chrome-style — closing the last tab quits Drift.

Root cause: the previous flow called `tab.browser->GetHost()->CloseBrowser(false)` for the last tab and relied on the async `OnBeforeClose` → `DriftOnBrowserClosed` callback to invoke `CefQuitMessageLoop()`. `CefQuitMessageLoop` exits CEF's internal message pump but does not reliably terminate NSApp's run loop when the app is running under the standard macOS event loop, so the window never closes.

Fix: In `tabStrip:didCloseTabAtIndex:`, when `_tabs.count == 1`, short-circuit to `[NSApp terminate:nil]` — the same path ⌘Q takes. `DriftApplication.terminate:` then calls `CefQuitMessageLoop()` at the point where NSApp is already unwinding, so both loops exit cleanly. Applies uniformly to tab-strip X, ⌘W (`closeActiveTab`), and Close Tab context menu.

---

## BUG-12 | EP-01-T03 | FIXED 2026-09-17

**Clicking URL bar does not select all — user must manually delete existing URL text.**

Root cause: `setSelectedRange:` in `controlTextDidBeginEditing:` fires before the NSTextField click handler repositions the cursor to the click point. The click handler runs after the delegate callback, overriding the selection with a cursor at the clicked position.

Fix: Add `DriftURLField : NSTextField` subclass that overrides `mouseDown:`. Records `wasEditing` before calling `[super mouseDown:e]` (which positions cursor). If the field wasn't already in edit mode, overrides with `selectAll` on the now-installed field editor. Remove `setSelectedRange:` from `controlTextDidBeginEditing:`. Update `focusAddressBar` (⌘L) to call `setSelectedRange:` synchronously after `makeFirstResponder:` — no race because no mouse click is involved.

---

## BUG-13 | EP-01-T03 | FIXED 2026-09-17

**Long URLs appear as jibberish — text overflows the URL pill without truncation.**

Root cause: `_urlDisplayLabel` had no `usesSingleLineMode` or `lineBreakMode` set, so NSTextField wrapped or clipped text in unexpected ways at the cell level.

Fix: `_urlDisplayLabel.cell.usesSingleLineMode = YES` and `[_urlDisplayLabel.cell setLineBreakMode:NSLineBreakByTruncatingTail]` in initialisation.

---

## BUG-14 | EP-01-T03 | FIXED 2026-09-17

**All tabs show the same URL after navigating — switching tabs does not update the URL bar.**

Root cause: `refreshNavBar` has a guard `if ([_urlField currentEditor]) return` to avoid clobbering an active field editor. When the user visits a URL and then switches tabs while the URL bar still has the field editor active, the guard fires and the new tab's URL is never displayed.

Fix: At the start of `switchToIndex:`, call `[_window makeFirstResponder:nil]` if the URL field has an active editor. This resigns edit mode before the tab switch, allowing `refreshNavBar` to update the display label correctly.

---

## BUG-15 | EP-01-T07 | FIXED 2026-09-18

**Jibberish in URL bar — two URLs visually overlapping when switching tabs.**

Root cause: `_urlField.textColor` was set to `--drift-text-secondary` (brownish) as a fallback when the display label is shown. Because `_urlDisplayLabel` has `drawsBackground = NO`, the field's brownish text bleeds through behind the label's attributed text — two strings visible simultaneously.

Fix: `_urlField.textColor = [NSColor clearColor]` whenever the display label is shown. The label renders the fully-styled URL on top; the field text is invisible.

---

## BUG-16 | EP-01-T03 | FIXED 2026-09-18

**New tab shows "about:blank" text in URL bar instead of an empty field.**

Root cause: `refreshNavBar` unconditionally set `_urlField.stringValue = url`, including when url = "about:blank". The display label rendered "about" as host fallback on top.

Fix: In `refreshNavBar`, if url is empty or "about:blank", set `_urlField.stringValue = @""`, hide display label and lock icon, return early. Added `_urlField.placeholderString = @"Search or type a URL"` for the empty state.

---

## BUG-17 | EP-01-T01 | FIXED 2026-09-18

**New tab does not auto-focus the URL bar — user must click to type.**

Root cause: `openTabWithURL:` had no call to focus the URL bar after switching to the new tab.

Fix: After `switchToIndex:` in `openTabWithURL:`, if url is "about:blank", call `focusAddressBar`. Cursor lands in the empty URL bar immediately — Chrome behaviour.

---

## BUG-18 | EP-01-T03 | FIXED 2026-09-18

**URL bar turns blue/shaded after typing a URL in a new tab and pressing Enter.**

Root cause: race between user input speed and CEF's async `DriftOnAddressChange("about:blank")` callback.

Sequence: (1) New tab opens → `urlBarFocusPending = YES`. (2) User types fast and presses Enter → `urlFieldAction:` calls `makeFirstResponder:nil` → field editor dismissed. (3) CEF fires `DriftOnAddressChange("about:blank")` late — after the user has already navigated. (4) `urlBarFocusPending` is still YES, `currentEditor` is nil → `focusAddressBar` fires → URL field becomes first responder again. (5) Then `DriftOnAddressChange("https://microsoft.com")` arrives → `refreshNavBar` sees `currentEditor != nil` → returns early (guard for "user is typing") → display label never shown → field editor stays active, rendering blue.

Fix: Clear `_urlBarFocusPending = NO` in `controlTextDidEndEditing:`. The moment the user dismisses the URL bar (Enter, Escape, click elsewhere), the pending focus is cancelled. Any late-firing `about:blank` address change no longer re-claims first responder.

---

## BUG-19 | EP-01-T07 | FIXED 2026-09-18

**URL bar text invisible when typing / navigating — blue selection strip visible but no text. Pattern of BUG-15 and BUG-18 recurring after SP-03.**

Root cause (architectural, not a one-off): the URL pill uses TWO overlapping text renderers — the editable `_urlField` and the `DriftURLDisplayLabel` overlay on top. When idle, `_urlField.textColor = clearColor` makes field text invisible so the styled display label (host in dark + path in secondary) can render cleanly on top without bleed-through (BUG-15's fix).

The gap: when the field enters edit mode, `controlTextDidBeginEditing:` sets the editor's `textStorage` attributes to dark. But `_urlField.textColor` remains `clearColor`. The NSTextView field editor can inherit the field's `textColor` momentarily during setup — before our attribute override — leaving text drawn in `clearColor`. Any interaction that opens the field editor via an unusual path (SP-03's star button adjacent to the field being one such trigger) can expose the race.

Why it kept coming back: every previous fix patched a specific *symptom path* (BUG-09 fallback colour, BUG-15 clearColor rule, BUG-18 focus flag clearing). None fixed the underlying invariant — "field.textColor must be visible whenever the editor is active". SP-03's star button added a new nearby responder that surfaced the race again.

Fix: In `controlTextDidBeginEditing:`, also set `_urlField.textColor = editColor` (dark) alongside the editor attribute overrides. The field's own colour is now dark whenever it's in edit mode, so the editor inherits a visible colour regardless of timing. `refreshNavBar` resets to `clearColor` when it next shows the display label.

---

## BUG-20 | EP-01-T09 | FIXED 2026-09-18

**"about:blank" appears as a bookmark in the bookmarks bar.**

Root cause: `toggleBookmark:` guard was `if (!t.url.length || [t.url isEqualToString:@"about:blank"]) return;` — exact string match only. Non-http schemes like `chrome://` bypassed the guard. Additionally, if a user pressed ⌘D while a page was mid-navigation and `t.title` happened to be `"about:blank"`, the bookmark's *title* would show as `"about:blank"` in the bar even though the URL was real.

Fix: Positive-list guard — only bookmark URLs whose scheme is `http://`, `https://`, or `file://`. Everything else (about:*, chrome:*, data:*, empty) is rejected. Cleaner and future-proof.

---

## BUG-21 | EP-01-T11 | FIXED 2026-09-18

**Download bar shows garbage progress like "0 / 4666 B" or wrong unit.**

Root cause: hard-coded KB formatting via integer division. For a 4666-byte file, `4666 / 1024 = 4` → "0 / 4 KB". Confusing at all size ranges.

Fix: Human-readable byte formatter (block-local `fmt`): B under 1 KB, `%.1f KB` under 1 MB, `%.1f MB` under 1 GB, `%.2f GB` above. Also improved status states: `"Waiting…"` when no bytes yet, `"X / Y"` while downloading, `"Complete — Y"` when done.

---

## BUG-22 | EP-01-T11 | FIXED 2026-09-18

**No "Save Link As…" or "Save Image As…" in right-click context menu — user cannot download a PDF/file that opens inline in the browser viewer.**

Root cause: `DriftClient::OnBeforeContextMenu` only offered "Open Link in New Tab" and "Copy Link Address" for links. No path to trigger a download.

Fix: Added two commands `kCmdSaveLinkAs` and `kCmdSaveImageAs`. Menu shows "Save Link As…" whenever the click target has a link URL, "Save Image As…" for images. Handler calls `browser->GetHost()->StartDownload(url)` which routes through `CefDownloadHandler` → `OnBeforeDownload` → download bar. Matches Chrome behaviour.
---

## BUG-23 | EP-01-T09 | FIXED 2026-09-18

**about:blank appears in bookmarks bar even after positive-list guard was added to toggleBookmark:.**

Root cause: `loadBookmarks` did not validate URLs from the JSON file. Entries written before the guard was introduced (including `about:blank`) were loaded unconditionally as long as `b.url.length > 0`.

Fix: Added URL validation in `loadBookmarks` — same positive-list check (`hasPrefix:@"http://"` / `https://` / `file://`) as in `toggleBookmark:`. Invalid entries from existing JSON are silently dropped on load.

---

## BUG-24 | EP-01-T11 | FIXED 2026-09-18

**"Save Link As…" downloads the file but download bar does not appear.**

Root cause 1: `OnContextMenuCommand` for `kCmdSaveLinkAs` called `browser->GetHost()->StartDownload(url)` directly, bypassing NSSavePanel. `OnBeforeDownload` fires and saves the file, but on some URL types (especially PDFs already loaded in the viewer) CEF may call `OnBeforeDownload` before `DriftOnDownloadStarted` updates the download bar layout — exposing a timing gap.

Root cause 2 (structural): `scheduleDownloadBarHide` called `[ws.downloads removeAllObjects]`, so if a completed download's 5-second timer fired while a new Save-Link-As download was in progress, the new item was wiped from the array and the bar hid before the user saw it.

Fix 1: "Save Link As…" and "Save Image As…" now call `DriftSaveLinkAs(browser_id, url)` → shows NSSavePanel sheet → on confirm stores `url→path` in `gPendingSavePaths` → calls `StartDownload`. `OnBeforeDownload` looks up `DriftConsumePendingSavePath(url)` and uses the chosen path (falls back to `~/Downloads/` for navigation-triggered downloads).

Fix 2: `scheduleDownloadBarHide` now only removes completed/canceled items with `filterUsingPredicate:`, preserving any in-progress download. Bar hides only when array is empty after pruning.

Fix 3: `DriftOnDownloadStarted` now accepts and stores `file_path` on the `DriftDownloadItem`. Download bar items show a "📁 Show in Finder" button (visible when file exists on disk) and an "✕ Clear" button per item. Per-item clear removes only that item.


---

## BUG-25 | EP-01-T11 | FIXED 2026-09-18

**Download bar shows two items for one download — one "Complete" and one stuck at "0B / X MB" forever.**

Root cause: two separate events race for the same download_id. (1) When a PDF link is navigated to, CEF downloads it inline for the viewer — `OnBeforeDownload` fires, item created, download completes. (2) When "Save Link As" (or the PDF viewer's own download button) is clicked, `StartDownload` triggers a second download on the same URL. CEF may serve this from cache synchronously, causing `OnDownloadUpdated(complete=true)` to fire **before** `OnBeforeDownload` / `DriftOnDownloadStarted` runs. The edge-case handler in `DriftOnDownloadUpdated` creates an anonymous placeholder item. Then `DriftOnDownloadStarted` adds a *second* item with the same id. Future `OnDownloadUpdated` calls update the first (anonymous/complete) item; the second item stays at 0B forever.

Fix: `DriftOnDownloadStarted` now checks for an existing item with the same `download_id` first. If found (placeholder from the race), it back-fills the filename, filePath, and totalBytes onto that item instead of creating a duplicate. No two items share the same id.

---

## BUG-26 | EP-01-T11 | FIXED 2026-09-18

**Download bar shows bytes but no speed; no completion summary.**

Root cause: `DriftDownloadItem` had no `startTime`; `refreshDownloadBar` had no speed calculation.

Fix: Added `startTime` (NSDate) set in `DriftOnDownloadStarted`. `refreshDownloadBar` calculates live speed (`receivedBytes / elapsed`) while in-progress and shows "X.X MB/s". On complete: shows "Downloaded in N.Ns · avg X.X MB/s".

---

## BUG-27 | EP-01-T11 | LOGGED 2026-09-18 · workaround shipped

**No download history page — user cannot see all past downloads (cf. Chrome's chrome://downloads).**

Root cause: Full download history tab (EP-01-T11b) is in backlog. No keyboard shortcut exists to navigate to downloads either.

Workaround: ⌘J opens ~/Downloads in Finder (Chrome parity shortcut). Full download history page logged as EP-01-T11b in tasks.md backlog.

