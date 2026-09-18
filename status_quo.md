# Drift — Status Quo

> One-page cold-start anchor. Rewritten, not appended, when state changes materially.
> Last updated: 2026-09-18 (cross-platform Phase 0 and Phase 1 complete)

## What works

- The macOS CEF 152 browser builds and runs with its multi-process architecture.
- EP-01-T01–T08: browser shell, tabs, navigation, window behavior, keyboard shortcuts, Drift theme, and context menus are complete.
- EP-01-T09: basic bookmark toggle/bar/navigation and JSON persistence work.
- EP-01-T10: basic history recording/panel/navigation/clear and JSON persistence work.
- EP-01-T11: download save flow/progress/clear/reveal-in-Finder works.
- EP-06-T08: clean macOS clone/bootstrap works; CEF and generated artifacts are ignored and fetched/generated locally.
- GitHub `V018Sep2026` pins the pre-migration snapshot at `9dfda174291a6345032fbb62e50c67bcd2475af9`; `main` descends from that checkpoint.
- Published commit attribution is `hariseldon84`; a local backup ref preserves the pre-rewrite commit objects.
- The product scope and architecture now explicitly target macOS, Windows, Android, and iPhone.
- EP-00-T01: shared C++20 core, native shells, BrowserEngine adapters, bounded Lit surfaces, and the platform capability matrix are locked in `docs/08_cross_platform_architecture.md`.

## What is half-built

- Nothing in source is half-migrated. The current macOS implementation remains intact in `src/browser/`.
- The architecture exists in documentation; its shared contracts and test harness do not exist yet.

## What is missing

- Automated C++ core tests, serialization fixtures, and macOS characterization checks.
- Platform-neutral command/event/error types and `BrowserEngine` contract.
- Extracted shared state for bookmarks, history, downloads, tabs, and navigation.
- macOS CEF adapter behind the new engine boundary.
- SQLite repository implementation and migration tests.
- Windows, Android, and iPhone runnable proofs.
- The page-understanding, object, Compare Mode, action, and widget product layers.

## In flight

EP-00 remains the active migration epic. No source extraction is half-finished; Phase 1 is complete and SP-05/Phase 2 is the next bounded unit of work.

## Open decisions waiting on Anand

| Decision | Needed by | Task |
|---|---|---|
| VPN vendor: Mullvad vs Guardian | Scale planning | EP-09 |
| Analytics: self-hosted Matomo or explicit deferral | Distribution planning | EP-06-T07 |
| Beta program: invite-only or open waitlist | Distribution planning | EP-06 |
| Crash symbol infrastructure | Distribution planning | EP-06-T06 |

Android WebView versus GeckoView is not a preference question yet; EP-00-T12 resolves it from the measured platform proof.

## The single most useful next action

Start Phase 2 with EP-00-T02 and EP-00-T03: add the smallest runnable C++ test target, define platform-neutral IDs/commands/events/errors, and capture current bookmark/state behavior before extraction.
