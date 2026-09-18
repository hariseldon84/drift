# Drift — Product Requirements Document

> Authoritative source for what Drift is, why decisions were made, and what gets built.
> Last updated: 2026-09-18. Maintained by Anand.
> Design source of truth: docs/design_specification.md
> Task tracking: tasks.md (uses nomenclature defined in §6 of this document)

---

## 1. Vision

Drift is a multi-device browser for macOS, Windows, Android, and iPhone that understands what is inside pages, not just what pages say. It recognises typed objects — products, prices, articles, people, job listings — and lets the user work across tabs and devices as if the web were one structured database.

**One-line positioning:** "Drift understands the things inside pages."

Not "AI browser." Not "smart browser." The intelligence is the object model — every other feature flows from it.

**Emotional register** (from designinspirationstory.md): opening Drift feels like walking into a warm mountain cafe in Shimla at 8am. The web is the beautiful, chaotic road outside. Drift is where you arrive — calm, organised, unhurried. Intelligence is light coming through the window, not a dashboard demanding attention.

---

## 2. Target User

**Primary:** Knowledge workers who spend 4+ hours a day in a browser — researchers, shoppers making considered purchases, analysts, students. They feel overwhelmed by tab sprawl and want the web to work with them, not against them.

**Secondary:** Privacy-conscious users who want local-first intelligence with no cloud dependency.

**Not in the first public release:** Enterprise IT buyers and developers seeking a DevTools-focused browser. Mobile is part of the product scope, but mobile shells reach capability parity in stages rather than delaying every release until all four platforms are identical.

---

## 3. Core Moat

Three layers, in order of defensibility:

1. **Object Understanding** — browser-native semantic recognition of web content into typed, structured objects. Not a sidebar asking GPT about a screenshot. The browser itself reads the page.
2. **Cross-Tab Composition** — objects persist across tabs and sessions. Select a product on Amazon, the same product on Best Buy, compare them as structured data. No other browser has a persistent cross-tab object graph.
3. **Action Engine with Human Gates** — Drift can act (add to cart, fill a form, navigate) on behalf of the user. Every consequential action requires explicit human confirmation.

Everything that gets built must advance at least one of these three layers, or it waits.

---

## 4. Design Principles

Non-negotiable. Full design language in docs/design_specification.md.

1. Normal browsing remains normal. Intelligence is always on but never intrusive.
2. Interaction must be invited. Drift does not surface intelligence without a signal.
3. Local-first. Inference runs on device. Cloud is a fallback, only with explicit permission.
4. Human control over consequential actions. Every action has a confirmation gate.
5. Chrome-like familiarity. Learning curve is near zero.
6. Sacred performance. The browser must feel as fast as Chrome or faster.

---

## 5. Gate Decisions

These were blocked in docs/07_PRD_GATE.md. All resolved here with reasoning. Do not re-litigate without a documented reason to change.

### GD-01 — Four-platform product, staged implementation
**Decision (supersedes the 2026-09-15 Mac-only decision):** Drift targets macOS, Windows, Android, and iPhone. The current macOS implementation remains the reference browser while a shared C++20 core, engine adapters, and native platform shells are established. Windows and mobile proofs happen before the intelligence/product feature roadmap resumes.
**Why:** Mac-first code organized around AppKit and CEF would make every later platform a rewrite. Requiring identical launch dates would also slow validation. One shared product model with staged, runnable platform slices preserves momentum without treating mobile or Windows as an afterthought.
**Revisit when:** A platform proof demonstrates that a selected engine cannot satisfy a required capability, or measured maintenance cost shows the shared boundary is more expensive than a platform-specific implementation.

### GD-02 — V0 Object Taxonomy: Product only
**Decision:** V0 recognises exactly one object type — Product.
**Why:** Compare Mode is the V0 killer workflow. Generalising the object model early risks shipping a half-baked model for all types rather than a world-class one for products.

**Product object schema:**
```
Product {
  name:         string
  brand:        string
  price:        { amount: number, currency: string }
  rating:       { score: number (0–5), count: number }
  availability: "in_stock" | "out_of_stock" | "limited"
  images:       string[]       // URLs, max 5
  key_specs:    string[]       // extracted bullet points, max 5
  source_url:   string
  source_domain: string
  extracted_at: ISO8601 timestamp
}
```
**V1 adds:** Article, Person, JobListing, Chart.

### GD-03 — Research Mode timing: V1
**Decision:** Research Mode ships in V1.
**Why:** Ambient topic detection across tabs is non-trivial. Compare Mode proves the object model first. Research Mode extends it.

