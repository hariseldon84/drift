# Drift — Agent Handover

> Recreated 2026-09-18 after Phase 2 of the four-platform migration.
> Start by reading `groundrules.md`, `status_quo.md`, `tasks.md`, and `commands.md`.

## Current mission

Drift is a local-first browser product for macOS, Windows, Android, and iPhone.
The working browser today is the macOS CEF app in `src/browser/`. Preserve it as
the behavior reference while extracting shared product behavior into a
platform-neutral C++20 core.

The locked architecture is:

```text
native shell / Lit surface
        -> versioned command
shared C++20 state + product core
        -> BrowserEngine request
platform adapter
        -> normalized event/result
shared observable state -> UI
```

Native shells own OS lifecycle, permissions, menus, packaging, and engine
hosting. The shared C++20 core owns Drift state and product behavior. Lit is for
Drift-owned product surfaces and internal pages, not for browser engine hosting
or native lifecycle replacement.

## Git and publication state

- `main` is the active migration branch.
- `origin/main` currently points at `c533df9` (`core: add platform-neutral browser state contract`).
- `V018Sep2026` is the preserved pre-migration checkpoint at
  `9dfda174291a6345032fbb62e50c67bcd2475af9`.
- Published commit identity is
  `hariseldon84 <514368+hariseldon84@users.noreply.github.com>`.
- Local ref `backup/pre-attribution-rewrite-20260918` preserves the superseded
  pre-rewrite objects. It is intentionally not pushed.
- CEF, `.deps/`, `build/`, generated app bundles, and wrapper archives do not
  belong in Git or Git LFS. Fresh clones fetch CEF with `scripts/fetch-cef.sh`.

At the time this handover was recreated, the working tree also had unrelated
local changes not owned by the Phase 2 migration:

- `src/browser/browser_window.mm`
- `assets/`

Do not revert or accidentally include those unless your task explicitly covers
them.

## Latest completed work

Phase 0, Phase 1, and Phase 2 of `_wip_cross_platform_migration.md` are done.

Phase 0 preserved the working browser:

- rewrote the published `anandets` attribution to the repository-owner identity;
- pushed `V018Sep2026` as the verified browser checkpoint;
- kept the generated CEF/build artifacts out of Git.

Phase 1 locked the product and architecture:

- made macOS, Windows, Android, and iPhone first-class product targets;
- documented the shared C++20 core, native shells, engine adapters, and bounded
  Lit UI rule in `docs/08_cross_platform_architecture.md`;
- replaced the sprint order with the Cross-Platform Foundation epic in
  `tasks.md`, `PRD.md`, `CLAUDE.md`, and `status_quo.md`.

Phase 2 added the first platform-neutral contract:

- `core/browser-state/browser_contract.h` defines strong IDs, commands, events,
  download/navigation/extraction values, and error vocabulary;
- `core/browser-state/browser_state.h/.cpp` implements deterministic tab and
  download state transitions;
- `core/browser-state/browser_state_codec.h/.cpp` implements the versioned
  deterministic `DRIFTBS1` binary codec;
- `tests/core/browser_state_test.cpp` covers reducer behavior, validation,
  serialization round trips, golden bytes, malformed input rejection, and
  RequestId validation;
- `tests/fixtures/macos/` captures source-derived bookmark/history JSON inputs;
- `docs/09_macos_behavior_parity.md` defines the parity gate that must stay
  green as state moves into the core.

The new shared core is deliberately not wired into the live macOS app yet.
That wiring begins with the next task.

## What works now

The macOS CEF 152 app builds and runs with:

- multi-process browser/helper startup;
- tab open, close, reorder, pin, switch, and restore;
- URL/search navigation, back, forward, reload, stop, loading state, and
  progress;
- native resize, fullscreen, zoom, minimize, and browser shortcuts;
- Drift theme, bundled fonts, and context menus;
- basic bookmarks with bar/navigation/JSON persistence;
- basic history with panel/navigation/clear/JSON persistence;
- downloads with save flow, progress, clear, and reveal in Finder.

The shared core can also build and test without CEF or Objective-C++ by setting
`DRIFT_BUILD_BROWSER_APP=OFF`.

## What does not exist yet

- Shared bookmark state wired into the macOS app.
- Shared history, downloads, tabs, navigation, and session state wired into the
  macOS app.
