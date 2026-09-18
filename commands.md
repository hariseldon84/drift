# Drift — Commands

> Every runnable command lives here in full, copy-paste ready.
> Created: SP-01 (EP-01-T01). Updated each sprint as new commands appear.

---

## Prerequisites

```bash
# Xcode Command Line Tools (if not already installed)
xcode-select --install

# CMake 3.25+
brew install cmake

# Ninja (faster builds than make)
brew install ninja
```

---

## Step 1 — Download CEF Binary Distribution

From the repository root, run the pinned dependency bootstrap:

```bash
./scripts/fetch-cef.sh
```

The script detects Apple Silicon vs Intel, downloads CEF
`152.0.6+g708dc14+chromium-152.0.7977.83`, verifies its pinned SHA-1, and
extracts it under `.deps/`. Both `.deps/` and the generated `build/` tree are
ignored by Git. CEF is required to build and run Drift, but its binaries do not
belong in this source repository or Git LFS.

---

## Step 2 — Configure the Build

Run from the `drift/` repo root.

```bash
cmake \
  -S . \
  -B build \
  -DPROJECT_ARCH="$(uname -m)" \
  -DCMAKE_BUILD_TYPE=Release \
  -G Ninja
```

Expected output ends with: `-- Build files have been written to: .../drift/build`

---

## Step 3 — Build

```bash
cmake --build build --parallel
```

First build takes ~5–10 minutes (compiling the CEF C++ wrapper).
The build automatically ad-hoc signs the app bundle with JIT entitlements (required for V8 on Apple Silicon).
Subsequent incremental builds are fast (~seconds).

---

## Step 4 — Run

```bash
open build/src/browser/Drift.app
```

Expected: A native macOS window opens and loads `https://example.com`.

> Always use `open`, not the binary directly. `open` goes through LaunchServices so macOS treats it as a proper app bundle — required for the multi-process architecture to work.

---

## Step 5 — Quit

Close the window or press **Cmd+Q**. The app shuts down cleanly.

To force-kill from terminal (e.g. if something is stuck):
```bash
pkill -9 -f "Drift"
```

---

## Step 6 — Verify Process Architecture

With Drift running, open Activity Monitor. You should see:
- `Drift` — browser process (main)
- `Drift Helper (Renderer)` — renderer process (runs V8, renders pages)
- `Drift Helper (GPU)` — GPU compositing process
- `Drift Helper` — utility/network helper

This confirms the multi-process CEF architecture is working.

---

## Rebuild after source changes

```bash
cmake --build build --parallel
```

No need to re-run `cmake` unless you add new source files to `CMakeLists.txt`.

---

## Clean build

```bash
cmake -E remove_directory build
# Then repeat Step 2 and Step 3
```

---

## Fresh clone on another Mac

```bash
git clone https://github.com/hariseldon84/drift.git
cd drift
./scripts/fetch-cef.sh
cmake -S . -B build -G Ninja -DPROJECT_ARCH="$(uname -m)" -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
open build/src/browser/Drift.app
```

The current application shell is macOS-specific Objective-C++/AppKit code, so
this checkout does not yet build on Windows. The future Windows shell will use
the matching Windows CEF binary while sharing platform-neutral C++ product code.