### GD-04 — Account model: None in V0, optional in V1
**Decision:** V0 requires no account. Everything is local. V1 introduces optional Drift account (Sign in with Google or Apple) for cloud sync only.
**Why:** Account requirement adds friction, raises privacy questions, and is unnecessary when all V0 data is local.

### GD-05 — Packaging: Free in V0, Free + Pro in V1
**Free (V0 and V1):** Full browser, Compare Mode (unlimited), Widget surface — Tasks, Notes, Timer (local storage only).
**Pro ($9.99/month, launches with V1):** Cloud sync (notes, tasks, object library), VPN integration, Research Mode advanced features, Widget library (additional Drift-built widgets).
**Why:** V0 ships Free only — no monetisation pressure while the core is being proven. Pro tier is designed in V0 but not activated. Free tier must be genuinely useful as a daily browser; Pro is additive.

### GD-06 — Intelligence strategy: Local-first, cloud fallback with permission
**First release:** 100% local. No cloud calls. Object recognition, page understanding, and comparison logic run on-device through a portable inference contract: Core ML where appropriate, ONNX Runtime or platform runtimes elsewhere.
**Scale phase:** Cloud fallback may be available, off by default and only with user permission. Only scoped structured data may be sent — never raw browsing history.
**Why:** Local-first is a core principle and competitive differentiator. Cloud-first requires server infrastructure day one and compromises the privacy promise.

### GD-07 — Performance budgets

The existing figures are the macOS reference budgets, measured on an M1 MacBook Pro with 8GB RAM. EP-00 must establish equivalent reference devices and budgets for Windows, Android, and iPhone before claiming cross-platform parity.

| Metric | Budget |
|---|---|
| Browser cold start | < 4 seconds |
| Browser warm start | < 2 seconds |
| Object recognition per page | < 500ms post page-load |
| Compare view render (2 products) | < 1 second |
| Widget surface load | < 300ms |
| Memory overhead vs Chrome (same tabs) | < 25% additional |
| Page load parity vs Chrome | within 5% |

**Why:** Drift cannot be slower than Chrome. Intelligence overhead that degrades browsing violates Design Principle 6. These are targets from day one, not post-launch optimisations.

### GD-08 — Success metrics (V0)

| Metric | Target | Timeframe |
|---|---|---|
| Daily Active Users | 1,000 | 3 months post-launch |
| Weekly Compare Mode users (% of DAU) | 40% | 3 months |
| Compare sessions per active user per week | ≥ 2 | 3 months |
| D7 retention | > 30% | From install |
| D30 retention | > 20% | From install |
| Default browser set rate | > 25% | 3 months |

**Leading indicators (weekly):** Installs, compare sessions/day, object recognition success rate, P95 recognition latency.

---

## 6. Nomenclature

All tasks, sprints, bugs, and gate decisions use this system. Reference by code in all conversations — never by description alone.

| Type | Format | Example | Where tracked |
|---|---|---|---|
| Epic | `EP-XX` | `EP-01` | tasks.md + PRD.md §8 |
| Sprint | `SP-XX` | `SP-03` | tasks.md |
| Task | `EP-XX-TXX` | `EP-02-T04` | tasks.md |
| Bug | `BUG-XXX [EP-XX]` | `BUG-012 [EP-04]` | bugs3.md |
| Gate Decision | `GD-XX` | `GD-01` | PRD.md §5 |

**Rules:**
- Every task belongs to exactly one epic.
- Every bug references its epic.
- Sprint assignments live in tasks.md.
- In conversation: "EP-04-T03 is blocked" — not "the compare view task is blocked."
- Bug numbering is sequential and never restarts across bugs.md files.

---

## 7. Release Plan

### Foundation — four-platform viability

**Definition of done:** The working macOS browser still passes its behavior-parity gate; a Windows CEF shell and Android/iPhone shells execute the same versioned command/event contract; both mobile shells produce one normalized extraction result through the shared core.

**Foundation epic:** EP-00 — Cross-Platform Foundation.

| Sprint | Focus | Exit evidence |
|---|---|---|
| SP-04 | Scope and architecture | Canonical documents and capability matrix agree |
| SP-05 | Contracts and characterization | Shared value types and deterministic tests run |
| SP-06 | State extraction | Existing bookmarks/history/downloads/tabs use shared models |
| SP-07 | Engine and persistence boundaries | macOS runs through the adapter with data compatibility |
| SP-08 | Windows proof | One CEF tab navigates through shared commands/events |
| SP-09 | Mobile proofs | Android and iPhone run the same core contract |

### First lovable release — Product comparison

**Definition of done:** A user can install a supported Drift build, use it as a browser, and complete a product comparison across two sites without friction. Platform release dates may be staged, but product logic and data contracts must remain shared.

