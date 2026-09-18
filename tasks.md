# Drift — Tasks

> One line per task. Detail in PRD.md. Reference by code only: "EP-04-T03 is blocked."
> Completed work moves to ## Done. Never deleted.

---

## SP-01 — Chromium Shell (part 1) · IN PROGRESS

- [x] EP-01-T01: Bootstrap CEF on macOS — basic window renders, process architecture
- [x] EP-01-T02: Tab bar — open, close, reorder (drag), pin, favicon
- [x] EP-01-T03: Address bar — URL navigation, autocomplete, search engine
- [x] EP-01-T04: Back / forward / reload, loading progress indicator
- [x] EP-01-T05: Window management — fullscreen, native macOS resize/zoom

## SP-02 — Chromium Shell (part 2)

- [x] EP-01-T06: Keyboard shortcuts — Chrome parity (⌘T, ⌘W, ⌘L, ⌘R, ⌘⇧T, ⌘1–9, ⌃Tab, ⌃⇧Tab)
- [x] EP-01-T07: Drift visual theme on chrome — warm cream palette, Inter, thin tab bar
- [x] EP-01-T08: Context menus — right-click on page, tab, link, image

## SP-03 — Browser Baseline

- [x] EP-01-T09: Bookmarks — add, folder, toolbar, import from Chrome/Safari
- [x] EP-01-T10: History — browsing history, search, clear
- [x] EP-01-T11: Downloads — download bar, progress, open/show in Finder

## SP-04 — Extension Compatibility

- [ ] EP-01-T12: Chrome extension compatibility — CRX load, chrome.* API surface

## SP-05 — Password Manager & Settings

- [ ] EP-01-T13: Password manager — save/fill credentials, local keychain integration
- [ ] EP-01-T14: Settings page — general, search engine, privacy, appearance

## SP-06 — Page Understanding Pipeline (part 1)

- [ ] EP-02-T01: DOM extraction module — JSON-LD, schema.org, OpenGraph parser
- [ ] EP-02-T02: Heuristic rule engine — CSS selectors for price, title, rating, availability

## SP-07 — Page Understanding Pipeline (part 2)

- [ ] EP-02-T03: Local ML classifier — product page vs non-product page
- [ ] EP-02-T04: Confidence scoring — combine all pipeline signals into 0–1 score
- [ ] EP-02-T05: Core ML model integration — local inference on macOS
- [ ] EP-02-T06: Performance harness — full pipeline < 500ms on M1 8GB (GD-07)
- [ ] EP-02-T07: Graceful degradation — stay silent if confidence < 0.5

## SP-08 — Product Schema + Core ML

- [ ] EP-03-T01: Product object schema (GD-02) — typed, validated, serialisable
- [ ] EP-03-T02: Object materialisation — pipeline output → Product object

## SP-09 — Object Recognition UI

- [ ] EP-03-T03: Recognition indicator — amber-gold pill in chrome ("1 product recognised")
- [ ] EP-03-T04: Object card (hover) — warm serif (Lora) name, key fields, "Add to Compare" CTA
- [ ] EP-03-T05: Object Panel (full, right-side) — all fields, two-column, pine green actions
- [ ] EP-03-T06: Object persistence — survives tab refresh, same-domain navigation
- [ ] EP-03-T07: Multi-object pages — recognise primary (most prominent) product only

## SP-10 — Compare Mode: Selection

- [ ] EP-04-T01: Compare Tray — persistent bottom bar, held product(s), cross-tab
- [ ] EP-04-T02: Cross-tab object state — held products survive tab switching for session

## SP-11 — Compare Mode: Compare View

- [ ] EP-04-T03: Compare View — two-column layout, matched field rows
- [ ] EP-04-T04: Field matching logic — align fields, handle missing fields gracefully
- [ ] EP-04-T05: Winner detection — lower price, higher rating → amber-gold cell highlight
- [ ] EP-04-T06: Compare View actions — "Take Action", "Save Comparison" (local)
- [ ] EP-04-T10: Compare limit enforcement — 2-product cap in V0

## SP-12 — Compare Mode: Action Engine

