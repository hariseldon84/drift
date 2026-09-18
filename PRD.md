# Drift — Product Requirements Document

> Authoritative source for what Drift is, why decisions were made, and what gets built.
> Last updated: 2026-09-15. Maintained by Anand.
> Design source of truth: docs/design_specification.md
> Task tracking: tasks.md (uses nomenclature defined in §6 of this document)

---

## 1. Vision

Drift is a Chromium-based desktop browser that understands what is inside pages, not just what pages say. It recognises typed objects — products, prices, articles, people, job listings — and lets the user work across tabs as if the web were one structured database.

**One-line positioning:** "Drift understands the things inside pages."

Not "AI browser." Not "smart browser." The intelligence is the object model — every other feature flows from it.

**Emotional register** (from designinspirationstory.md): opening Drift feels like walking into a warm mountain cafe in Shimla at 8am. The web is the beautiful, chaotic road outside. Drift is where you arrive — calm, organised, unhurried. Intelligence is light coming through the window, not a dashboard demanding attention.

---

## 2. Target User

**Primary:** Knowledge workers who spend 4+ hours a day in a browser — researchers, shoppers making considered purchases, analysts, students. They feel overwhelmed by tab sprawl and want the web to work with them, not against them.

**Secondary:** Privacy-conscious users who want local-first intelligence with no cloud dependency.

**Not in V0:** Enterprise IT buyers, developers (no DevTools focus), casual mobile-first users.

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

## 5. Gate Decisions (all locked 2026-09-15)

These were blocked in docs/07_PRD_GATE.md. All resolved here with reasoning. Do not re-litigate without a documented reason to change.

### GD-01 — Launch OS: Mac first, Windows in V1
**Decision:** macOS only for V0.
**Why:** CEF is most mature on Mac. Target users skew heavily Mac. Polished experience on one platform beats mediocre on two. Windows distribution, installer, and auto-update complexity is deferred.
**Revisit when:** V0 has >500 DAU and Windows demand is evidenced by user requests.

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
**V0:** 100% local. No cloud calls. Object recognition, page understanding, and comparison logic run entirely on device via Core ML.
**V1:** Cloud fallback available, off by default. Only with user permission. Only structured summaries sent — never raw page content, never browsing history.
**Why:** Local-first is a core principle and competitive differentiator. Cloud-first requires server infrastructure day one and compromises the privacy promise.

### GD-07 — Performance budgets (measured on M1 MacBook Pro, 8GB RAM)

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

### V0 — MLP (Minimum Lovable Product)
**Platform:** Mac only.
**Definition of done:** A user can install Drift, use it as their daily browser, and complete a product comparison across two sites without friction.

**V0 Epics:**
- EP-01: Chromium Shell & Browser Baseline
- EP-02: Page Understanding Pipeline
- EP-03: Product Object Recognition
- EP-04: Compare Mode
- EP-05: Connected Widget Surface
- EP-06: Distribution & Installation (Mac)

**Sprint plan (2-week sprints, ~7 months total):**

| Sprint | Focus | Epics |
|---|---|---|
| SP-01 | Chromium shell, window management, basic navigation | EP-01 |
| SP-02 | Tab management, address bar, keyboard shortcuts | EP-01 |
| SP-03 | Bookmarks, history, downloads | EP-01 |
| SP-04 | Chrome extension compatibility layer | EP-01 |
| SP-05 | Password manager, settings infrastructure | EP-01 |
| SP-06 | DOM parsing pipeline, deterministic extraction | EP-02 |
| SP-07 | Heuristic object detection, local ML integration | EP-02 |
| SP-08 | Product schema, confidence scoring, Core ML | EP-02 + EP-03 |
| SP-09 | Recognition UI — amber-gold indicator, object card, object panel | EP-03 |
| SP-10 | Compare Tray, cross-tab state, selection flow | EP-04 |
| SP-11 | Compare View — field matching, winner highlighting | EP-04 |
| SP-12 | Action engine — suggestion card, confirmation gate, execution | EP-04 |
| SP-13 | Widget surface — Tasks, Notes, Timer + object graph connections | EP-05 |
| SP-14 | Performance tuning, polish, Mac installer, beta distribution | EP-06 |

### V1 — Scale
**Platform:** Mac + Windows.
**Goal:** Object model generalises. Pro tier launches. Research Mode ships.
**V1 Epics:** EP-07 through EP-13 (see §9).

### V2+ — Platform
**Goal:** Ecosystem. Developer platform. Autonomous actions.
**V2 Epics:** EP-14 through EP-16 (see §9).

---

## 8. V0 Epic Specifications