**Product epics:**

- EP-01: Browser Baseline
- EP-02: Page Understanding Pipeline
- EP-03: Product Object Recognition
- EP-04: Compare Mode
- EP-05: Connected Widget Surface
- EP-06: Distribution & Installation

### Scale

**Goal:** The object model generalizes, optional encrypted multi-device sync is introduced, the Pro tier launches, and Research Mode ships.
**Epics:** EP-07 through EP-13 (see §9).

### Platform

**Goal:** Ecosystem, developer platform, and carefully gated autonomous actions.
**Epics:** EP-14 through EP-16 (see §10).

---

## 8. Epic Specifications

### EP-00 — Cross-Platform Foundation

**Goal:** Turn the working macOS implementation into the first adapter of a four-platform product without discarding its behavior.
**Architecture:** Native platform shells issue versioned commands to a shared C++20 state/product core. Engine-specific behavior is isolated behind `BrowserEngine`; normalized events flow back to the core. Lit is used only for Drift-owned product surfaces and internal pages.
**Detailed contract:** `docs/08_cross_platform_architecture.md`.
**Sprints:** SP-04 through SP-09.

**Gate:** Product feature sprints do not resume until macOS parity, Windows navigation, and Android/iPhone normalized-extraction proofs are runnable.

| Code | Task |
|---|---|
| EP-00-T00 | Execute and track the complete migration through the temporary phase checklist |
| EP-00-T01 | Lock the platform capability matrix and architecture contract (complete) |
| EP-00-T02 | Add platform-neutral browser IDs, commands, events, errors, and deterministic tests (complete) |
| EP-00-T03 | Capture source-derived macOS fixtures and define its manual behavior-parity gate (complete; execution evidence is required at T09) |
| EP-00-T04 | Extract bookmarks as the first tested shared-core vertical slice |
| EP-00-T05 | Extract history and download state into shared-core models |
| EP-00-T06 | Extract tab, navigation, and session state into shared-core models |
| EP-00-T07 | Introduce `BrowserEngine` and move CEF integration into the macOS/desktop adapter |
| EP-00-T08 | Preserve JSON compatibility, then add SQLite behind repository interfaces |
| EP-00-T09 | Pass the macOS parity and performance gate after extraction |
| EP-00-T10 | Prove Windows navigation with one CEF tab and shared commands/events |
| EP-00-T11 | Add the Windows CEF bootstrap and build/CI smoke path |
| EP-00-T12 | Prove Android capabilities and decide WebView versus GeckoView from measurements |
| EP-00-T13 | Prove the iPhone WKWebView shell against the same shared contract |
| EP-00-T14 | Produce one normalized extraction result through both mobile adapters |
| EP-00-T15 | Pass the cross-platform foundation gate before product feature work resumes |

### EP-01 — Chromium Shell & Browser Baseline

**Goal:** A working, stable browser users would not be embarrassed to use as their daily driver.
**Sprint:** SP-01 through SP-03 completed; remaining work resumes after EP-00.
**Out of first release:** Custom tab groups, vertical tabs, sidebar, and cloud sync.

**Tasks:**
| Code | Task |
|---|---|
| EP-01-T01 | Bootstrap CEF on macOS — basic window renders, process architecture |
| EP-01-T02 | Tab bar — open, close, reorder (drag), pin, favicon |
| EP-01-T03 | Address bar — URL navigation, autocomplete (history + bookmarks), search engine |
| EP-01-T04 | Back / forward / reload, loading progress indicator |
| EP-01-T05 | Window management — fullscreen, native macOS resize/zoom |
| EP-01-T06 | Keyboard shortcuts — Chrome parity (⌘T, ⌘W, ⌘L, ⌘R, ⌘⇧T, ⌘1–9, etc.) |
| EP-01-T07 | Drift visual theme on chrome — warm cream palette, Inter, thin tab bar per design_specification.md |
| EP-01-T08 | Context menus — right-click on page, tab, link, image |
| EP-01-T09 | Basic bookmarks — toggle, toolbar, navigation, and JSON persistence (complete); folders/import remain backlog |
| EP-01-T10 | Basic history — record, panel, navigation, clear, and JSON persistence (complete); search/internal page remain backlog |
| EP-01-T11 | Basic downloads — save flow, progress, clear, and reveal in Finder (complete); pause/resume/history remain backlog |
| EP-01-T12 | Desktop Chrome extension compatibility — MV3 CRX load and required chrome.* API surface |
| EP-01-T13 | Password manager — save/fill credentials using platform secure-storage adapters |
| EP-01-T14 | Settings — shared settings model with platform-appropriate surfaces |

