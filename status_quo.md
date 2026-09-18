# Drift — Status Quo

> One page. Rewritten (not appended) when state changes materially. Read this first every session.
> Last updated: 2026-09-18 (SP-03 complete + BUG-23/24 fixed)

---

## What works

- CEF 152 bootstrap on macOS ARM64: window opens, multi-process confirmed
- **EP-01-T01 DONE** — CEF bootstrap (2026-09-16)
- **EP-01-T02 DONE** — Tab bar: open, close, drag-reorder, pin, favicon letter badge (2026-09-16)
- **EP-01-T03 DONE** — Address bar: URL navigation, search fallback, HTTPS lock icon (2026-09-16)
- **EP-01-T04 DONE** — Back/forward/reload buttons + amber loading progress bar (2026-09-17)
- **EP-01-T05 DONE** — Window management: fullscreen, zoom, resize, miniaturize, double-click-to-zoom (2026-09-17)
- **EP-01-T06 DONE** — Keyboard shortcuts: ⌘T ⌘W ⌘L ⌘R ⌘⇧T ⌘1–9 ⌃Tab ⌃⇧Tab (2026-09-17)
- **EP-01-T07 DONE** — Drift visual theme: Inter everywhere, warm palette, nav button hover/tint, URL host/path colour split (2026-09-17)
- **EP-01-T08 DONE** — Context menus: page/link/image right-click; tab strip right-click with New/Duplicate/Close/Close Others/Pin (2026-09-17)
- **EP-01-T09 DONE** — Bookmarks: star button in URL pill (⌘D), bookmarks bar (⌘⇧B toggle), JSON persistence at ~/Library/Application Support/Drift/bookmarks.json (2026-09-18)
- **EP-01-T10 DONE** — History: recorded on every page title change, 5000-entry cap, floating panel (⌘Y), double-click to navigate, Clear History, JSON persistence (2026-09-18)
- **EP-01-T11 DONE** — Downloads: CefDownloadHandler, NSSavePanel for "Save Link As", per-item "Show in Finder" + "✕" clear in download bar, amber progress, auto-hides 5s after complete (2026-09-18)
- bugs3.md: 24 bugs logged; BUG-01–BUG-24 all fixed

Key architecture facts in CLAUDE.md. Key gotchas in handover.md.

## What is half-built

Nothing.

## What is missing

- EP-01-T12: Chrome extension compatibility (SP-04, next)
- EP-02 onwards: Page understanding pipeline

## In flight

Nothing.

## Open decisions waiting on Anand

| Decision | Needed by | Task |
|---|---|---|
| VPN vendor: Mullvad vs Guardian | V1 planning | EP-09 |
| Analytics: Matomo self-hosted confirmed, or defer? | SP-14 | EP-06-T07 |
| Beta programme: invite-only vs open waitlist | SP-14 | EP-06 |
| Symbol server for Breakpad: self-hosted or defer? | SP-14 | EP-06-T06 |

## The single most useful next action

SP-04: EP-01-T12 Chrome extension compatibility — CRX load, chrome.* API surface.
