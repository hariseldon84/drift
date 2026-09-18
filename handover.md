# Drift — Agent Handover

> Updated 2026-09-18 after the four-platform architecture decision.
> Read `groundrules.md`, then `status_quo.md`, before starting work.

## Product and current direction

Drift is a local-first browser for macOS, Windows, Android, and iPhone. Its differentiator is typed web objects, cross-tab/device composition, Compare Mode, and human-gated actions.

The working product today is a macOS CEF browser. Do not rewrite or discard it. It is the behavior reference for a controlled migration to native platform shells, a shared C++20 state/product core, and replaceable engine adapters. The locked design is in `docs/08_cross_platform_architecture.md`; canonical task order is in `tasks.md`.

## Git checkpoints

- `main` and remote `V018Sep2026` were created from the same verified source tree at commit `9dfda174291a6345032fbb62e50c67bcd2475af9` before migration code began.
- The published commit identity is `hariseldon84 <514368+hariseldon84@users.noreply.github.com>`.
- Local branch `backup/pre-attribution-rewrite-20260918` preserves the pre-rewrite commit objects.
- Generated `build/`, downloaded CEF, and app bundles are not in Git or Git LFS.

## What works

The macOS implementation supports:

- CEF 152 multi-process startup;
- tab open/close/reorder/pin/restore and tab switching;
- URL/search navigation, back/forward, reload/stop, loading state, and progress;
- native resize/fullscreen/zoom/minimize and browser keyboard shortcuts;
- Drift visual theme with bundled fonts;
- page/link/image and tab-strip context menus;
- basic bookmarks, bookmarks bar, and JSON persistence;
- basic history panel, clear/navigation, and JSON persistence;
- downloads with save flow, progress, clear, and reveal in Finder.

`bugs3.md` is the current bug log. BUG-27 retains a shipped workaround plus a backlog follow-up; other logged baseline bugs are fixed as recorded there.

## Current implementation map

```text
src/browser/
  main.mm                 process setup and macOS entry point
  browser_window.mm       AppKit UI, state, JSON persistence, and direct CEF calls
  browser_window.h        C++/Objective-C++ bridge declarations
  drift_client.*          CEF client callbacks routed to the UI
  drift_app.*             CEF application setup and command-line configuration
scripts/fetch-cef.sh      pinned/checksummed CEF bootstrap
```

`browser_window.mm` is about 1,800 lines and currently mixes concerns that EP-00 will separate. There is no shared core, BrowserEngine abstraction, Lit implementation, automated test suite, Windows shell, Android shell, or iPhone shell yet.

## Locked architecture

```text
native shell / Lit surface
        -> versioned command
shared C++20 state + product core
        -> BrowserEngine request
platform adapter
        -> normalized event/result
shared observable state -> UI
```

- macOS: Objective-C++/AppKit + CEF
- Windows: C++/Win32 or WinUI + CEF
- Android: Kotlin + WebView proof; GeckoView fallback
- iPhone: Swift + WKWebView baseline
- Lit: Drift-owned product surfaces/internal pages only
- extensions: desktop-only initial scope

No CEF, AppKit, WebKit, JNI, Swift, Kotlin, or WinUI type enters the shared core. No C++ standard-library type crosses a DLL/mobile language boundary.

## Next work: EP-00-T02 and EP-00-T03

Phase 1 documentation is complete. Phase 2 must establish contracts and characterization before moving existing state.

Recommended first vertical sequence:

1. add the smallest C++ test target and run it in the current CMake build;
2. define opaque IDs plus command/event/error value types with no engine types;
3. characterize current bookmark JSON and state transitions with fixtures/tests;
4. write the macOS parity checklist from observable behavior;
5. only then extract bookmarks for EP-00-T04.

Do not start by moving every file or creating empty target folders.

## Build and run

```bash
./scripts/fetch-cef.sh
cmake -S . -B build -G Ninja -DPROJECT_ARCH="$(uname -m)" -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
open build/src/browser/Drift.app
```

The app is ad-hoc signed for development. CEF is loaded in dylib mode. Full commands and fresh-clone steps are in `commands.md`.

## Important CEF / Objective-C++ facts

- The current target is Manual Reference Counting because of the CEF build. Do not use `__weak`; block captures use `__unsafe_unretained` where required, and `dealloc` calls `[super dealloc]`.
- `CefRefPtr` Objective-C properties must be `(nonatomic)`.
- Embedded child browser `DoClose` returns `true`; returning `false` can close the shared native window.
- Live `NSTextField` input comes from `currentEditor.string`, not the stored `stringValue`.
- Defer `selectAll:` until the field editor is first responder.
- Avoid `attributedStringValue` for the address field because it can diverge from the field editor.
- Normalize `charactersIgnoringModifiers` case for shifted shortcuts.
- CEF may consume Command-key events; the current shell uses one local `NSEvent` monitor.

## Persistence paths to preserve

- `~/Library/Application Support/Drift/bookmarks.json`
- `~/Library/Application Support/Drift/history.json`

EP-00 extraction must keep these formats readable until explicit migrations and compatibility tests exist.

## Manual baseline smoke test

1. Build and launch Drift.
2. Open multiple tabs; navigate, reorder, pin, close, and restore one.
3. Exercise back/forward, reload/stop, URL/search navigation, and core keyboard shortcuts.
4. Add a bookmark, toggle the bar, relaunch, and verify persistence.
5. Visit pages, open History with Cmd+Y, navigate an item, clear history, and verify persistence.
6. Download a file, verify progress/completion, and reveal it in Finder.
7. Quit and confirm the browser/helper processes exit.

Record any newly observed bug in `bugs3.md` before fixing it.