### EP-02 — Page Understanding Pipeline

**Goal:** Drift reads a page and extracts structured meaning, entirely on-device, within 500ms.
**Architecture:** Four-stage pipeline: (1) deterministic extraction, (2) heuristic rules, (3) local classifier, and (4) portable local model inference. Normalized inputs/outputs live in the shared core; Core ML, ONNX Runtime, and mobile platform runtimes are adapters. Cloud is not in the first-release pipeline.
**Sprint:** Resumes after the EP-00 foundation gate.

**Tasks:**
| Code | Task |
|---|---|
| EP-02-T01 | DOM extraction module — JSON-LD, schema.org, OpenGraph parser |
| EP-02-T02 | Heuristic rule engine — CSS selector patterns for price, title, rating, availability |
| EP-02-T03 | Local ML classifier — identify product page vs non-product page |
| EP-02-T04 | Confidence scoring — combine all pipeline stage signals into 0–1 score |
| EP-02-T05 | Portable local inference — Core ML/ONNX/platform adapters behind one contract |
| EP-02-T06 | Performance harness — enforce the platform-specific budgets in GD-07 |
| EP-02-T07 | Graceful degradation — if no stage produces confidence > 0.5, stay silent |

### EP-03 — Product Object Recognition

**Goal:** When the pipeline identifies a product with sufficient confidence, Drift materialises a typed Product object visible to the user.
**Confidence threshold:** 0.75 to show recognition indicator. 0.5–0.75 logs silently, no UI.
**Sprint:** After the EP-00 foundation gate; assign during product sprint planning.

**Tasks:**
| Code | Task |
|---|---|
| EP-03-T01 | Product object schema implementation (GD-02) — typed, validated, serialisable |
| EP-03-T02 | Object materialisation — pipeline output → Product object |
| EP-03-T03 | Recognition indicator — amber-gold pill in chrome ("1 product recognised") |
| EP-03-T04 | Object card (hover state) — warm serif name, key fields, "Add to Compare" CTA |
| EP-03-T05 | Object Panel (full, right-side) — all fields, two-column layout, pine green actions |
| EP-03-T06 | Object persistence — survives tab refresh, survives same-domain navigation |
| EP-03-T07 | Multi-object pages — recognise primary (most prominent) product only in V0 |

### EP-04 — Compare Mode

**Goal:** The flagship V0 workflow. User compares two products from different sites as structured objects.
**Flow:** Recognise → Add to Compare → Compare Tray → browse second site → Add second → Compare View.
**Design reference:** Screens 5–9, design_inspirations/Drift - Compare Mode Mockups.html.
**Sprint:** After the EP-00 foundation gate; assign during product sprint planning.

**Tasks:**
| Code | Task |
|---|---|
| EP-04-T01 | Compare Tray — persistent bottom bar, shows held product(s), cross-tab |
| EP-04-T02 | Cross-tab object state — held products survive tab switching, persist for session |
| EP-04-T03 | Compare View — two-column layout, matched field rows per design spec |
| EP-04-T04 | Field matching logic — align fields across two Product objects, handle missing fields |
| EP-04-T05 | Winner detection — flag lower price, higher rating with amber-gold cell highlight |
| EP-04-T06 | Compare View actions — "Take Action", "Save Comparison" (local, V0) |
| EP-04-T07 | Action Suggestion card — calm suggestion ("Add the cheaper one to cart?") |
| EP-04-T08 | Action Confirmation gate — minimal modal, stoppable mid-action |
| EP-04-T09 | Action execution — DOM interaction for add-to-cart (Amazon, Best Buy, Flipkart — V0 targets locked 2026-09-16) |
| EP-04-T10 | Compare limit — 2-product only in V0; 3+ is V1 |

### EP-05 — Connected Widget Surface

**Goal:** Empty browser state becomes a calm, useful productivity surface connected to the object graph.
**V0 widgets:** Tasks (local, object graph connected), Notes (local), Timer (standalone).
**Design reference:** Evolution of Screen 1, design_inspirations/Drift - Compare Mode Mockups.html.
**Sprint:** After the EP-00 foundation gate; assign during product sprint planning.

**Tasks:**
| Code | Task |
|---|---|
| EP-05-T01 | Widget surface container — renders when zero tabs open, warm cream base |
| EP-05-T02 | Tasks widget — local task list, add/complete/delete, receives Product objects from EP-04 |
| EP-05-T03 | Notes widget — local rich-text notes, manual entry + EP-04 session capture |
| EP-05-T04 | Timer widget — countdown + stopwatch, beautiful, standalone |
| EP-05-T05 | Widget layout — user can resize/reorder, preference saved locally |
| EP-05-T06 | Object graph connection — Compare session completion offers "Add to Tasks" |
| EP-05-T07 | New Tab trigger — widget surface available as new tab page also |

