# Drift — Agent Handover

> Written 2026-09-17. For the next agent picking up EP-01-T07.
> Read `groundrules.md` first — it governs every session without exception.

---

## What Drift is

Drift is a Chromium-based macOS desktop browser being built from scratch using CEF (Chromium Embedded Framework). It will run a local page-understanding pipeline that recognises typed objects (V0: Products) on web pages and surfaces them through Compare Mode and a widget surface. All inference runs on-device. No browsing data leaves the machine.

The project is in active development. V0 is a functional browser with Drift's intelligence layer on top. The shell (browser chrome) is being built first; the intelligence pipeline starts in EP-02.

---

## Repository map

```
drift/
├── scripts/
│   └── fetch-cef.sh       ← downloads and verifies the pinned Mac CEF distribution
├── .deps/                ← local CEF dependency cache (Git-ignored)
├── src/browser/          ← ALL active code lives here (C++ / Objective-C++)
│   ├── browser_window.mm ← The entire browser UI: window, tab strip, nav bar, all delegates
│   ├── browser_window.h  ← C++ bridge declarations (called from drift_client.cpp)
│   ├── drift_client.cpp  ← CefClient implementation — routes CEF callbacks to the ObjC UI
│   ├── drift_client.h
│   ├── drift_app.cpp     ← CefApp — startup URL, command-line flags
│   └── drift_app.h
├── docs/                 ← Product docs, design spec, architecture, competitive research
│   ├── design_specification.md  ← Source of truth for ALL colours, fonts, spacing
│   └── 01–07_*.md        ← Product thesis through PRD gate
├── PRD.md                ← Full product requirements (22 sections). Read for the WHY.
├── CLAUDE.md             ← Stack, architecture, locked decisions. Read for the HOW.
├── tasks.md              ← Sprint plan SP-01–SP-14 with every task coded EP-XX-TXX
├── status_quo.md         ← One-page anchor: what works, what's next. Read first each session.
├── groundrules.md        ← Session rules. Non-negotiable.
├── bugs3.md              ← Active bug log. bugs.md and bugs2.md are frozen.
├── commands.md           ← Every build/run command in full runnable form
└── design_inspirations/  ← 34 UI mockup screens (the visual target)
```

`browser_window.mm` is the only file that matters right now. It is ~900 lines of Objective-C++ and contains everything: tab model, tab strip view, window controller, address bar, nav buttons, progress bar, keyboard shortcuts, and all C++ bridge implementations.

---

## Tech stack (what matters for EP-01)

| Thing | Detail |
|---|---|
| CEF version | 152, dylib mode — loaded at runtime via `CefScopedLibraryLoader` |
| Build | CMake 3.25+. Run `./scripts/fetch-cef.sh` once, then configure/build from repo root. |
| Memory model | **MRC (Manual Reference Counting)** — CEF compiles without ARC. No `__weak`, use `__unsafe_unretained`. Every `dealloc` must call `[super dealloc]`. `@property` on C++ types must be `(nonatomic)`. |
| Sandbox | Disabled via `--no-sandbox` in `DriftApp::OnBeforeCommandLineProcessing` for unsigned dev builds |
| Run | `open build/src/browser/Drift.app` |

---

## What is built and working (as of 2026-09-17)

### EP-01-T01 — CEF bootstrap
Window opens, multi-process architecture running (Drift + Drift Helper × 4).

### EP-01-T02 — Tab bar
Open, close, drag-to-reorder, pin, favicon letter badge. Tab strip sits in the unified chrome area using `NSWindowStyleMaskFullSizeContentView` with transparent title bar. Usable height below title bar computed via `_window.contentLayoutRect`.

### EP-01-T03 — Address bar
URL navigation (Enter navigates), domain + search fallback (Google), HTTPS lock icon, field editor handling via `currentEditor`. Startup URL is `about:blank`.

### EP-01-T04 — Back / Forward / Reload + Progress
Three SF Symbol buttons left of the URL pill. Amber progress bar at the bottom of the nav bar driven by `OnLoadingProgressChange`. Reload button toggles to Stop (✕) during loads. `canGoBack` / `canGoForward` tracked from `OnLoadingStateChange`.

### EP-01-T05 — Window management
Resize, fullscreen (green button / ⌃⌘F), zoom, miniaturize all working. Double-click in tab strip empty space or title bar area zooms, respecting the system preference.

