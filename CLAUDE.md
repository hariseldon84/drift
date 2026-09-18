# Drift — Architecture & Engineering Reference

> Read this before touching any code. Read PRD.md for the *why* behind decisions.
> This file covers the *how*: stack, structure, constraints, and locked decisions.
> Last updated: 2026-09-16

---

## What Drift Is (one paragraph for engineers)

Drift is a Chromium-based desktop browser that runs a four-stage page understanding pipeline after every page load. The pipeline extracts typed objects (V0: Products only) from pages, materialises them into a cross-tab object graph, and exposes them through Compare Mode and a connected widget surface. All inference runs locally on-device. No browsing data leaves the machine. The browser must feel as fast as Chrome — intelligence is additive, never a tax on performance.

---

## Tech Stack

| Layer | Technology | Notes |
|---|---|---|
| Browser engine | **CEF (Chromium Embedded Framework)** | Latest stable. C++. Prebuilt binaries. |
| Browser process / renderer | **C++17** | CEF's multi-process model. Renderer is sandboxed. |
| macOS integration | **Swift 5.9+** | Sparkle, Keychain, system APIs, window chrome native layer |
| UI panels | **TypeScript + Lit** | Web components rendered in CEF overlay windows |
| Page understanding pipeline | **C++ (deterministic) + Swift (Core ML)** | EP-02 pipeline |
| Local ML inference | **Core ML** (primary) + **ONNX Runtime** (fallback) | Core ML on Apple Silicon. ONNX for V1 Windows portability. |
| Local storage | **SQLite 3 + sqlcipher** | Encrypted. Keys in macOS Keychain. |
| Auto-update | **Sparkle 2.x** | macOS only. Signed feeds required. |
| Crash reporting | **Breakpad** | Bundled in CEF. Local symbol server required before beta. |
| Build system | **CMake 3.25+** | C++ deps via FetchContent. JS via npm. |
| E2E testing | **Playwright** | Controls the built Drift/CEF window |
| Pipeline unit testing | **jsdom + Jest** | Fast headless, no browser process needed |
| Analytics (opt-in, post-launch) | **Matomo** (self-hosted) | Deferred until server available |
| Fonts | **Inter + Lora** | Bundled locally — no CDN at runtime |
| Icons | **Phosphor Icons** | Inline SVG, currentColor |
| Design tokens | `docs/design_specification.md` | Source of truth for all hex values, spacing, radius |

---

## Repository Structure

```
drift/
├── src/
│   ├── browser/              # CEF browser process (C++) — window, tabs, navigation
│   ├── renderer/             # CEF renderer process (C++) — page interaction layer
│   ├── shell/                # macOS native shell (Swift) — Sparkle, Keychain, window chrome
│   ├── intelligence/
│   │   ├── extraction/       # EP-02-T01: JSON-LD, schema.org, OpenGraph parsers
│   │   ├── heuristics/       # EP-02-T02: CSS selector rules per retailer
│   │   ├── classifier/       # EP-02-T03: ML product page classifier
│   │   ├── inference/        # EP-02-T05: Core ML + ONNX Runtime integration
│   │   └── pipeline.cpp      # EP-02: Four-stage orchestrator, confidence scoring
│   ├── graph/                # EP-03: Product object schema, materialisation, persistence
│   ├── compare/              # EP-04: Compare Mode — tray, view, field matching, actions
│   ├── widgets/              # EP-05: Widget surface — Tasks, Notes, Timer
│   ├── action/               # EP-04-T07–T09: Action engine, confirmation gate, DOM execution
│   │   └── retailers/        # One file per retailer (amazon.cpp, bestbuy.cpp, flipkart.cpp)
│   └── platform/             # macOS utils — Keychain, default browser API
├── ui/                       # TypeScript + Lit web components for all Drift UI panels
│   ├── object-card/          # Hover card (EP-03-T04)
│   ├── object-panel/         # Full right-side panel (EP-03-T05)
│   ├── compare/              # Compare tray + compare view (EP-04)
│   ├── widgets/              # Widget surface components (EP-05)
│   └── design-tokens.css     # CSS custom properties from design_specification.md
├── models/                   # Compiled Core ML (.mlpackage) and ONNX (.onnx) model files
│   ├── product-classifier/   # EP-02-T03: Is this a product page?
│   └── field-extractor/      # EP-02-T05: Extract structured fields
├── resources/
│   ├── fonts/                # Inter + Lora, bundled locally
│   └── icons/                # Phosphor SVGs
├── tests/
│   ├── e2e/                  # Playwright browser E2E tests
│   ├── pipeline/             # jsdom + Jest unit tests for extraction pipeline
│   └── fixtures/             # Saved HTML snapshots — Amazon, Best Buy, Flipkart product pages
├── docs/                     # Product documentation
├── design_inspirations/      # 34 screens across 3 mockup sets
├── PRD.md                    # What and why — 22 sections
├── CLAUDE.md                 # This file — how and where
├── tasks.md                  # EP-XX-TXX task tracking, sprint assignments
├── status_quo.md             # Cold-start anchor — read first every session
├── groundrules.md            # Session rules — always followed
├── commands.md               # Every runnable command in full (created SP-01)
└── bugs3.md                  # Bug log (created on first bug observed)
```