- `BrowserEngine` interface and macOS CEF adapter.
- Repository interfaces and SQLite persistence boundary.
- Windows, Android, or iPhone runnable proofs.
- Object graph, Compare Mode, action layer, widgets, sync, or local
  intelligence product layers.

Do not create empty destination folders just to make the tree look migrated.
Each migration phase needs a runnable, tested vertical slice.

## Next task

Start EP-00-T04:

```text
Extract bookmarks behind a tested shared-core state model.
```

Bound the slice tightly:

1. Make the macOS `bookmarks-v1.json` fixture executable as a compatibility
   test.
2. Add platform-neutral Bookmark values and deterministic bookmark state.
3. Preserve the current JSON format and file path.
4. Keep Objective-C++ JSON I/O temporarily, mapping it to and from core values.
5. Route add, remove, query, and render decisions through shared state.
6. Build, run core tests, and execute the bookmark section of
   `docs/09_macos_behavior_parity.md`.

Do not introduce SQLite or repository interfaces in EP-00-T04; those belong to
EP-00-T08.

## Build and verification commands

Run all commands from the repository root. The canonical command list is
`commands.md`.

Fresh macOS dependency/bootstrap:

```bash
./scripts/fetch-cef.sh
cmake -S . -B build -DPROJECT_ARCH="$(uname -m)" -DCMAKE_BUILD_TYPE=Release -G Ninja
cmake --build build --parallel
ctest --test-dir build --output-on-failure
open build/src/browser/Drift.app
```

Shared core only:

```bash
cmake -S . -B build-core -G Ninja -DDRIFT_BUILD_BROWSER_APP=OFF -DBUILD_TESTING=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build build-core --parallel
ctest --test-dir build-core --output-on-failure
```

Shared core with UBSan:

```bash
cmake -S . -B build-core-ubsan -G Ninja -DDRIFT_BUILD_BROWSER_APP=OFF -DBUILD_TESTING=ON -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS="-fsanitize=undefined -fno-omit-frame-pointer" -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=undefined"
cmake --build build-core-ubsan --parallel
ctest --test-dir build-core-ubsan --output-on-failure
```

Normal and UBSan core tests passed in Phase 2. BUG-29 records the current
macOS 26.5.1 / AppleClang 17 AddressSanitizer startup deadlock before `main()`;
do not report it as a Drift test failure or as a passing ASan gate.

## Files to know

```text
core/browser-state/       current platform-neutral contract, reducer, and codec
src/browser/              live macOS CEF implementation
tests/core/               shared-core CTest coverage
tests/fixtures/macos/     bookmark/history compatibility inputs
docs/08_cross_platform_architecture.md
docs/09_macos_behavior_parity.md
_wip_cross_platform_migration.md
tasks.md
bugs3.md
commands.md
```

`src/browser/browser_window.mm` is still the large live implementation. Treat it
carefully: it owns current AppKit UI state, JSON persistence, and direct CEF
calls until migration slices remove those responsibilities.

## Objective-C++ and CEF facts to preserve

- The browser target uses Manual Reference Counting because of CEF integration.
- Do not introduce `__weak`; existing block captures use `__unsafe_unretained`
  where required.
- `dealloc` must call `[super dealloc]`.
- `CefRefPtr` Objective-C properties must be `(nonatomic)`.
- Embedded child browser `DoClose` returns `true`.
- Live URL field input comes from `currentEditor.string`, not stale
  `stringValue`.
- Normalize `charactersIgnoringModifiers` case for shifted shortcuts.
- CEF may consume Command-key events; the shell uses a local `NSEvent` monitor.

## Persistence paths to keep compatible

```text
~/Library/Application Support/Drift/bookmarks.json
~/Library/Application Support/Drift/history.json
```

EP-00 extraction must keep existing JSON readable until explicit migrations and
compatibility tests exist.

## Manual baseline smoke

Before claiming a migration slice preserves behavior:

1. Build and launch Drift.
2. Open multiple tabs; navigate, reorder, pin, close, and restore one.
3. Exercise back/forward, reload/stop, URL/search navigation, and shortcuts.
4. Add a bookmark, toggle the bar, relaunch, and verify persistence.
5. Visit pages, open History with Cmd+Y, navigate an item, clear history, and
   verify persistence.
6. Download a file, verify progress/completion, and reveal it in Finder.
7. Quit and confirm browser/helper processes exit.

Record newly observed bugs in `bugs3.md` before fixing them.
