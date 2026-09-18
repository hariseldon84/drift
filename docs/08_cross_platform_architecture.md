# Drift Cross-Platform Architecture

> Status: locked for the foundation migration on 2026-09-18.
> This document supersedes the platform assumptions in `docs/04_architecture.md` where they conflict. The product concepts and security model in that document remain valid.

## 1. Decision

Drift is one multi-device product targeting macOS, Windows, Android, and iPhone. It is not one identical executable and it is not four unrelated browsers.

The architecture is:

1. a shared, platform-neutral C++20 product core;
2. a small native shell for each operating system;
3. an engine interface with platform-specific adapters;
4. shared Lit surfaces for Drift-owned product experiences where web UI is a good fit;
5. platform-specific packaging, security integration, and lifecycle code.

The current macOS browser remains the behavioral reference while code is extracted behind these boundaries. This is a controlled refactor, not a rewrite.

## 2. Why C++20, and what it does not guarantee

C++ is the right initial shared-core choice because the working desktop browser and CEF already use C++, it has excellent native interoperability, and it gives control over latency, allocation, and binary size. That avoids migrating working browser behavior before the architecture is proven.

Being close to the machine does **not** automatically make software faster, stable, or reliable. C++ also permits memory corruption, lifetime errors, data races, and undefined behavior. Drift earns reliability through constraints:

- RAII and value types for ownership;
- no raw owning pointers;
- explicit thread-affinity rules at every boundary;
- deterministic state transitions with unit tests;
- AddressSanitizer, UndefinedBehaviorSanitizer, and ThreadSanitizer in supported development/CI jobs;
- fuzzing for parsers and serialized input when those modules exist;
- no C++ standard-library types across DLL, Swift, Kotlin, JNI, or Objective-C ABI boundaries.

Platform bindings use a narrow C ABI or a versioned serialized command/event protocol. C++ implementation details stay inside the core.

## 3. Target repository shape

```text
apps/
  macos/                    native AppKit/Objective-C++ shell
  windows/                  native Win32/WinUI shell
  android/                  native Kotlin shell
  ios/                      native Swift shell
core/
  domain/                   Product, ObjectGraph, CompareSession, ActionPlan
  browser-state/            tabs, navigation, bookmarks, history, downloads
  intelligence/             extraction and normalized results
  storage/                  repository interfaces and migrations
  sync/                     versioned sync contracts; implementation later
engine/
  api/                      platform-neutral BrowserEngine contract
  cef/                      macOS and Windows CEF adapters
  android/                  Android engine adapter
  webkit/                   iPhone WKWebView adapter
ui/
  components/               shared Lit components and design tokens
  compare/                  Compare Mode surfaces
  objects/                  object cards and object panels
  widgets/                  connected widget surfaces
  internal-pages/           history, downloads, settings, diagnostics
tests/
  core/                     deterministic state and serialization tests
  contract/                 adapter conformance tests
  platform/                 native integration and smoke tests
```

This is the destination, not the present tree. Until extraction is complete, `src/browser/` remains the working macOS implementation.

## 4. Platform capability matrix

| Capability | macOS | Windows | Android | iPhone |
|---|---|---|---|---|
| Native shell | AppKit / Objective-C++ | Win32 or WinUI / C++ | Kotlin | Swift |
| Initial engine | CEF | CEF | Android WebView proof | WKWebView |
| Shared C++20 core | Yes | Yes | Yes, through JNI/C ABI | Yes, through C ABI/Objective-C++ bridge |
| Drift Lit surfaces | Yes | Yes | Selected surfaces | Selected surfaces |
| Tabs, bookmarks, history, objects | Full target | Full target | Full target | Full target |
| Chrome extensions | Desktop only | Desktop only | Not in initial scope | Not in initial scope |
| Local intelligence | Core ML/portable core | ONNX/portable core | Platform runtime/portable core | Core ML/portable core |
| Packaging | Signed/notarized app | Signed installer | Play distribution | App Store distribution |

Engine choice is an adapter decision, not a product-model decision. Android must validate WebView against required browser capabilities before Drift commits to it; GeckoView remains a possible fallback. WKWebView is the default iPhone engine because alternate browser-engine entitlement and regional distribution constraints make a bundled Chromium engine an unsuitable baseline.

## 5. Runtime flow

```text
Native shell or Lit surface
          |
          | versioned Command
          v
Shared C++20 state + product core
          |
          | BrowserEngine request
          v
Platform engine adapter
          |
          | normalized Event/Result
          v
Shared core -> observable state -> shell/surface renders
```

The UI does not call CEF, WebView, or WKWebView directly for product behavior. The core does not import AppKit, WinUI, Swift, Kotlin, JNI, CEF, or WebKit types.

## 6. BrowserEngine contract

The first contract is intentionally small and driven by existing behavior.