---

## Architecture — How the Layers Connect

```
┌─────────────────────────────────────────────────────────┐
│  macOS Window (Swift shell)                             │
│  ┌───────────────────────────────────────────────────┐  │
│  │  Browser Chrome (CEF browser process, C++)        │  │
│  │  Tab bar · Address bar · Window controls          │  │
│  ├───────────────────────────────────────────────────┤  │
│  │  Viewport (CEF renderer process, sandboxed)       │  │
│  │  ← Web page renders here                         │  │
│  ├───────────────────────────────────────────────────┤  │
│  │  Drift UI Overlay (TypeScript + Lit, CEF offscreen)│ │
│  │  Object card · Object panel · Compare tray       │  │
│  │  Compare view · Widget surface · Action gates    │  │
│  └───────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────┘
         │ page-load event
         ▼
┌─────────────────────────────────────────────────────────┐
│  Page Understanding Pipeline  (browser process, C++)    │
│                                                         │
│  Stage 1 — Deterministic extraction                     │
│    JSON-LD → schema.org → OpenGraph → microformats     │
│                                                         │
│  Stage 2 — Heuristic rules                              │
│    CSS selectors per retailer domain                    │
│                                                         │
│  Stage 3 — Local ML classifier  (Swift / Core ML)       │
│    Is this a product page? → confidence 0–1            │
│                                                         │
│  Stage 4 — Local model  (Swift / Core ML or ONNX)       │
│    Extract structured fields → Product object          │
│                                                         │
│  Confidence scorer — combines all stages → 0–1         │
│  < 0.5 → silent  |  0.5–0.75 → log  |  ≥ 0.75 → UI   │
└────────────────────────┬────────────────────────────────┘
                         │ Product object (confidence ≥ 0.75)
                         ▼
┌─────────────────────────────────────────────────────────┐
│  Object Graph  (browser process, SQLite in-memory V0)  │
│  Typed Product objects · Cross-tab state · Session     │
└────────────────────────┬────────────────────────────────┘
                         │
           ┌─────────────┼─────────────┐
           ▼             ▼             ▼
     Compare Mode   Widget Surface  Object Panel
     (EP-04)        (EP-05)         (EP-03-T05)
```

**Key invariant:** The renderer process never touches the object graph directly. Data crosses the process boundary as serialised structs — never as executable code or raw DOM references. This is the security boundary (PRD §18).

---

## Locked Technical Decisions

Do not revisit without a written reason added to PRD.md.

| Decision | What is locked |
|---|---|
| CEF, not Electron | Electron overhead makes a daily browser feel slow. CEF gives browser-grade performance. |
| MV3 extensions only | Chrome deprecated MV2 in 2024. Drift does not re-introduce it. |
| Core ML primary, ONNX fallback | Core ML gives hardware acceleration on Apple Silicon at zero cost. ONNX is the V1 Windows path. |
| No Python runtime shipped | Models are pre-compiled to `.mlpackage` or `.onnx`. No interpreter in the installed app. |
| Object graph in browser process | V0 simplicity. V1 may extract to SQLite persistence layer for cross-session retention. |
| UI panels as web components (Lit) | Design system is web-based. Lit is the lightest web component library — no virtual DOM. |
| Zero cloud calls in V0 | 100% local pipeline. Cloud fallback is V1, opt-in, sends structured summaries only. |
| Lora for intelligence moments | Locked 2026-09-16. See docs/design_specification.md. Do not substitute. |
| Inter for all UI chrome | Bundled locally. Not loaded from CDN at runtime. |
| SQLite + sqlcipher | OS Keychain holds the encryption key. Do not roll custom key management. |
| One file per retailer in action engine | `src/action/retailers/amazon.cpp`, `bestbuy.cpp`, `flipkart.cpp`. No generic DOM interaction layer — retailer structures are too divergent. |

---