### EP-06 — Distribution & Installation

**Goal:** Users can install, update, and register Drift safely on every released platform. Each native shell owns its operating-system packaging and trust integration.
**Design reference:** Screen 11 (Onboarding), design_inspirations/Drift - Compare Mode Mockups.html.
**Sprint:** After the EP-00 platform proofs and product gates.

**Tasks:**
| Code | Task |
|---|---|
| EP-06-T01 | macOS app bundle — signed, notarized, Gatekeeper compliant |
| EP-06-T02 | macOS DMG installer — drag-to-Applications, safe first launch |
| EP-06-T03 | macOS auto-update — Sparkle with signed feeds |
| EP-06-T04 | Platform default-browser registration and first-run prompt |
| EP-06-T05 | First-run onboarding and supported browser-data import per platform |
| EP-06-T06 | Local crash logs plus platform symbol management; submission is opt-in |
| EP-06-T07 | Privacy-preserving analytics only with opt-in, or explicit launch deferral |
| EP-06-T08 | Reproducible macOS source checkout — generated output and CEF binaries excluded; pinned/checksummed macOS bootstrap (complete) |

---

## 9. Scale Epics — Overview

Full task specifications written when V0 ships or when sprint planning for V1 begins.

### EP-07 — Research Mode
Ambient topic detection across open tabs. Timeline/feed view, newest-first, source-attributed. Triggered at 4+ tabs on same topic. Amber-gold accent on highest-signal item.
**Design reference:** Screens 1–2, design_inspirations/Drift - Research and Memory Mockups.html.

### EP-08 — Per-Tab Incognito & Guest Mode
Single tab private session in a normal window. Subtle visual treatment on tab. No cookies/history for that tab. Guest Mode: separate clean BrowserContext window handed off temporarily.

### EP-09 — VPN Integration (Partner)
Partner with Mullvad or Guardian (vendor evaluation pending — open decision). Paid Pro tier. No Drift-built infrastructure. Legal compliance review per market required before launch.

### EP-10 — Object Library & Cross-Session Persistence
Right-side panel, all collected objects grouped by type (Products, Articles, People). Pine green tray icon trigger. Cloud sync for Pro users.
**Design reference:** Screen 7, design_inspirations/Drift - Research and Memory Mockups.html.

### EP-11 — Widget Library & Cloud Sync
Additional Drift-built widgets (Calendar, Reading List, 2–3 more TBD). Cloud sync for Tasks and Notes (Pro). Local-first remains default; sync is opt-in.

### EP-12 — Retired into EP-00 and EP-06
The former Windows-only distribution epic is superseded. Windows architecture and the runnable proof are part of EP-00; release packaging belongs to EP-06. The task code is retained here so historical references do not silently change meaning.

### EP-13 — Pro Tier & Account System
Optional Drift account (Sign in with Google or Apple). Unlocks cloud sync, VPN, Research Mode advanced features, Widget library. $9.99/month or $99/year.

---

## 10. Platform Epics — Placeholder

### EP-14 — App Store / Third-Party Widget Ecosystem
Developer SDK, widget review process, trust and safety. A company within a company — do not open until V1 is proven.

### EP-15 — Auto Pilot
Autonomous multi-step action sequences. User defines a workflow; Drift executes with confirmation gates at each consequential step.

### EP-16 — Expanded Object Types
Chart, Video, Company, Person as first-class typed objects. Cross-type composition (e.g., compare two companies using their person graphs).

---

## 11. Open Source Stack Decisions

Locked 2026-09-16. These replace "build from scratch" wherever a proven open source tool exists.

