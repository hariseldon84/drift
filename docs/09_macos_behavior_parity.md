# macOS Behavior-Parity Gate

> Source-derived baseline and manual gate definition for EP-00-T03. Created
> 2026-09-18. Manual execution evidence is intentionally recorded at EP-00-T09.
> Reference implementation: `V018Sep2026` at
> `9dfda174291a6345032fbb62e50c67bcd2475af9`.

## Purpose

This gate prevents the cross-platform extraction from silently changing the
working macOS browser. Run it against the checkpoint and again after each
vertical slice. A migrated behavior passes only when its observable result and
persistence compatibility match the reference, unless an intentional change is
recorded in the PRD and task board.

## Automated foundation checks

From the repository root:

```bash
cmake -S . -B build-core -G Ninja \
  -DDRIFT_BUILD_BROWSER_APP=OFF \
  -DBUILD_TESTING=ON \
  -DCMAKE_BUILD_TYPE=Debug
cmake --build build-core --parallel
ctest --test-dir build-core --output-on-failure
```

Expected: `drift.browser_state` passes. It covers stable identifiers, tab
creation/selection/movement/closure, navigation metadata, progress validation,
download transitions, deterministic snapshot encoding, version rejection, and
truncated-input rejection.

The core-only configuration must not look for CEF or require Objective-C++.

## Persistence fixtures

- `tests/fixtures/macos/bookmarks-v1.json` captures the current array/object
  shape: `url`, `title`, and UTC `timestamp`. Only `http://`, `https://`, and
  `file://` URLs load; the included `about:blank` entry must be discarded.
- `tests/fixtures/macos/history-v1.json` captures the current newest-first array
  with the same three fields. Non-empty URLs load, timestamps remain strings,
  and the runtime list is capped at 5,000 entries.

EP-00-T04/T05 must turn these fixtures into executable compatibility tests
before replacing the Objective-C++ JSON code.

## Manual baseline setup

1. Preserve any personal files in
   `~/Library/Application Support/Drift/` before testing persistence.
2. Build and launch using the commands in `commands.md`.
3. Confirm the initial window is approximately 1280×800 and the first tab is
   `about:blank` with the address field ready for input.
4. Keep the reference branch and migrated build results separate in the test
   record; do not switch branches while either app instance is running.

## Browser and window behavior

- [ ] One main window opens; renderer, GPU, and utility/helper processes appear.
- [ ] Window resize, minimize, zoom, native fullscreen, and double-click zoom work.
- [ ] Closing the last tab quits Drift and its helper processes.
- [ ] Quitting with Cmd+Q exits cleanly without a lingering browser process.

## Tabs

- [ ] Cmd+T and the tab-strip action create an active blank tab.
- [ ] Clicking a tab selects it without changing its URL or title.
- [ ] Dragging a tab reorders it and preserves the active tab identity.
- [ ] Pin/unpin changes the tab treatment and survives in-session reorder.
- [ ] Duplicate Tab opens the same URL in a new tab.
- [ ] Close Other Tabs preserves the chosen tab and closes the rest.
- [ ] Closing an active tab selects the nearest surviving tab at the same index,
      or the previous final tab when the closed tab was last.
- [ ] Cmd+Shift+T restores the most recently closed URL.
- [ ] Cmd+1 through Cmd+9 select existing tab positions.
- [ ] Control+Tab and Control+Shift+Tab wrap forward/backward.

## Address and navigation

- [ ] Cmd+L focuses the address field and selects all text.
- [ ] Cmd+A selects all only while the address editor is active.
- [ ] A full `http://`, `https://`, or `file://` URL navigates directly.
- [ ] A dotted token without spaces receives an `https://` prefix.
- [ ] Other text becomes a Google search query.
- [ ] Back/forward enabled state follows CEF navigation state.
- [ ] Cmd+R reloads; the button becomes Stop while loading.
- [ ] Amber progress follows load progress and hides after completion.
- [ ] HTTPS pages show the lock indicator; blank/non-HTTPS pages do not.
- [ ] The address field does not overwrite text while the user is editing.

## Context menus and visual baseline

- [ ] Page, link, and image context menus expose their current CEF-backed actions.
- [ ] Tab menu exposes New, Duplicate, Close, Close Others, and Pin/Unpin.
- [ ] Inter is used for browser chrome and the warm Drift palette remains intact.
- [ ] URL host/path emphasis, hover states, disabled states, and active-tab
      treatment match the checkpoint visually.

## Bookmarks

- [ ] Cmd+D adds the active real URL and turns the star amber/filled.
- [ ] Cmd+D again removes the same URL and restores the outline star.
- [ ] `about:blank`, `chrome://`, and `data:` cannot be bookmarked.
- [ ] Cmd+Shift+B toggles the bookmarks bar without overlapping the viewport.
- [ ] Clicking a bookmark navigates the active tab.
- [ ] Relaunch loads valid bookmark entries from `bookmarks.json`.
- [ ] The `bookmarks-v1.json` legacy invalid entry is ignored.

## History

- [ ] Completed titled navigations are inserted newest-first.
- [ ] `about:blank` is not recorded.
- [ ] Cmd+Y opens/reuses the History panel.
- [ ] Double-clicking a row navigates the active tab.
- [ ] Clear History empties the panel and persists the empty array.
- [ ] Relaunch preserves order and fields from `history.json`.
- [ ] More than 5,000 entries drops the oldest entry.

## Downloads

- [ ] A normal download uses the expected save path flow.
- [ ] Save Link As uses the selected path.
- [ ] In-progress filename, byte counts, and amber progress are visible.
- [ ] Completion shows the completed state and reveal-in-Finder action.
- [ ] Clear removes only the selected completed/cancelled item.
- [ ] Automatic cleanup never removes an in-progress item.

## Gate result

Record the following in the implementing task/commit:

| Evidence | Required value |
|---|---|
| Reference commit | `9dfda174291a6345032fbb62e50c67bcd2475af9` |
| Migrated commit | Exact tested SHA |
| macOS version and architecture | Exact test machine |
| CEF version | Exact pin from CMake |
| Core tests | Command and pass output |
| Manual checklist | Pass/fail plus deviations |
| Persistence fixtures | Compatibility test output |
| Known differences | PRD/task/bug reference or `none` |

## Not baseline behavior

Do not block parity on features the checkpoint never implemented: bookmark
folders/import, history search/internal page, download pause/resume/history,
extensions, password management, settings, session restore across relaunch,
page understanding, Compare Mode, widgets, or sync.