### EP-01 — Chromium Shell & Browser Baseline

**Goal:** A working, stable browser users would not be embarrassed to use as their daily driver.
**Sprint:** SP-01 through SP-05.
**Out of V0:** Custom tab groups, vertical tabs, sidebar, sync.

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
| EP-01-T09 | Bookmarks — add, folder, toolbar, import from Chrome/Safari |
| EP-01-T10 | History — browsing history, search, clear |
| EP-01-T11 | Downloads — download bar, progress, open/show in Finder |
| EP-01-T12 | Chrome extension compatibility — CRX load, chrome.* API surface |
| EP-01-T13 | Password manager — save/fill credentials, local keychain integration |
| EP-01-T14 | Settings page — general, search engine, privacy, appearance |

### EP-02 — Page Understanding Pipeline

**Goal:** Drift reads a page and extracts structured meaning, entirely on-device, within 500ms.
**Architecture:** Four-stage pipeline: (1) Deterministic extraction (JSON-LD, schema.org, OpenGraph), (2) Heuristic rules (CSS selectors, DOM patterns), (3) Local ML classifier, (4) Core ML local model. Cloud not in V0 pipeline.
**Sprint:** SP-06 through SP-07.

**Tasks:**
| Code | Task |
|---|---|
| EP-02-T01 | DOM extraction module — JSON-LD, schema.org, OpenGraph parser |
| EP-02-T02 | Heuristic rule engine — CSS selector patterns for price, title, rating, availability |
| EP-02-T03 | Local ML classifier — identify product page vs non-product page |
| EP-02-T04 | Confidence scoring — combine all pipeline stage signals into 0–1 score |
| EP-02-T05 | Core ML model integration — local inference on macOS, compiled model |
| EP-02-T06 | Performance harness — full pipeline < 500ms on M1, 8GB RAM (GD-07) |
| EP-02-T07 | Graceful degradation — if no stage produces confidence > 0.5, stay silent |

### EP-03 — Product Object Recognition

**Goal:** When the pipeline identifies a product with sufficient confidence, Drift materialises a typed Product object visible to the user.
**Confidence threshold:** 0.75 to show recognition indicator. 0.5–0.75 logs silently, no UI.
**Sprint:** SP-08 through SP-09.

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
**Sprint:** SP-10 through SP-12.

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
**Sprint:** SP-13.

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

### EP-06 — Distribution & Installation (Mac)

**Goal:** User can download, install, and update Drift on macOS without friction.
**Design reference:** Screen 11 (Onboarding), design_inspirations/Drift - Compare Mode Mockups.html.
**Sprint:** SP-14.

**Tasks:**
| Code | Task |
|---|---|
| EP-06-T01 | macOS app bundle — signed, notarised, Gatekeeper compliant |
| EP-06-T02 | .dmg installer — drag-to-Applications, auto-launch on first install |
| EP-06-T03 | Auto-update — Sparkle framework integration, silent background updates |
| EP-06-T04 | Set as default browser — prompt on first launch, system API |
| EP-06-T05 | First-run onboarding — Screen 11 design, import from Chrome/Safari |
| EP-06-T06 | Crash reporting — local crash logs, no cloud telemetry in V0 |
| EP-06-T07 | Basic analytics — opt-in only, local aggregate counts (DAU, compare sessions) |
| EP-06-T08 | Reproducible source checkout — generated build output and CEF binaries excluded; pinned CEF bootstrap for each supported platform |

---

## 9. V1 Epics — Overview

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

### EP-12 — Windows Distribution
Windows installer (MSIX or WinSparkle), CEF parity on Windows, performance validation on target Windows hardware.

### EP-13 — Pro Tier & Account System
Optional Drift account (Sign in with Google or Apple). Unlocks cloud sync, VPN, Research Mode advanced features, Widget library. $9.99/month or $99/year.

---

## 10. V2+ Epics — Placeholder

### EP-14 — App Store / Third-Party Widget Ecosystem
Developer SDK, widget review process, trust and safety. A company within a company — do not open until V1 is proven.

### EP-15 — Auto Pilot
Autonomous multi-step action sequences. User defines a workflow; Drift executes with confirmation gates at each consequential step.

### EP-16 — Expanded Object Types (V2)
Chart, Video, Company, Person as first-class typed objects. Cross-type composition (e.g., compare two companies using their person graphs).

---

## 11. Open Source Stack Decisions

Locked 2026-09-16. These replace "build from scratch" wherever a proven open source tool exists.

