# Drift — Tasks

> One line per task. Detail in PRD.md. Reference by code only: "EP-00-T03 is blocked."
> Completed work moves to `## Done` and is never deleted.

---

## Active Epic — Cross-Platform Migration

- [ ] EP-00-T00: Execute the approved four-platform migration (`_wip_cross_platform_migration.md`; PRD §21)

## SP-06 — Shared State Extraction · NEXT

- [ ] EP-00-T04: Extract bookmarks behind a tested shared-core state model
- [ ] EP-00-T05: Extract history and download state behind shared-core models
- [ ] EP-00-T06: Extract tab, navigation, and session state behind shared-core models

## SP-07 — Engine and Persistence Boundaries

- [ ] EP-00-T07: Introduce the platform-neutral BrowserEngine interface and macOS CEF adapter
- [ ] EP-00-T08: Preserve JSON compatibility, then add SQLite behind versioned repository interfaces
- [ ] EP-00-T09: Pass the macOS behavior-parity and performance gate after extraction

## SP-08 — Windows Proof

- [ ] EP-00-T10: Build a Windows shell with one CEF tab driven by shared commands/events
- [ ] EP-00-T11: Add pinned Windows CEF bootstrap and a Windows build/CI smoke path

## SP-09 — Mobile Proofs

- [ ] EP-00-T12: Build an Android engine proof and decide WebView versus GeckoView from measured capability gaps
- [ ] EP-00-T13: Build an iPhone WKWebView proof using the same shared command/event contract
- [ ] EP-00-T14: Produce one normalized extraction result on Android and iPhone through the shared core
- [ ] EP-00-T15: Pass the cross-platform foundation gate before feature sprints resume

## Post-Foundation Product Queue

- [ ] EP-01-T12: Desktop Chrome extension compatibility — MV3 CRX load and required chrome.* API surface
- [ ] EP-01-T13: Password manager — save/fill credentials with platform secure-storage adapters
- [ ] EP-01-T14: Settings — shared settings model with platform-appropriate surfaces
- [ ] EP-02-T01: DOM extraction — JSON-LD, schema.org, and OpenGraph into a normalized result
- [ ] EP-02-T02: Heuristic rule engine — selectors for price, title, rating, and availability
- [ ] EP-02-T03: Local ML classifier — product page versus non-product page
- [ ] EP-02-T04: Confidence scoring — combine pipeline signals into a 0–1 score
- [ ] EP-02-T05: Portable local inference — Core ML/ONNX/platform adapters behind one contract
- [ ] EP-02-T06: Performance harness — enforce platform-specific pipeline budgets
- [ ] EP-02-T07: Graceful degradation — stay silent when confidence is below 0.5
- [ ] EP-03-T01: Product schema — typed, validated, serializable shared-core model
- [ ] EP-03-T02: Object materialization — normalized pipeline output to Product
- [ ] EP-03-T03: Recognition indicator — amber-gold product-recognized signal
- [ ] EP-03-T04: Object card — key fields and Add to Compare action
- [ ] EP-03-T05: Object Panel — complete product fields and actions
- [ ] EP-03-T06: Object persistence — survive refresh and same-domain navigation
- [ ] EP-03-T07: Multi-object pages — identify the primary product in the first release
- [ ] EP-04-T01: Compare Tray — persistent cross-tab product selection
- [ ] EP-04-T02: Cross-tab object state — preserve held products during the session
- [ ] EP-04-T03: Compare View — two-column matched-field layout
- [ ] EP-04-T04: Field matching — align fields and handle missing values
- [ ] EP-04-T05: Winner detection — highlight lower price and higher rating
- [ ] EP-04-T06: Compare actions — Take Action and local Save Comparison
- [ ] EP-04-T07: Action suggestion — calm, non-pushy recommendation
- [ ] EP-04-T08: Action confirmation — explicit, stoppable human gate
- [ ] EP-04-T09: Action execution — Amazon, Best Buy, and Flipkart adapters
- [ ] EP-04-T10: Compare limit — two products in the first release
- [ ] EP-05-T01: Widget surface container — useful empty-tab/new-tab state
- [ ] EP-05-T02: Tasks widget — local list connected to Product objects
- [ ] EP-05-T03: Notes widget — local rich text and comparison capture
- [ ] EP-05-T04: Timer widget — countdown and stopwatch
- [ ] EP-05-T05: Widget layout — resize, reorder, and local preference persistence
- [ ] EP-05-T06: Object graph connection — offer Add to Tasks after a comparison
- [ ] EP-05-T07: New-tab trigger — expose the widget surface as the new tab page
- [ ] EP-06-T01: macOS signed and notarized app bundle
- [ ] EP-06-T02: macOS DMG installer
- [ ] EP-06-T03: macOS auto-update through Sparkle
- [ ] EP-06-T04: Platform default-browser registration and prompt
- [ ] EP-06-T05: First-run onboarding and supported browser-data import
- [ ] EP-06-T06: Local crash reporting and platform symbol management
- [ ] EP-06-T07: Opt-in privacy-preserving analytics, or explicit launch deferral