## Design System Integration

All colours, spacing, radius, and shadow values come from `docs/design_specification.md`. They are implemented as CSS custom properties in `ui/design-tokens.css`. Always use the variables — never hardcode hex values in component CSS.

**Critical palette rules for engineers:**
- `--drift-amber: #D4820A` — used **only** when Drift's intelligence activates. Never decorative.
- `--drift-green: #2D5016` — structure, navigation, trust signals, confirmations.
- `--drift-base: #FAF7F2` — browser frame. Never pure white (`#ffffff`).
- Never use: corporate blue, neon, purple, cold grey, pure black, pure white.

Intelligence moments use **Lora** (`--font-intelligence`). Everything else uses **Inter** (`--font-ui`). Enforced via CSS variables — do not override per-component.

---

## Product Object Schema (V0)

The single typed object in V0. Full spec in PRD.md §5 (GD-02). Implementation in `src/graph/product.h`.

```cpp
struct Product {
  std::string name;
  std::string brand;
  struct Price {
    double      amount;
    std::string currency;   // ISO 4217, e.g. "USD", "INR"
  } price;
  struct Rating {
    double score;           // 0.0–5.0
    int    count;
  } rating;
  enum class Availability {
    IN_STOCK, OUT_OF_STOCK, LIMITED
  } availability;
  std::vector<std::string> images;      // URLs, max 5
  std::vector<std::string> key_specs;   // max 5
  std::string source_url;
  std::string source_domain;
  std::string extracted_at;             // ISO 8601
};
```

V1 adds Article, Person, JobListing, Chart. Do not generalise the schema in V0.

---

## Performance Budgets (GD-07)

Hard constraints measured on M1 MacBook Pro, 8GB RAM. Not aspirations.

| Metric | Budget |
|---|---|
| Browser cold start | < 4 seconds |
| Browser warm start | < 2 seconds |
| Page understanding pipeline | < 500ms post page-load |
| Compare view render | < 1 second |
| Widget surface load | < 300ms |
| Memory overhead vs Chrome | < 25% additional |
| Page load parity vs Chrome | within 5% |

Page load parity is tracked from SP-01 — not deferred to SP-14. Every PR touching the browser process includes a before/after measurement.

---

## System Requirements (PRD §12)

| | Minimum | Recommended |
|---|---|---|
| macOS | 13 Ventura | 14 Sonoma+ |
| RAM | 8GB | 16GB |
| CPU | Intel Core i5 / M1 | Apple M1+ |
| Disk | 500MB | 1GB |

Performance budgets are measured on the minimum recommended configuration (M1, 8GB).

---

## Testing Strategy

**Pipeline unit tests — run on every commit:**
```
npm test
```
- Tool: jsdom + Jest
- Location: `tests/pipeline/`
- Fixtures: `tests/fixtures/` — saved HTML from Amazon, Best Buy, Flipkart (standard product, variants, missing fields, non-product page)

**Browser E2E tests — run on every PR:**
```
npx playwright test
```
- Tool: Playwright controlling the built Drift app
- Covers: Compare Mode golden path, widget object graph connection, action confirmation gate
- Never test against live retailer sites in CI — use recorded fixtures only

**Performance regression — run on PRs touching browser process or pipeline:**
- Bundled perf harness outputs cold/warm start times and pipeline latency
- PR is blocked if any budget from GD-07 is exceeded

---

## Key File Index

| Question | File |
|---|---|
| What are we building and why? | `PRD.md` |
| Where is the project right now? | `status_quo.md` — read first every session |
| What task am I working on? | `tasks.md` — reference by `EP-XX-TXX` code |
| What does the UI look like? | `design_inspirations/` — 34 screens |
| What are the design tokens? | `docs/design_specification.md` |
| How do I run the build? | `commands.md` (created SP-01) |
| Where is bug X tracked? | `bugs3.md` (created on first bug) |
| What are the session rules? | `groundrules.md` |
| What did competitors do? | `docs/02_competitive_validation.md` |
| Full architecture detail? | `docs/04_architecture.md` |

---

## What NOT to Build in V0

Explicitly deferred. Do not add even if it seems small.

- Cloud sync of any kind
- User accounts or authentication
- VPN functionality
- Research Mode
- Per-tab incognito or guest mode
- Object types beyond Product
- 3+ product comparison
- Widget cloud sync
- Third-party extension API for the object graph
- App store or third-party widgets
- Auto Pilot / autonomous multi-step actions

Any PR introducing the above is out of V0 scope. Close with a reference to the relevant task code in `tasks.md`.