### EP-01-T06 — Keyboard shortcuts
`⌘T` new tab · `⌘W` close tab · `⌘L` focus address bar · `⌘R` reload · `⌘⇧T` reopen last closed · `⌘1–9` jump to tab. Single `NSEvent` local event monitor in `initWithURL:`. Closed-tab URLs tracked in `_closedTabURLs` (NSMutableArray on the controller).

---

## What is next

**EP-01-T07 — Drift visual theme on chrome** (next task)

Apply the full Drift design to the browser chrome:
- Inter font for all chrome text (tab titles, URL field, nav bar)
- Refined tab strip: proper active/inactive tab shapes, correct colours from design spec
- Nav button sizing, hover states, disabled-state opacity
- URL pill: font, size, colour split between host (darker) and path (lighter)
- General colour pass — ensure every element uses the exact spec values

Design tokens: `docs/design_specification.md`. The hex values are already partially in code comments. Never hardcode without a comment referencing the token name.

After T07: **EP-01-T08 — Context menus** (right-click on page, tab, link, image).

---

## Key CEF / ObjC gotchas discovered this session

**1. `DoClose` must return `true` for embedded child browsers**
All tab browsers share a single parent `NSWindow`. Returning `false` from `DoClose` causes CEF to call `performClose:` on the shared parent window → `windowShouldClose:` → `[NSApp terminate:nil]` → browser closes on every tab close. Return `true`; CEF still calls `OnBeforeClose` for proper cleanup.

**2. NSTextField field editor vs. `stringValue`**
`_urlField.stringValue` returns the stored display value, not live typing content. During editing, read from `[_urlField currentEditor].string`. Guard `refreshNavBar` with `if ([_urlField currentEditor]) return`.

**3. `selectAll:` must be deferred**
`[editor selectAll:nil]` inside `controlTextDidBeginEditing:` silently fails — field editor is not yet first responder. Use `dispatch_async(dispatch_get_main_queue(), ^{ [editor selectAll:nil]; })`.

**4. `attributedStringValue` corrupts the field editor**
Use plain `stringValue` everywhere for display. Setting `attributedStringValue` diverges the field editor state.

**5. `charactersIgnoringModifiers` returns uppercase when Shift is held**
For shortcuts, `NSEvent.charactersIgnoringModifiers` returns `"T"` not `"t"` when Shift is pressed, because Shift changes the base character. Compare with `ch.lowercaseString`, not `ch` directly.

**6. CEF swallows Command+key events**
CEF can consume Command+key events before they reach AppKit's key binding system. Use `NSEvent addLocalMonitorForEventsMatchingMask:NSEventMaskKeyDown` to intercept before CEF. The monitor fires before `sendEvent:` dispatches to any window.

**7. MRC blocks**
`__unsafe_unretained typeof(self) ws = self` inside blocks. Never `__weak` — won't compile without ARC. Remove monitors in `dealloc`. Always call `[super dealloc]`.

**8. `CefRefPtr` property must be `(nonatomic)`**
`@property CefRefPtr<CefBrowser>` must be declared `(nonatomic)` or it calls `objc_copyCppObjectAtomic` and crashes.

---

## Design system — critical rules

All values from `docs/design_specification.md`. In ObjC, use `[NSColor colorWithSRGBRed:green:blue:alpha:]` and comment the token name.

| Token | Hex | sRGB approx | Use |
|---|---|---|---|
| `--drift-base` | `#FAF7F2` | 0.980, 0.969, 0.949 | Browser frame, nav bar |
| `--drift-surface` | `#F2EDE4` | 0.949, 0.929, 0.898 | URL pill, tab backgrounds |
| `--drift-amber` | `#D4820A` | 0.831, 0.510, 0.039 | Intelligence only — progress bar |
| `--drift-green` | `#2D5016` | 0.176, 0.314, 0.086 | Lock icon, confirmations |
| `--drift-text` | `#1C1410` | 0.110, 0.078, 0.063 | Primary text |
| `--drift-muted` | `#7A6E65` | 0.478, 0.431, 0.396 | URL display, secondary text |
| `--drift-subtle` | `#B5A89C` | 0.710, 0.659, 0.612 | URL path, tertiary |

Font: **Inter** for all chrome UI. **Lora** for intelligence moments only. Both bundled in `resources/fonts/`. Never load from CDN.

Never use: corporate blue, neon, purple, cold grey, pure black `#000000`, pure white `#ffffff`.

---

