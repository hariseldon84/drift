# Drift

A local-first, multi-device browser that understands the things inside pages.

Drift targets macOS, Windows, Android, and iPhone. It extracts typed objects — starting with products — and makes them useful across tabs and devices through Compare Mode, a connected object graph, and human-gated actions.

## Current status

The runnable implementation today is a macOS CEF browser with tabs, navigation, bookmarks, history, downloads, native chrome, keyboard shortcuts, and JSON persistence. That working browser is preserved on branch `V018Sep2026`.

`main` now contains the first tested, platform-neutral C++20 browser-state contract. Before new intelligence features resume, the project will move existing macOS state through that core, isolate engines behind adapters, and prove Windows, Android, and iPhone vertical slices.

| Phase | State |
|---|---|
| macOS browser baseline (SP-01–SP-03) | Complete |
| Architecture and platform contract (SP-04) | Complete |
| Shared contracts and characterization (SP-05) | Complete |
| Shared state extraction (SP-06) | Next |
| Engine and persistence boundaries (SP-07) | Planned |
| Windows proof (SP-08) | Planned |
| Android and iPhone proofs (SP-09) | Planned |
| Product intelligence and Compare Mode | Queued after the foundation gate |

See `tasks.md` for the task-level plan and `docs/08_cross_platform_architecture.md` for the locked architecture.

## Architecture

Drift is not one identical executable and not four independent codebases:

- **Shared C++20 core:** browser state, typed objects, Compare/Action logic, storage contracts, and future sync contracts.
- **Native shells:** Objective-C++/AppKit on macOS, C++/Win32 or WinUI on Windows, Kotlin on Android, and Swift on iPhone.
- **Engine adapters:** CEF on macOS and Windows; Android WebView proof with GeckoView as fallback; WKWebView on iPhone.
- **Shared Lit surfaces:** Drift-owned object, Compare, widget, and internal-page UI where web components are a good fit.

C++ is selected for continuity, native interoperability, and performance control. It does not guarantee stability: Drift requires RAII, explicit ownership/threading, narrow ABI boundaries, tests, sanitizers, and measurements.

## What makes Drift different

| Capability | Description |
|---|---|
| Page understanding | Deterministic extraction, heuristics, and local inference produce typed, sourced fields |
| Compare Mode | Hold products across tabs, align fields, show meaningful differences, and take a gated action |
| Object graph | Products and later object types compose beyond a single page or tab |
| Connected surfaces | Tasks, Notes, Timer, history, and downloads can use the same product state |
| Local first | Browsing data and page understanding remain on-device by default |

## Build the current macOS browser

Requirements: macOS 13+, Xcode 15+, CMake 3.25+, and Ninja.

```bash
./scripts/fetch-cef.sh
cmake -S . -B build -G Ninja -DPROJECT_ARCH="$(uname -m)" -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
open build/src/browser/Drift.app
```

CEF and generated build products are fetched/generated locally and ignored by Git. Do not add them to Git LFS. See `commands.md` for the full command reference.

## Design

Drift uses a warm cream base (`#FAF7F2`), amber (`#D4820A`) only for intelligence, and pine green (`#2D5016`) for structure and confirmation. Inter is the UI typeface; Lora is reserved for intelligence moments. The source of truth is `docs/design_specification.md`.