---

## Backlog — Browser Baseline Polish

- [ ] EP-01-T10a: Shared History internal page — grouped timeline, search, item deletion, and time-range clearing
- [ ] EP-01-T10b: Local history categorization — on-device category model and filters
- [ ] EP-01-T11a: Downloads — pause, resume, cancel, transfer rate, and ETA
- [ ] EP-01-T11b: Shared Downloads internal page — history, search, repeat download, and reveal action

## Backlog — Scale

- [ ] EP-07: Research Mode — ambient detection, timeline feed, and source attribution
- [ ] EP-08: Per-tab incognito and Guest Mode
- [ ] EP-09: VPN integration — partner evaluation and Pro tier
- [ ] EP-10: Object Library and cross-session persistence
- [ ] EP-11: Widget Library and encrypted multi-device sync
- [ ] EP-13: Pro tier and optional account system

## Backlog — Platform

- [ ] EP-14: App Store / third-party widget ecosystem
- [ ] EP-15: Auto Pilot with human confirmation gates
- [ ] EP-16: Expanded object types — Chart, Video, Company, Person

---

## Done

- [x] EP-00-T02: Added platform-neutral IDs/commands/events/errors, deterministic reducer/codec, and tests (2026-09-18)
- [x] EP-00-T03: Captured source-derived macOS JSON fixtures and defined the manual behavior-parity gate (2026-09-18)
- [x] EP-00-T01: Locked the four-platform capability matrix and architecture contract (2026-09-18)
- [x] EP-01-T01: Bootstrap CEF 152 on macOS with the required multi-process architecture
- [x] EP-01-T02: Tab bar — open, close, drag-reorder, pin, and favicon badge
- [x] EP-01-T03: Address bar — URL navigation, search fallback, and HTTPS indicator
- [x] EP-01-T04: Back, forward, reload/stop, and loading progress
- [x] EP-01-T05: macOS window management — resize, fullscreen, zoom, and minimize
- [x] EP-01-T06: Keyboard shortcuts — core Chrome-parity navigation and tab commands
- [x] EP-01-T07: Drift chrome theme — bundled Inter and warm design tokens
- [x] EP-01-T08: Page/link/image and tab-strip context menus
- [x] EP-01-T09: Basic bookmarks — toggle, toolbar, navigation, and JSON persistence
- [x] EP-01-T10: Basic history — record, panel, navigation, clear, and JSON persistence
- [x] EP-01-T11: Basic downloads — save flow, progress, clear, and reveal in Finder
- [x] EP-06-T08: Reproducible macOS checkout — fetched/checksummed CEF; generated artifacts excluded (2026-09-18)
- [x] Design: 34 screens across three mockup sets
- [x] `docs/design_specification.md`: Lora, Inter, palette, and interaction language locked
- [x] PRD/engineering/session documentation baseline created
- [x] Serif typeface: Lora locked (2026-09-16)
- [x] Action targets: Amazon, Best Buy, and Flipkart locked (2026-09-16)