- [ ] EP-04-T07: Action Suggestion card — calm suggestion, non-pushy treatment
- [ ] EP-04-T08: Action Confirmation gate — minimal modal, stoppable mid-action
- [ ] EP-04-T09: Action execution — add-to-cart DOM interaction (Amazon, Best Buy, Flipkart)

## SP-13 — Widget Surface

- [ ] EP-05-T01: Widget surface container — renders when zero tabs open
- [ ] EP-05-T02: Tasks widget — local list, receives Product objects from EP-04
- [ ] EP-05-T03: Notes widget — local rich-text, captures Compare session summary
- [ ] EP-05-T04: Timer widget — countdown + stopwatch, standalone
- [ ] EP-05-T05: Widget layout — resize/reorder, preference saved locally
- [ ] EP-05-T06: Object graph connection — Compare complete → "Add to Tasks" offer
- [ ] EP-05-T07: New Tab trigger — widget surface as new tab page

## SP-14 — Distribution & Beta

- [ ] EP-06-T01: macOS app bundle — signed, notarised, Gatekeeper compliant
- [ ] EP-06-T02: .dmg installer — drag-to-Applications, auto-launch
- [ ] EP-06-T03: Auto-update — Sparkle 2.x, silent background updates
- [ ] EP-06-T04: Set as default browser — first launch prompt, system API
- [ ] EP-06-T05: First-run onboarding — Screen 11 design, import from Chrome/Safari
- [ ] EP-06-T06: Crash reporting — Breakpad, local logs, no cloud telemetry
- [ ] EP-06-T07: Basic analytics — Matomo opt-in only, local aggregate counts

---

## Backlog — Browser Baseline Polish (queued after SP-05)

- [ ] EP-01-T10a: History page as a tab (chrome://history-style) — HTML+JS rendered in a Drift-owned tab via custom scheme handler. Replaces the V0 NSPanel. Includes: grouped-by-day timeline, per-item timestamps, in-page search, per-item delete, "Delete browsing data" (time-range picker).
- [ ] EP-01-T10b: ML-driven history categorisation — auto-tag every visited URL into categories (Work, News, Shopping, Social, Docs, Video…). Local model, no LLM at inference. Category chips as filters in the history page. Training data: LLM-labelled seed set of URL+title pairs → distil into small local classifier (Core ML). Feeds into EP-07 Research Mode later.
- [ ] EP-01-T11a: Downloads — pause / resume / cancel per item, KB/s rate, ETA. Match Chrome's downloads shelf behaviour.
- [ ] EP-01-T11b: Downloads page (chrome://downloads-style) as a tab — full history of past downloads, search, re-download, show in Finder.

## Backlog — V1 (EP-07 through EP-13)

- [ ] EP-07: Research Mode — ambient detection, timeline feed, source attribution
- [ ] EP-08: Per-Tab Incognito & Guest Mode
- [ ] EP-09: VPN Integration — partner (Mullvad/Guardian), Pro tier
- [ ] EP-10: Object Library & Cross-Session Persistence
- [ ] EP-11: Widget Library & Cloud Sync
- [ ] EP-12: Windows Distribution
- [ ] EP-13: Pro Tier & Account System

## Backlog — V2+

- [ ] EP-14: App Store / Third-Party Widget Ecosystem
- [ ] EP-15: Auto Pilot (autonomous multi-step actions)
- [ ] EP-16: Expanded Object Types (Chart, Video, Company, Person)

---

## Done

- [x] Design: 34 screens across 3 mockup sets (Compare Mode, Research/Memory, Privacy/Widgets/VPN)
- [x] docs/design_specification.md — complete, Lora locked, hex values confirmed
- [x] PRD.md — 22 sections, all gate decisions + open source stack locked
- [x] CLAUDE.md — stack, architecture, repo structure, locked decisions
- [x] status_quo.md — cold-start anchor written
- [x] Serif typeface → Lora (2026-09-16)
- [x] Action execution targets → Amazon, Best Buy, Flipkart (2026-09-16)
- [x] EP-06-T08: Reproducible source checkout — generated artifacts excluded; pinned CEF bootstrap verified from a fresh clone (2026-09-18, PRD §22)