| Component | Tool | Rationale |
|---|---|---|
| Shared product/state core | **C++20** | Fits the working CEF implementation, crosses native platforms, and permits predictable latency. Reliability comes from RAII, tests, sanitizers, and narrow ABI boundaries — not from the language being close to the machine. |
| Desktop Chromium embedding | **CEF (Chromium Embedded Framework)** | Production-grade Chromium embedding on macOS and Windows. The pinned binary is fetched per platform, never committed. |
| Android engine | **Android WebView proof; GeckoView fallback** | Decide from measured capability gaps during EP-00, not from framework preference. |
| iPhone engine | **WKWebView baseline** | Compatible with normal App Store distribution. Alternative engines require separate entitlement/region evaluation and are not the baseline. |
| Native shells | **AppKit/Objective-C++, Win32/WinUI, Kotlin, Swift** | Lifecycle, accessibility, secure storage, permissions, and engine hosting remain platform-native. |
| Shared Drift surfaces | **TypeScript + Lit** | Reuse product UI and internal pages without pretending the browser engine or native shell is a web component. |
| Structured data extraction (EP-02-T01) | **Standards-based parser modules** — JSON-LD, schema.org, OpenGraph | Use the extruct-style deterministic approach without shipping a Python runtime. Heuristic + ML stages remain custom. |
| Local ML inference (EP-02-T05, EP-03) | **Portable contract with Core ML, ONNX Runtime, or platform adapters** | One normalized model/result boundary allows hardware-appropriate runtimes without forking product logic. |
| Local storage — object graph + widgets | **SQLite + SQLCipher-capable repository** | Use the platform SQLite distribution or a pinned build. Encryption keys live in platform secure storage; do not roll custom key management. |
| macOS auto-update (EP-06-T03) | **Sparkle** | macOS-specific release infrastructure; other platforms use their native distribution/update path. |
| Crash reporting (EP-06-T06) | **CEF/Breakpad on desktop plus native mobile crash facilities** | Keep collection local by default and treat upload as opt-in. Every released native binary needs symbols. |
| Testing | **CTest/core tests + adapter contract tests + platform smoke runners** | Lit/internal pages may use browser tooling, but no single UI runner proves native engine integration on all platforms. |
| Analytics — opt-in only (EP-06-T07) | **Matomo** (self-hosted candidate) | Requires server operations and a separate privacy review. Defer analytics entirely if that work is not ready for distribution. |

**What has NO open source substitute:**
- Product extraction heuristics (EP-02-T02) — custom CSS selector rules per retailer
- Product ML classifier (EP-02-T03) — custom trained model, fine-tuned on product pages
- Compare Mode UI and object graph (EP-04) — core product, built from scratch
- Action execution DOM layer (EP-04-T09) — retailer-specific, no general library

---

## 12. System Requirements

The existing implementation requires macOS 13 Ventura or later and is optimized first on Apple Silicon. Exact supported Windows, Android, and iPhone versions are an EP-00 output: they must be selected from engine support, security-update lifetime, store rules, and measurements from the platform proofs rather than guessed in the PRD.

| Platform | Foundation baseline | Version decision gate |
|---|---|---|
| macOS | macOS 13+, CEF 152, Apple Silicon primary | Revalidate before public release |
| Windows | CEF 152 desktop proof | Lock after the SP-08 build/CI proof |
| Android | WebView proof; GeckoView fallback | Lock after SP-09 capability/performance comparison |
| iPhone | WKWebView proof | Lock after SP-09 capability/performance validation |

Every public release must receive security updates supported by its selected engine and operating-system baseline.

---

## 13. Localisation

**V0: English only.** All UI copy, error messages, and widget labels in English.
**Currency:** Display extracted currency as-is from source page (do not convert). Flipkart prices display in ₹, Amazon in $ or local currency per page.
**Date/time:** System locale for timestamps in widgets and object panels.
**RTL:** Not supported in V0. Deferred to V1.
**V1:** Localisation framework introduced. Hindi and additional languages evaluated based on Flipkart user base data.

---

## 14. Accessibility

**V0 baseline: WCAG 2.1 AA for all Drift-built UI surfaces** (browser chrome, object panel, compare view, widget surface, onboarding). Web page content rendered in the viewport is not in scope — that is the page author's responsibility.

Specific requirements:
- Keyboard navigation: all Drift UI panels fully operable without a mouse. Tab order logical, focus visible.
- Screen reader: VoiceOver on Apple platforms, Narrator on Windows, and TalkBack on Android for Drift-owned controls and surfaces.
- Colour contrast: amber-gold `#D4820A` on cream `#FAF7F2` — verify contrast ratio meets 3:1 minimum for large text / UI components (not body copy). Use deeper amber `#B86E08` if any element fails.
- No colour-only information: the incognito tab indicator uses both colour (pine green border) and icon (shield) — not colour alone.
- Motion: respect `prefers-reduced-motion`. All transition animations disabled when set.

---

## 15. Error Handling Philosophy

**Principle: Drift fails silently rather than noisily. The user's browsing is never interrupted by Drift's internal errors.**

