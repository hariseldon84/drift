# Drift — Status Quo

> One-page cold-start anchor. Rewritten, not appended, when state changes materially.
> Last updated: 2026-09-19 (EP-00-T08 complete — SQLite behind repository interfaces)

## What works

- The macOS CEF 152 browser builds and runs with its multi-process architecture.
- EP-01-T01–T08: browser shell, tabs, navigation, window behavior, keyboard shortcuts, Drift theme, and context menus are complete.
- EP-01-T09: basic bookmark toggle/bar/navigation and JSON persistence work — now routed through shared core.
- EP-01-T10: basic history recording/panel/navigation/clear and JSON persistence work.
- EP-01-T11: download save flow/progress/clear/reveal-in-Finder works.
- EP-06-T08: clean macOS clone/bootstrap works; CEF and generated artifacts are ignored and fetched/generated locally.
- GitHub `V018Sep2026` pins the pre-migration snapshot at `9dfda174291a6345032fbb62e50c67bcd2475af9`; `main` descends from that checkpoint.
- Published commit attribution is `hariseldon84`; a local backup ref preserves the pre-rewrite commit objects.
- The product scope and architecture now explicitly target macOS, Windows, Android, and iPhone.
- EP-00-T01: shared C++20 core, native shells, BrowserEngine adapters, bounded Lit surfaces, and the platform capability matrix are locked in `docs/08_cross_platform_architecture.md`.
- EP-00-T02: `core/browser-state/` now supplies strong IDs, command/event/error values, deterministic tab/download transitions, and the versioned `DRIFTBS1` codec.
- EP-00-T02 verification: normal Debug and UBSan core builds pass `drift.browser_state`; the core-only build does not require CEF or Objective-C++.
- EP-00-T03: macOS bookmark/history JSON fixtures and `docs/09_macos_behavior_parity.md` define the extraction gate.
- EP-00-T04: `core/bookmarks/` provides `BookmarkState`, `add_bookmark`, `remove_bookmark`, `has_bookmark`, and `is_bookmarkable_url`. `browser_window.mm` routes all bookmark operations through it. `drift.bookmarks` CTest (6 checks including fixture compat) passes. `DriftBookmark` ObjC class removed.
- EP-00-T05: History and download state extracted behind `drift_history` and `drift_browser_state`; 7 CTests passing.
- EP-00-T06: Tab/navigation/session state routed through `drift_browser_state`; build clean.
- EP-00-T07: `IBrowserEngine` interface + `CefEngineAdapter`; all browser calls in shell routed through engine abstraction.
- EP-00-T08: `IBookmarkRepository` + `IHistoryRepository` interfaces in `core/`; `SqliteBookmarkRepo` + `SqliteHistoryRepo` in `core/storage/` (sqlite3 system lib); JSON-to-SQLite migration on first launch; 4 CTest suites passing including `drift.storage`.
- BUG-29 records a host-toolchain ASan initialization deadlock before `main()`; it remains open while UBSan is the verified sanitizer on this machine.

## What is half-built

- Nothing in the macOS app is half-migrated. Each extracted slice is behavior-preserving.

## What is missing

- The macOS behavior-parity gate (EP-00-T09) — manual run after extraction.
- Windows, Android, and iPhone runnable proofs.
- The page-understanding, object, Compare Mode, action, and widget product layers.

## In flight

EP-00 is the active migration epic. SP-07 complete (T07, T08). Next: EP-00-T09 (macOS parity gate).

## Open decisions waiting on Anand

| Decision | Needed by | Task |
|---|---|---|
| VPN vendor: Mullvad vs Guardian | Scale planning | EP-09 |
| Analytics: self-hosted Matomo or explicit deferral | Distribution planning | EP-06-T07 |
| Beta program: invite-only or open waitlist | Distribution planning | EP-06 |
| Crash symbol infrastructure | Distribution planning | EP-06-T06 |

Android WebView versus GeckoView is not a preference question yet; EP-00-T12 resolves it from the measured platform proof.

## The single most useful next action

EP-00-T09: run `docs/09_macos_behavior_parity.md` manually against `open build/src/browser/Drift.app` to confirm the extraction left behavior intact, then declare SP-07 done.