## Bugs fixed this session (all in bugs3.md)

| ID | Description | Root cause |
|---|---|---|
| BUG-01 | Enter navigated to wrong URL | `stringValue` vs `currentEditor.string` |
| BUG-02 | Text overlap, ⌘A broken | `attributedStringValue` + sync `selectAll:` |
| BUG-03 | Red X hung, never closed | Browser-less tabs in `closingTabs` |
| BUG-04 | ⌘⇧T did nothing after closing blank tab | `about:blank` filtered from `closedTabURLs` |
| BUG-05 | Any tab close closed the entire browser | `DoClose` returning `false` → CEF called `performClose:` on shared parent window |
| BUG-06 | ⌘⇧T monitor check always failed | `charactersIgnoringModifiers` returns `"T"` not `"t"` when Shift held |

---

## Build and run

```bash
# From repo root — first clone / dependency bootstrap
./scripts/fetch-cef.sh

# Configure once. CMake discovers the pinned CEF package in .deps/.
cmake -S . -B build -G Ninja -DPROJECT_ARCH="$(uname -m)" -DCMAKE_BUILD_TYPE=Release

# Incremental build (~5–30s after the initial wrapper build)
cmake --build build --parallel

# Launch
open build/src/browser/Drift.app

# Full clean reconfigure if needed
cmake -E remove_directory build
cmake -S . -B build -G Ninja -DPROJECT_ARCH="$(uname -m)" -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

The app auto-signs on every build. No manual codesign step needed.
CEF is required at build and runtime, but the downloaded distribution and all
generated build output are ignored by Git. Do not add either to Git LFS.

---

## Groundrules (read groundrules.md in full)

- Read `status_quo.md` first every session. Rewrite it (not append) when state changes.
- No work starts unless it is in `tasks.md`. Add it, then do it.
- Bugs go in `bugs3.md`. Never delete entries — update status in place.
- Every decision lands in `PRD.md` AND `tasks.md` the same turn.
- A fix is not fixed until it has been run and verified with output shown.
- Ship reports are not optional: what changed, how to test, what to expect.

---

## SP-03 Testing Guide (EP-01-T09, T10, T11)

Build and launch first:
```bash
cmake --build build
open build/src/browser/Drift.app
```

### EP-01-T09 — Bookmarks

**Star button (⌘D)**
1. Navigate to any real page (e.g. `apple.com`).
2. Press ⌘D. The star icon in the URL pill should turn solid amber (bookmarked).
3. Press ⌘D again. Star returns to outline (unbookmarked).

**Bookmarks toolbar (⌘⇧B)**
1. Add a few bookmarks (⌘D on 2–3 pages).
2. Press ⌘⇧B. A 26px bar appears below the address bar showing the bookmark titles.
3. Click a bookmark button → tab navigates to that URL.
4. Press ⌘⇧B again → bar hides; content area fills back.

**Persistence**
1. Add a bookmark, quit Drift, relaunch.
2. Press ⌘⇧B → bookmark is still there.
3. Confirm file at `~/Library/Application Support/Drift/bookmarks.json` exists.

---

### EP-01-T10 — History

**Recording**
1. Navigate to 3–4 different pages. Each full page load records an entry when `OnTitleChange` fires.

**History panel (⌘Y)**
1. Press ⌘Y. A 480×560 floating panel opens titled "History".
2. Panel shows entries in reverse-chronological order: `YYYY-MM-DD   Page Title`.
3. Double-click any row → active tab navigates to that URL.

**Clear History**
1. Click "Clear History" in the panel → list empties.
2. Quit and relaunch → history remains empty (file cleared).

**Persistence**
1. `~/Library/Application Support/Drift/history.json` should exist after browsing.
2. Cap is 5000 entries (old entries dropped from the bottom).

---

### EP-01-T11 — Downloads

**Triggering a download**
1. Navigate to any direct file link (e.g. a PDF or ZIP).
2. The 48px download bar should slide up from the bottom of the content area.
3. Bar shows filename, KB received / KB total, and an amber progress fill.
4. When complete: fill turns green, status shows "Complete".
5. After 5 seconds: download bar hides automatically.

**File location**
- Files are saved to `~/Downloads/` with the suggested filename.

**What is not in V0**
- "Open" / "Show in Finder" buttons (deferred to SP-04+).
- Multiple concurrent download item rows beyond available width.
- Import from Chrome/Safari bookmarks (deferred to SP-04+).
