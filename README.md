# Drift

A Chromium-based desktop browser for macOS that understands what you're looking at.

Drift runs a four-stage page understanding pipeline after every page load. It extracts typed objects — starting with products — builds a cross-tab object graph, and surfaces intelligence through Compare Mode and a connected widget layer. All inference runs locally. No browsing data leaves the machine.

---

## What makes it different

| Feature | Description |
|---|---|
| **Page Understanding** | DOM extraction → heuristic rules → local ML classifier → structured field extraction, all in < 500ms |
| **Compare Mode** | Hold products from any tab, compare side-by-side, see price/rating winner highlighted, take action (add to cart) |
| **Object Graph** | Products recognised across tabs persist for the session; Compare Tray survives tab switches |
| **Widget Surface** | Tasks, Notes, Timer — connected to the object graph. Replaces the new tab page |
| **100% local** | Core ML on Apple Silicon. No cloud calls in V0. No account required |

---

## Status

V0 browser baseline is complete. Intelligence pipeline and Compare Mode are next.

| Sprint | What | State |
|---|---|---|
| SP-01 – SP-02 | CEF shell, tab bar, address bar, nav, keyboard shortcuts, Drift theme, context menus | **Done** |
| SP-03 | Bookmarks, History, Downloads | **Done** |
| SP-04 | Chrome extension compatibility | Up next |
| SP-06 – SP-08 | Page understanding pipeline + Product schema | Planned |
| SP-09 – SP-12 | Object card, Compare Mode, Action Engine | Planned |
| SP-13 | Widget surface | Planned |
| SP-14 | Distribution, signing, auto-update, onboarding | Planned |

Full task list in `tasks.md`. Design in `design_inspirations/` (34 screens).

---

## Tech stack

| Layer | Technology |
|---|---|
| Browser engine | CEF (Chromium Embedded Framework), C++20 |
| macOS shell | Objective-C++ + AppKit (current V0 shell) |
| Product UI panels | TypeScript + Lit web components (planned shared layer) |
| Local ML | Core ML (Apple Silicon primary), ONNX Runtime (Windows fallback) |
| Storage | SQLite 3 + sqlcipher, key in macOS Keychain |
| Build | CMake 3.25+ |
| Fonts | Inter (UI), Lora (intelligence moments) — bundled, no CDN |

---

## Building

**Requirements:** macOS 13+, Xcode 15+, CMake 3.25+, CEF 152 binaries.

```bash
./scripts/fetch-cef.sh
cmake -S . -B build -G Ninja -DPROJECT_ARCH="$(uname -m)" -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
open build/src/browser/Drift.app
```

See `commands.md` for the full command reference.

---

## Design

Warm cream palette (`#FAF7F2` base). Amber (`#D4820A`) appears only when intelligence activates — never decorative. Pine green (`#2D5016`) for navigation and confirmations. Full token reference in `docs/design_specification.md`.

---

## Platform

macOS 13 Ventura minimum. Apple M1+ recommended. Windows is V1 (EP-12).