| Situation | Drift behaviour |
|---|---|
| Page understanding pipeline fails | Stay silent. No indicator shown. Log locally. |
| Object recognition confidence < 0.5 | Stay silent. |
| Object recognition confidence 0.5–0.75 | Log locally. No UI. |
| Object recognition confidence ≥ 0.75 | Show amber-gold indicator. |
| Compare field missing on one product | Show "—" in that cell. Never hide the row. |
| Action execution fails (add to cart) | Show calm inline message in confirmation modal: "Something went wrong. Try opening the page directly." No alarm colour. |
| Local ML model unavailable | Fall back to heuristic stage only. No user-facing message. |
| Widget data corrupt | Reset widget to empty state silently. Do not crash. |
| Extension crash | Isolate in extension process (CEF handles this). Show tab-level error only if rendering is affected. |

**Debug mode** (accessible via Settings → Advanced → Show recognition diagnostics): surfaces pipeline confidence scores, extraction source, and object field origins. Off by default. Intended for beta feedback, not production use.

---

## 16. Extension Compatibility Scope

**Desktop only for the first release.** macOS and Windows target Manifest V3 (MV3). Chrome deprecated MV2; Drift does not re-introduce it. Android and iPhone extension compatibility is explicitly outside the foundation and first mobile scope.

