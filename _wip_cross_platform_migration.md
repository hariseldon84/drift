# Drift Cross-Platform Migration — Working Checklist

> Temporary execution tracker for the approved four-platform architecture migration.
> Delete this file only after every phase has landed and the canonical documents are current.

## Phase 0 — Preserve the working browser

- [x] Rewrite the two published commits from `anandets` to the repository-owner identity.
- [x] Preserve the pre-rewrite remote tip under a local backup ref.
- [x] Create and push the checkpoint branch `V018Sep2026` at the verified snapshot.
- [x] Verify `V018Sep2026` pins the clean snapshot and `main` starts from that commit.

## Phase 1 — Lock the revised product and architecture

- [x] Make macOS, Windows, Android, and iPhone explicit product targets.
- [x] Add a platform capability matrix, including desktop-only extension scope.
- [x] Lock C++20 shared-core boundaries and native platform shells.
- [x] Lock the BrowserEngine command/event contract and engine adapters.
- [x] Define Lit's scope as shared Drift surfaces, not the browser engine or native shell.
- [x] Replace the current sprint order with a Cross-Platform Foundation epic.
- [x] Update PRD.md, tasks.md, CLAUDE.md, status_quo.md, and handover.md.

## Phase 2 — Contracts and characterization tests

- [ ] Add platform-neutral browser identifiers, state models, commands, and events.
- [ ] Add deterministic unit tests for state transitions and serialization.
- [ ] Add a macOS behavior-parity checklist before extraction begins.

## Phase 3 — Extract shared browser state

- [ ] Extract bookmarks as the first tested vertical slice.
- [ ] Extract history and download state.
- [ ] Extract tabs, navigation, and session state.

## Phase 4 — Introduce the engine boundary

- [ ] Define the BrowserEngine interface with no AppKit or CEF types.
- [ ] Convert existing CEF callbacks into the macOS/desktop CEF adapter.
- [ ] Make the macOS shell issue commands and render state/events only.

## Phase 5 — Separate persistence

- [ ] Add repository interfaces for bookmarks, history, downloads, and objects.
- [ ] Keep JSON implementations initially; add SQLite behind the same interfaces.
- [ ] Add schema and serialization compatibility tests.

## Phase 6 — Windows proof

- [ ] Create a Windows shell with one CEF tab and shared-core navigation events.
- [ ] Validate the CEF distribution/bootstrap and CI path on Windows.

## Phase 7 — Mobile proofs

- [ ] Create an Android shell proof using the selected engine adapter.
- [ ] Create an iPhone shell proof using WKWebView by default.
- [ ] Run the same extraction script and normalized result contract on both.

## Phase 8 — Resume the product vision

- [ ] Build Product, ObjectGraph, CompareSession, and ActionPlan in the shared core.
- [ ] Build shared Lit surfaces for objects, Compare Mode, widgets, and internal pages.
- [ ] Add encrypted multi-device sync only after storage/version contracts are stable.
