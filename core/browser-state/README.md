# Browser State Core

This directory contains the first platform-neutral C++20 contract and tested
state reducer for Drift. It has no CEF, AppKit, Objective-C++, Swift, Kotlin,
JNI, WebKit, filesystem, or UI dependency.

## Files

- `browser_contract.h` — strongly typed IDs plus command, event, download, and
  engine-error vocabulary.
- `browser_state.h/.cpp` — deterministic tab/download state and event reducer.
- `browser_state_codec.h/.cpp` — bounded, versioned snapshot codec.

These C++ types are internal core types. They are **not** exported directly
across a DLL, Swift, or Kotlin boundary. Phase 4 maps them to the stable C ABI
or serialized protocol used by adapters.

## Snapshot format: `DRIFTBS1`

The codec is a deterministic compatibility fixture for core state. All integer
fields are unsigned little-endian. Strings are UTF-8 byte sequences prefixed
by a `u32` byte length. Boolean fields are one byte and accept only `0` or `1`.

```text
8 bytes  magic: DRIFTBS1
u64      active tab ID; 0 means none
u32      tab count
repeat tabs:
  u64    tab ID
  string URL
  string title
  u8     pinned
  u8     loading
  u32    progress in millionths (0..1,000,000)
  u8     can go back
  u8     can go forward
u32      closed-tab count
repeat closed tabs:
  string URL
  string title
  u8     pinned
u32      download count
repeat downloads:
  u64    download ID
  u64    originating tab ID; 0 means absent
  string file name
  u64    received bytes
  u64    total bytes; 0 means unknown
  u8     DownloadStatus
```

The decoder rejects unknown versions, duplicate/zero required IDs, an active
ID not present in the tab list, invalid booleans/status/progress, impossible
download byte counts, truncated input, trailing bytes, collections above
100,000 entries, and strings above 16 MiB.

The encoder validates its in-memory input and returns an empty byte vector when
the state violates the same invariants. Callers must treat an empty result as a
serialization failure; the valid empty browser state still produces a
non-empty `DRIFTBS1` payload.

Changing the existing layout requires a new magic/version and compatibility
tests. Do not reinterpret `DRIFTBS1` fields in place.