| Component | Tool | Rationale |
|---|---|---|
| Chromium embedding | **CEF (Chromium Embedded Framework)** | Only production-grade option for embedding Chromium on macOS. C++. Used by VS Code, Figma, Discord. |
| Structured data extraction (EP-02-T01) | **extruct pattern** — JSON-LD, schema.org, OpenGraph parsers | No single open source product extractor exists. Deterministic stage uses standard markup parsers. Heuristic + ML stages are custom (that is the moat). |
| Local ML inference (EP-02-T05, EP-03) | **Core ML** (primary) + **ONNX Runtime** (fallback) | Core ML gives hardware acceleration on Apple Silicon at zero cost. ONNX Runtime enables cross-platform portability for V1 Windows. Use one runtime per inference job to avoid memory thrashing. |
| Local storage — object graph + widgets | **SQLite + sqlcipher** | SQLite is built into macOS. sqlcipher adds encryption. OS Keychain manages encryption keys — do not roll custom key management. |
| macOS auto-update (EP-06-T03) | **Sparkle** | macOS gold standard. Used by Dropbox, Slack. Handles signing, delta updates, rollback. |
| Crash reporting (EP-06-T06) | **Breakpad** (Google) | Already inside Chromium. Zero additional dependency. Requires symbol management — plan symbol server before beta. |
| Browser testing | **Playwright** | Best debugging tooling, multi-browser, Microsoft-maintained. Use jsdom for fast headless unit tests of the page understanding pipeline. |
| Analytics — opt-in only (EP-06-T07) | **Matomo** (self-hosted) | Only truly open source privacy-respecting analytics. Requires a server — defer to SP-14. Ship V0 without analytics if server is not ready; add Matomo in the first post-launch patch. |

**What has NO open source substitute:**
- Product extraction heuristics (EP-02-T02) — custom CSS selector rules per retailer
- Product ML classifier (EP-02-T03) — custom trained model, fine-tuned on product pages
- Compare Mode UI and object graph (EP-04) — core product, built from scratch
- Action execution DOM layer (EP-04-T09) — retailer-specific, no general library

---

## 12. System Requirements

Locked 2026-09-16.

**Minimum supported:** macOS 13 Ventura (released 2022).
**Why:** Core ML features required by EP-02-T05 use APIs introduced in macOS 13. Ventura also introduced improved process sandbox APIs used by CEF.
**Recommended:** macOS 14 Sonoma or later (better Core ML performance, Neural Engine improvements).
**Apple Silicon:** Primary development and optimisation target. Intel Mac support maintained but not the performance benchmark.

| Requirement | Minimum | Recommended |
|---|---|---|
| macOS | 13 Ventura | 14 Sonoma+ |
| RAM | 8GB | 16GB |
| Disk space | 500MB | 1GB |
| CPU | Intel Core i5 / M1 | Apple M1+ |
| Internet | Required for browsing | — |

Performance budgets in GD-07 are measured on M1, 8GB RAM (the minimum recommended configuration).

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
- Screen reader: VoiceOver (macOS) compatibility for chrome elements, object panel, and widget surface.
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

**V0 targets Manifest V3 (MV3) only.** Chrome deprecated MV2 in 2024; Drift does not re-introduce it.

