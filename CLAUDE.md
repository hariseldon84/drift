# Drift — Architecture & Engineering Reference

> Read this before touching code. `PRD.md` explains why; this file defines how.
> Last updated: 2026-09-18. Cross-platform foundation decision locked.

---

## What Drift Is

Drift is one local-first browser product for macOS, Windows, Android, and iPhone. Its differentiator is a shared model of typed web objects, cross-tab/device composition, and human-gated actions. Rendering engines and native shells vary by platform; Drift-owned state and product behavior do not.

The current runnable product is the macOS CEF browser in `src/browser/`. It is the behavioral reference for a controlled extraction, not disposable prototype code.

## Locked Architecture

```text
native shell / Drift Lit surface
              |
              | versioned command
              v
shared C++20 state + product core
              |
              | BrowserEngine request
              v
platform engine adapter
              |
              | normalized event/result
              v
shared state -> shell/surface renders
```

Detailed specification: `docs/08_cross_platform_architecture.md`.

### Ownership by layer

| Layer | Owns | Must not own |
|---|---|---|
| Native shell | Window/app lifecycle, accessibility, menus, permissions, secure storage, packaging, engine host | Product state transitions or engine-neutral domain rules |
| Shared C++20 core | Browser state, Product/ObjectGraph/CompareSession/ActionPlan, repository and sync contracts | AppKit, WinUI, Swift, Kotlin, JNI, CEF, WebKit, or platform widgets |
| BrowserEngine adapter | Engine calls, callbacks, threading translation, normalized page/extraction results | Product policy or UI |
| Lit surfaces | Compare Mode, object UI, widgets, and internal Drift pages where sharing is valuable | Browser engine hosting or native application lifecycle |

### Platform matrix

| Platform | Shell | Initial engine | Binding to core |
|---|---|---|---|
| macOS | Objective-C++ / AppKit | CEF | direct C++/Objective-C++ bridge |
| Windows | C++ / Win32 or WinUI | CEF | direct C++ boundary |
| Android | Kotlin | Android WebView proof; GeckoView fallback | JNI over a narrow C ABI/protocol |
| iPhone | Swift | WKWebView | C ABI through an Objective-C++ bridge |

Chrome-extension compatibility is a desktop-only goal for the first release.

## Language and Boundary Rules

C++20 is used for the shared core because it matches the working CEF code, interoperates with all target platforms, and permits predictable control of latency and allocation. Being close to machine code does not make a program safe or stable by itself.

Required practices:

- RAII and value types; no raw owning pointers;
- explicit ownership and thread affinity in every interface;
- deterministic state reducers with unit tests;
- sanitizers in supported development/CI jobs;
- no C++ standard-library types across DLL, Swift, Kotlin, JNI, or C ABI boundaries;
- versioned commands/events and serialization;
- treat page and engine data as untrusted input.

Do not rewrite working code into another language merely for architectural symmetry. New isolated modules may use Rust or another language only after the stable boundary makes the trade-off measurable.

## Current Repository State

```text
drift/
  src/browser/
    main.mm                 CEF process setup and macOS entry point
    browser_window.mm       current UI, browser state, persistence, and CEF integration
    browser_window.h        bridge declarations
    drift_client.*          CEF client and callback routing
    drift_app.*             CEF application setup
  scripts/fetch-cef.sh      pinned/checksummed macOS CEF bootstrap
  resources/                bundled fonts and authored assets
  docs/                     product, design, and architecture documents
  tests/                    not present yet; introduced in EP-00-T02/T03
```

`browser_window.mm` currently combines native UI, tabs/navigation, bookmarks, history, downloads, JSON persistence, and direct CEF calls. Extraction must be behavior-preserving and proceed in tested vertical slices.

## Target Repository Shape

```text
apps/
  macos/
  windows/
  android/
  ios/
core/
  domain/
  browser-state/
  intelligence/
  storage/
  sync/
engine/
  api/
  cef/
  android/
  webkit/
ui/
  components/
  compare/
  objects/
  widgets/
  internal-pages/
tests/
  core/
  contract/
  platform/
```

Do not create this tree as empty architecture theater. Each move must put an existing behavior through the new boundary and include its test.

## First Shared Contracts

The initial `BrowserEngine` surface is driven by the existing browser:

- commands: create/select/close/reorder/restore tab; navigate; back/forward; reload/stop; request metadata/extraction; start/cancel/inspect download;
- events: tab lifecycle; URL/title/favicon; loading and navigation capability; load failure; renderer termination; download progress/completion; extraction result; typed error;
- values: stable platform-neutral IDs, URLs, timestamps, progress, results, and error codes.

The shared core is authoritative for Drift state. Engines report facts; shells render state and collect intent.

## Persistence

Preserve the current JSON formats while state is extracted. Add repository interfaces and compatibility tests first; add SQLite and encryption behind those interfaces later. Multi-device sync is not implemented until schemas, versioning, migrations, and conflict rules are stable.

## Product and UI Rules

- Normal browsing remains normal; intelligence is invited, not intrusive.
- `docs/design_specification.md` is the source of truth for tokens and interaction language.
- Inter is the UI font; Lora is reserved for intelligence moments.
- Lit is shared selectively for Drift-owned surfaces and internal pages.
- Platform-native accessibility behavior takes precedence over forced pixel identity.

## Engine and Dependency Rules

- CEF is a fetched desktop dependency, not checked-in source and not Git LFS content.
- The CEF version pin remains shared between macOS and Windows bootstrap scripts.
- CEF types stop at `engine/cef/`; WebView types stop at `engine/android/`; WebKit types stop at `engine/webkit/`.
- No engine-specific assumption may enter `core/`.
- iPhone uses WKWebView by default; alternative engine work requires a separate entitlement/region decision.
- Android WebView versus GeckoView is decided by the SP-09 measured proof.

## Build and Verification

Current macOS build commands are maintained in `commands.md`:

```bash
./scripts/fetch-cef.sh
cmake -S . -B build -G Ninja -DPROJECT_ARCH="$(uname -m)" -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
open build/src/browser/Drift.app
```

Current automated application tests do not yet exist. EP-00-T02/T03 must establish:

- deterministic C++ core unit tests;
- serialization compatibility tests;
- BrowserEngine adapter contract tests;
- macOS characterization/smoke checks;
- sanitizer configurations where supported;
- platform build smoke jobs as platform proofs land.

No cross-platform parity, performance, or stability claim is accepted without execution on its target platform.

## Performance Reference

The existing macOS budgets in GD-07 remain the first reference. SP-08/SP-09 must select representative Windows, Android, and iPhone devices and lock equivalent budgets. C++ is an implementation tool, not evidence of performance.

## Key Files

| Question | File |
|---|---|
| What and why? | `PRD.md` |
| Cross-platform contract? | `docs/08_cross_platform_architecture.md` |
| What is true today? | `status_quo.md` |
| What is next? | `tasks.md` |
| How do I build/run? | `commands.md` |
| What does it look like? | `docs/design_specification.md` and `design_inspirations/` |
| What bugs are known? | `bugs3.md` |
| What rules govern a session? | `groundrules.md` |

## Explicit Foundation Non-Goals

- a full rewrite of the macOS browser;
- one browser engine on all four platforms;
- one UI technology for every surface;
- mobile Chrome-extension parity;
- cloud sync before local contracts are stable;
- empty abstractions without a runnable vertical slice;
- committing build products or downloaded SDKs.