Commands include:

- create, select, close, reorder, and restore a tab;
- navigate, go back, go forward, reload, and stop;
- request page metadata or a normalized extraction;
- start, cancel, and inspect a download;
- report engine-relevant host state such as viewport, foreground/background, or network availability when the engine requires it.

Events include:

- tab created, selected, moved, or closed;
- URL, title, favicon, loading, and navigation capability changes;
- load failure and renderer termination;
- download progress and completion;
- normalized extraction result;
- recoverable and terminal engine errors.

Identifiers, commands, events, and errors are platform-neutral value types. Adapter conformance tests run the same command/event scenarios against every engine that can run in CI.

## 7. State and persistence

The shared core is the source of truth for Drift-owned state. Engine adapters report facts; shells render state and collect user intent.

Extraction order:

1. bookmarks as the first complete vertical slice;
2. history and downloads;
3. tabs, navigation, and session restore;
4. Product, ObjectGraph, CompareSession, and ActionPlan.

Persistence sits behind repository interfaces. Existing JSON files remain the first implementation so behavior is preserved. SQLite and encryption are added behind the same interfaces after serialization compatibility tests exist. Sync is delayed until local schemas, versioning, and conflict rules are stable.

Native shells own operating-system permissions and application lifecycle. They publish
platform-neutral host-state notifications to the core only when product behavior needs
them; they are not BrowserEngine commands and do not expose platform permission types.

Future multi-device sync is optional and end-to-end encrypted. Eligible categories are
bookmarks, settings, saved objects/comparisons, tasks, and notes, each explicitly
enabled by the user. Browsing history, raw page content, downloads, keystrokes, form
data, and credentials never sync. The service stores ciphertext; schemas, migrations,
conflict rules, key recovery, category-level consent, and deletion propagation must be
specified and tested before implementation. Cloud inference, if introduced, is a
separate opt-in flow and never silently reuses sync consent.

## 8. UI sharing rule

Native code owns window lifecycle, browser chrome where the platform requires it, accessibility integration, secure storage, menus, permissions, and engine hosting.

Lit owns Drift-designed surfaces that benefit from one visual and interaction implementation: object cards, Compare Mode, widgets, history/download/settings pages, and diagnostics. Lit does not host the browser engine, replace native lifecycle code, or become a universal abstraction over every platform control.

Shared design tokens and interaction specifications apply even when a mobile surface is implemented natively.

## 9. Migration phases and exit gates

| Phase | Work | Exit gate |
|---|---|---|
| 0 | Preserve working browser | `V018Sep2026` pins the verified snapshot and `main` begins from that commit |
| 1 | Lock product and architecture | Canonical docs agree on four targets, boundaries, and sprint order |
| 2 | Contracts and characterization tests | Platform-neutral types compile; state/serialization behavior is deterministic |
| 3 | Extract shared state | Bookmarks, history, downloads, tabs, and navigation use tested core models |
| 4 | Introduce engine boundary | macOS behavior runs through `BrowserEngine` with no regression |
| 5 | Separate persistence | Repository contracts preserve existing data and support SQLite behind the boundary |
| 6 | Windows proof | One Windows CEF tab navigates through shared commands/events |
| 7 | Mobile proofs | Android and iPhone navigate and produce one normalized extraction result |
| 8 | Resume product vision | Object graph, Compare Mode, widgets, and intelligence build on shared contracts |

No phase advances by creating empty folders alone. Each gate requires a thin, runnable vertical slice.

## 10. Alternatives considered

**Four fully native product implementations:** best platform fidelity, but duplicates the state model and Drift's differentiating behavior. Native shells are retained; product logic is shared.

**React Native or Flutter for the entire browser:** useful for application UI, but browser hosting, multi-process behavior, extensions, lifecycle, security, and platform distribution still require substantial native work. They do not remove the hard engine problem.

**Kotlin Multiplatform / Compose Multiplatform:** credible for greenfield shared business logic and UI. It is not selected now because it would replace the existing C++/CEF center before boundaries are proven. Reconsider only after the engine contract exists and a measured prototype shows lower lifecycle cost.

**Rust shared core:** attractive memory-safety properties and proven in cross-platform browser components. Reconsider for new isolated modules or a future core migration after the command/event ABI is stable; do not rewrite working browser state merely to change languages.

**One CEF build on every platform:** CEF is appropriate for macOS and Windows. It is not the baseline for Android or iPhone, so the architecture must not leak CEF concepts into shared product logic.

## 11. Non-goals for the foundation

- rewriting the existing macOS browser before characterization tests exist;
- pixel-identical chrome on all four operating systems;
- mobile Chrome-extension compatibility;
- cloud sync before local data contracts are stable;
- committing CEF or generated build artifacts to Git or Git LFS;
- claiming performance or parity without measurements on each target.