**Must work before desktop extension compatibility is declared complete:**
- uBlock Origin (MV3 version) — most-used extension globally
- 1Password / Bitwarden — password managers (complement Drift's own)
- Grammarly — content script on all pages
- Google Translate — page-level translation

**Extension interaction with Drift's object recognition:**
- Content scripts that modify the DOM run before Drift's page understanding pipeline. This is acceptable — Drift reads the final rendered DOM, not the raw HTML.
- Extensions that aggressively remove DOM nodes (some ad blockers) may reduce extraction confidence. This is logged but not surfaced to the user.
- Drift does not expose its object graph to extensions in V0. No extension API for the object model.

**Explicitly out of scope:**
- MV2 extensions
- Chrome DevTools extensions
- Extensions requiring `chrome.debugger` API
- Chrome-extension parity on Android or iPhone

---

## 17. Privacy Model Detail

**What never leaves the device or participates in sync:**
- Browsing history
- Page content (HTML, text, images)
- Downloaded files and download history
- Keystrokes or form data
- Passwords and credentials (future password sync requires a separate security decision)

**What is local by default but may join optional end-to-end encrypted sync:**
- Bookmarks and user settings
- Saved Product objects and comparisons (structured fields only; never source HTML)
- Widget data such as tasks and notes

Each category is separately enabled. The sync service stores ciphertext and cannot use sync data for inference, analytics, or advertising. Versioned schemas, migrations, conflict rules, key recovery, deletion propagation, and a second-device threat model must be locked before implementation.

**What may leave the device through a separate explicit opt-in:**
- Anonymous aggregate usage counts (DAU, compare sessions per day) — no user identity, no session linking
- Crash reports (local by default; user can choose to submit via a "Send crash report" prompt post-crash)

**What opt-in analytics collects (Matomo, self-hosted):**
- Count of browser sessions per day (no user ID)
- Count of compare sessions per day
- Object recognition success/failure rate (aggregate)
- Does NOT collect: URLs visited, page content, search queries, object data

**With cloud inference fallback enabled in the Scale phase (separate consent, off by default):**
- Only the minimum typed fields needed for that one request are sent; never source HTML, history, URLs, or session context.
- Transport is encrypted and request data is not retained or added to sync.
- Sync consent never implies cloud-inference consent, and neither implies analytics consent.

---

## 18. Browser Security Model

**Process isolation:** Each selected engine's supported isolation model is preserved; Drift does not weaken it for code sharing. CEF renderer processes, Android/iPhone web-content processes, and the shared core communicate through scoped, serialized data rather than executable code or raw DOM references.

**Content Security Policy:** Drift respects page-level CSP headers. Drift-owned product UI is hosted by the shell or a privileged internal surface, not inserted as page-owned UI that a site can impersonate.

**Object data trust model:** Data extracted from a page is treated as untrusted user input. All extracted strings are sanitised before display in Drift's UI. An attacker cannot execute code by crafting a product page with malicious markup in a product name field.

**Action engine security:** Engine adapters execute typed actions using the safest supported engine API. Consequential action confirmation is rendered in trusted Drift chrome or a privileged Drift surface, not by the page.

**First-release stance on tracking:** Native tracker blocking is deferred while the product moat is validated. Desktop users may use supported MV3 blockers after EP-01-T12; mobile does not depend on extension availability for its eventual privacy model.

---

## 19. Support & Feedback (V0)

**In-app feedback:** A "Send Feedback" item in the Help menu. Opens a simple form (rendered in a Drift panel, not a web page). User types feedback; on submit, writes a local markdown file to `~/Library/Application Support/Drift/feedback/` and optionally opens the user's default mail client with the file attached. No server required.

**Bug reporting:** Settings → Advanced → Export Diagnostics. Creates a zip of local crash logs, the last 100 recognition diagnostics without page content, and platform/version/memory metadata. User submits it manually via email or GitHub Issues.

**Documentation:** A static docs site (generated from markdown, hosted on GitHub Pages). Linked from Help menu. No in-app help system in V0.

**Support channel:** GitHub Issues for V0 beta. Email support evaluated for public launch.

---

## 20. Open Decisions (updated)

| Decision | Needed by | Owner |
|---|---|---|
| ~~Serif typeface~~ | ~~SP-09~~ | **Locked: Lora** (2026-09-16) |
| ~~Action execution targets~~ | ~~SP-12~~ | **Locked: Amazon, Best Buy, Flipkart** (2026-09-16) |
| VPN vendor shortlist (Mullvad vs Guardian) | Scale planning | Anand |
| Analytics — self-hosted Matomo or explicit deferral? | Distribution planning | Anand |
| Beta programme: invite-only vs open waitlist | Distribution planning | Anand |
| Crash symbol infrastructure per released desktop platform | Distribution planning | Anand |

---

## 21. Cross-Platform Architecture Decision

Locked 2026-09-18. Detailed specification: `docs/08_cross_platform_architecture.md`.

- **Product targets:** macOS, Windows, Android, and iPhone.
- **Shared center:** C++20 owns platform-neutral browser state, the object model, comparison/action logic, storage contracts, and future sync contracts.
- **Platform edges:** AppKit/Objective-C++ on macOS, Win32/WinUI on Windows, Kotlin on Android, and Swift on iPhone own lifecycle, accessibility, secure storage, permissions, packaging, and engine hosting.
- **Engine adapters:** CEF on macOS and Windows; an Android WebView proof with GeckoView as fallback; WKWebView as the iPhone baseline.
- **UI sharing:** Lit is for Drift-owned product surfaces and internal pages. It is not the browser engine or a replacement for native shells.
- **Boundary:** platform UI issues versioned commands; shared core updates state and calls `BrowserEngine`; adapters return normalized events/results. No AppKit, CEF, JNI, Swift, Kotlin, or WebKit types enter the core.
- **Multi-device sync:** a later, separately enabled, end-to-end encrypted service for eligible categories only. Raw browsing history, page content, downloads, keystrokes/forms, and credentials do not sync; consent is per category and separate from cloud inference or analytics.
- **Migration:** preserve the working macOS browser, characterize it, and extract thin vertical slices. Do not restart the application.

**Reasoning:** Browser engines, process models, extension support, store policy, and system integration differ materially across desktop and mobile. A fully cross-platform UI framework cannot remove that native work. Sharing Drift's differentiating product behavior while isolating engines gives useful reuse without forcing the weakest common denominator.

---

## 22. Technical Architecture Principles

Full architecture: `docs/08_cross_platform_architecture.md`; product-system concepts: `docs/04_architecture.md`.

1. **C++20 shared core, not C++ everywhere.** C++ is selected for continuity, native interoperability, and control. It does not guarantee speed or reliability; ownership rules, tests, sanitizers, measurements, and narrow boundaries do.
2. **CEF is a desktop engine adapter, not the architecture.** CEF serves macOS and Windows. Shared logic never includes CEF types.
3. **Native shells are intentional.** Each operating system keeps responsibility for lifecycle, accessibility, secure storage, permissions, and distribution.
4. **Stable cross-language boundary.** Never expose C++ standard-library types across DLL/mobile boundaries. Use a narrow C ABI or a versioned serialized command/event protocol.
5. **Portable local inference.** No Python runtime ships. Core ML, ONNX Runtime, and platform runtimes implement one normalized contract.
6. **Shared core owns Drift state.** Engine adapters report facts; shells render observable state and collect user intent.
7. **Lit has a bounded role.** Use it for Drift-owned product surfaces and internal pages, not browser engine hosting or universal native abstraction.
8. **Persistence is replaceable.** Preserve current JSON behavior first; introduce SQLite/encryption behind repository contracts only after compatibility tests.
9. **Extensions are desktop-scoped initially.** macOS and Windows may target MV3; Android and iPhone do not inherit that promise.
10. **Zero telemetry without opt-in.** Browsing data and page content do not leave the device by default.
11. **Binary dependencies are fetched, not committed.** CEF distributions and generated outputs stay ignored and checksum-verified. Git LFS is not used for reproducible SDKs or app bundles.
12. **Vertical slices prove architecture.** A new folder or interface is not progress until an existing behavior runs through it and tests protect the result.