**Must work in V0 (tested and verified before beta):**
- uBlock Origin (MV3 version) — most-used extension globally
- 1Password / Bitwarden — password managers (complement Drift's own)
- Grammarly — content script on all pages
- Google Translate — page-level translation

**Extension interaction with Drift's object recognition:**
- Content scripts that modify the DOM run before Drift's page understanding pipeline. This is acceptable — Drift reads the final rendered DOM, not the raw HTML.
- Extensions that aggressively remove DOM nodes (some ad blockers) may reduce extraction confidence. This is logged but not surfaced to the user.
- Drift does not expose its object graph to extensions in V0. No extension API for the object model.

**Explicitly out of scope for V0:**
- MV2 extensions
- Chrome DevTools extensions
- Extensions requiring `chrome.debugger` API

---

## 17. Privacy Model Detail

**What never leaves the device (ever, regardless of settings):**
- Browsing history
- Page content (HTML, text, images)
- Extracted object data
- Widget data (tasks, notes)
- Keystrokes or form data

**What leaves the device only with explicit opt-in:**
- Anonymous aggregate usage counts (DAU, compare sessions per day) — no user identity, no session linking
- Crash reports (local by default; user can choose to submit via a "Send crash report" prompt post-crash)

**What opt-in analytics collects (Matomo, self-hosted):**
- Count of browser sessions per day (no user ID)
- Count of compare sessions per day
- Object recognition success/failure rate (aggregate)
- Does NOT collect: URLs visited, page content, search queries, object data

**With cloud fallback enabled (V1, off by default):**
- Only structured summaries are sent — typed object fields, not page content
- No browsing history, no URLs, no session context
- Request/response is encrypted in transit
- Drift's cloud receives: `{ type: "Product", fields: [...] }` — never `{ url: "...", html: "..." }`

---

## 18. Browser Security Model

**Process isolation:** CEF's multi-process architecture is preserved. Each tab runs in a separate renderer process. Drift's intelligence layer runs in the browser process, not the renderer — extracted object data crosses the process boundary as structured data, never as executable code.

**Content Security Policy:** Drift respects page-level CSP headers. Drift's own injected UI elements (object card, compare tray) are injected via CEF's native UI overlay mechanism, not via script injection into the page — they are not subject to the page's CSP.

**Object data trust model:** Data extracted from a page is treated as untrusted user input. All extracted strings are sanitised before display in Drift's UI. An attacker cannot execute code by crafting a product page with malicious markup in a product name field.

**Action engine security:** Actions (add to cart) interact with the DOM via CEF's browser process, not via injected scripts. The action confirmation gate is rendered by Drift's native UI, not by the page — it cannot be spoofed by a malicious page.

**V0 stance on tracking:** No built-in tracker blocking in V0 (uBlock Origin covers this via extensions). V1 evaluates native tracker blocking. This is a deliberate deferral — tracker blocking is table-stakes but not Drift's moat.

---

## 19. Support & Feedback (V0)

**In-app feedback:** A "Send Feedback" item in the Help menu. Opens a simple form (rendered in a Drift panel, not a web page). User types feedback; on submit, writes a local markdown file to `~/Library/Application Support/Drift/feedback/` and optionally opens the user's default mail client with the file attached. No server required.

**Bug reporting:** Settings → Advanced → Export Diagnostics. Creates a zip of: local crash logs, recognition pipeline logs (last 100 entries, no page content), and system info (macOS version, RAM). User submits manually via email or GitHub Issues.

**Documentation:** A static docs site (generated from markdown, hosted on GitHub Pages). Linked from Help menu. No in-app help system in V0.

**Support channel:** GitHub Issues for V0 beta. Email support evaluated for public launch.

---

## 20. Open Decisions (updated)

| Decision | Needed by | Owner |
|---|---|---|
| ~~Serif typeface~~ | ~~SP-09~~ | **Locked: Lora** (2026-09-16) |
| ~~Action execution targets~~ | ~~SP-12~~ | **Locked: Amazon, Best Buy, Flipkart** (2026-09-16) |
| VPN vendor shortlist (Mullvad vs Guardian) | V1 planning | Anand |
| Analytics vendor — Matomo self-hosted confirmed? Or defer entirely? | SP-14 | Anand |
| Beta programme: invite-only vs open waitlist | SP-14 | Anand |
| Symbol server for Breakpad: self-hosted or defer to post-launch? | SP-14 | Anand |

---

## 22. Technical Architecture Principles

Full architecture: docs/04_architecture.md. PRD-level locked decisions:

1. **CEF, not Electron.** CEF gives browser-grade rendering performance. Electron is not appropriate for a daily browser.
2. **Core ML for local inference on Mac.** No Python runtime shipped. Models compiled to Core ML format.
3. **Object graph in-process.** Lives in the browser process. V1 may add SQLite persistence layer.
4. **Zero telemetry without opt-in.** No browsing data sent anywhere in V0.
5. **Chrome extension API compatibility.** CRX format, chrome.* API surface. No custom extension format in V0.
6. **Binary dependencies are fetched, not committed.** CEF is required for the desktop browser, but the pinned platform distribution and every generated build artifact live in ignored local directories. A bootstrap script downloads and checksum-verifies CEF after clone. Git LFS is reserved for authored assets that genuinely need versioning, not reproducible SDKs or app bundles. This keeps clones small, avoids GitHub's binary-size limits, and lets macOS and future Windows builds fetch their own platform package from the same version pin.

---

## 12. Open Decisions

Not blocking build-start. Must be resolved before the relevant sprint.

| Decision | Needed by | Owner |
|---|---|---|
| ~~Serif typeface~~ | ~~SP-09~~ | **Locked: Lora** (2026-09-16) |
| ~~Action execution targets~~ | ~~SP-12~~ | **Locked: Amazon, Best Buy, Flipkart** (2026-09-16) |
| VPN vendor shortlist: Mullvad vs Guardian vs other | EP-09 planning (V1) | Anand |
| Analytics vendor (opt-in): self-hosted vs PostHog vs Plausible | SP-14 (EP-06-T07) | Anand |
| Beta programme: invite-only vs open waitlist | SP-14 | Anand |
