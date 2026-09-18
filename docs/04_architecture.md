# Drift Browser — Technical Architecture

> Historical product-system architecture. Platform and code-organization assumptions
> are superseded by `docs/08_cross_platform_architecture.md` (2026-09-18).
> The Web Object, page-understanding, action, security, and provenance concepts below
> remain design inputs unless the PRD says otherwise.

## 1. Architectural Thesis

Drift should be built as a **Chromium-derived desktop browser shell** whose differentiated capability sits above the rendering engine.

Do not build a rendering engine.

Primary architectural layers:

1. Chromium browser/runtime
2. Drift browser shell
3. Resource Intelligence subsystem
4. Page Understanding subsystem
5. Object Graph / Object Store
6. Interaction & Mode subsystem
7. Action Engine
8. Local Intelligence runtime
9. Cloud Intelligence gateway
10. Persistence / sync / security
11. Observability and experimentation

The crucial architectural concept is the **Web Object**.

Everything distinctive in Drift should ideally compose around one common object representation.

---

# 2. Process Model

Use Chromium's existing multi-process architecture:
- browser process
- renderer processes
- GPU process
- utility/network processes
- extension processes

Drift-specific services should run outside renderer processes whenever possible.

Suggested additional logical services:
- Resource Manager Service
- Page Understanding Service
- Object Store Service
- Mode Orchestrator
- Action Orchestrator
- Local Model Service
- Cloud Model Gateway
- Session Archive Service
- Drift Telemetry Service

Isolation is essential because page content is untrusted.

---

# 3. Drift Web Object Model

Every recognized object should map into a typed internal schema.

Example:

```json
{
  "object_id": "obj_...",
  "type": "product",
  "page_id": "page_...",
  "session_id": "session_...",
  "source_url": "https://...",
  "source_origin": "example.com",
  "dom_locator": "...",
  "accessibility_locator": "...",
  "visual_bounds": {"x":0,"y":0,"w":0,"h":0},
  "canonical_text": "...",
  "attributes": {
    "title": "...",
    "price": 9999,
    "currency": "INR",
    "brand": "...",
    "model": "..."
  },
  "provenance": {
    "title": "dom",
    "price": "schema_org",
    "model": "local_model"
  },
  "confidence": {
    "type": 0.98,
    "identity": 0.84
  },
  "created_at": "...",
  "expires_at": null
}
```

The exact schema should support:
- typed attributes
- per-field provenance
- per-field confidence
- page/session ownership
- stable selection state
- identity candidates
- serialization into tray/compare views

---

# 4. Page Understanding Pipeline

## Stage A — Deterministic extraction
Input:
- DOM
- accessibility tree
- semantic HTML
- OpenGraph
- schema.org / JSON-LD
- microdata
- visible text
- CSS/layout metadata

Output:
- candidate objects
- candidate object types
- field candidates

This should be the cheapest and fastest path.

## Stage B — Lightweight heuristics
Examples:
- repeated card layouts
- price regex + currency context
- table headers/rows
- article hierarchy
- form semantics
- product title/price/image proximity
- visible-listing structures

## Stage C — Local ML classifier
Use for:
- object type
- card boundaries
- mode inference
- stale-tab classification
- semantic duplicate estimation
- same-object candidate matching

## Stage D — Local lightweight language/multimodal model
Use only when semantics are not captured reliably by structured page data.

Possible jobs:
- infer product model from text
- identify comparable fields
- classify ambiguous object
- summarize a small selected object
- normalize labels

## Stage E — Cloud reasoning
Escalate only for:
- complex synthesis
- difficult multimodal content
- broad comparison
- advanced extraction from hostile/unstructured pages
- deep research

The request should contain the minimum scoped context needed.

---

# 5. Object Selection Architecture

Normal Mode must not intercept page interaction.

Use an explicit selection activation path:
- mode active
- keyboard modifier
- object-picker button
- command palette action
- context menu

Implementation considerations:
- overlay hit regions rendered by browser chrome or isolated extension-style surface
- never alter site DOM when avoidable
- visual outline should be non-destructive
- pointer events should not interfere unless selection state is active
- accessibility keyboard path required

Selection state should live in browser process / Drift service, not webpage JavaScript.

---

# 6. Object Tray

The tray is a session-scoped object collection.

State model:
- object reference
- source snapshot
- extracted fields
- selection timestamp
- page availability
- freshness
- optional thumbnail
- provenance

Requirements:
- persists when switching tabs
- tolerates source tab being closed
- supports removal/reorder
- supports multi-select actions
- may persist across restart only if user chooses to save the set

Storage:
- encrypted local SQLite or equivalent
- binary thumbnails/blobs stored separately
- session IDs prevent accidental cross-context leakage

---

# 7. Compare Engine

Pipeline:

1. collect selected objects
2. resolve object types
3. run identity matching
4. infer common comparison schema
5. normalize field names
6. normalize units/currencies
7. preserve source provenance
8. detect missing fields
9. optionally search selected missing fields
10. render structured comparison tab

Important:
- never fabricate absent fields
- distinguish extracted vs inferred vs generated values
- every field should retain a source pointer where possible
- comparison rendering should remain useful without an LLM

For products, structured metadata should be preferred over generated extraction.

---

# 8. Research Engine

Research Mode pipeline:

1. selected pages/passages
2. chunk visible content
3. create local embeddings / fingerprints
4. cluster semantic duplicates
5. identify unique propositions
6. map proposition → source(s)
7. calculate novelty relative to current research set
8. optional contradiction candidate detection
9. deep synthesis via cloud model only when requested

"What is new?" should be defined relative to an explicit corpus:
- selected tabs
- research tray
- current session
- optionally browser history later

Never imply universal novelty.

---

# 9. Action Engine

The Action Engine should use typed commands rather than free-form model-generated browser code.

Example command family:
- OPEN_URL
- FOCUS_TAB
- MOVE_TAB
- CLOSE_TAB
- FREEZE_TAB
- DOWNLOAD_RESOURCE
- SEARCH_OBJECT
- EXTRACT_OBJECT
- FILL_FIELDS
- CLICK_ELEMENT
- SUBMIT_FORM

Each command needs:
- risk class
- reversibility
- origin
- user scope
- required permission
- execution timeout
- success/failure result
- audit entry

## Risk tiers
0. read-only
1. browser-local reversible
2. site interaction
3. consequential external action

Tier 3 always requires confirmation.

Agent reasoning may propose commands, but only the deterministic Action Engine executes them.

---

# 10. Modes

Modes are policy bundles, not separate browsers.

A Mode config can define:
- enabled object types
- visual annotations
- action palette
- resource priorities
- context scope
- comparison schema hints
- local model profiles
- cloud escalation policy

Examples:

## Normal
- object recognition low/medium intensity
- no overlay unless invoked
- standard tab policy

## Compare
- product/listing detection high
- object outlines when hovered in picker state
- tray visible
- comparison actions prioritized

## Research
- article/passage/table detection high
- source provenance prioritized
- semantic dedupe enabled
- research tray enabled

## Auto Pilot
- top-level mode explicitly enabled by user
- local classifier estimates mode
- confidence threshold determines switching
- visible indicator shows current inferred mode

---

# 11. Resource Intelligence

Start with OS and Chromium metrics:
- renderer RSS/private memory where available
- CPU
- page visibility
- last interaction
- media playback
- WebRTC
- active downloads
- form-edit state if detectable
- notification/service-worker importance
- tab history recency

Tab-state classifier:
- Active
- Warm
- Cold

Use a risk score for freezing:
- current tab = never
- audio/video = protect
- active WebRTC = protect
- recent interaction = protect
- unsaved-input suspicion = protect where possible
- old static content = good freeze candidate

Archive recommendation is a separate ML problem and never executes automatically.

"Crash prediction" should not ship as a claim until empirical precision is proven. A safer V0 term is **memory pressure / stability risk**.

---

# 12. Local Model Runtime

Requirements:
- Mac Apple Silicon optimization first if launching Mac first
- Windows CPU/GPU/NPU support later or in parallel depending launch plan
- quantized models
- strict memory budget
- process isolation
- model unload when idle
- batching where possible

Model categories:
- embedding model
- object classifier
- reranker
- small language model
- optional small vision-language model

Do not keep every model resident.

Model router should ask:
1. can deterministic logic solve it?
2. can classical ML solve it?
3. can local model solve it within latency/quality budget?
4. only then call cloud.

---

# 13. Cloud Intelligence Gateway

One internal interface should abstract cloud providers.

Responsibilities:
- redaction
- context minimization
- model routing
- quota accounting
- retries
- structured-output enforcement
- abuse/safety checks where needed
- provider failover
- logging without storing sensitive payloads by default

The product should not expose provider complexity to V0 users.

---

# 14. Persistence

Recommended stores:
- Chromium native profile data for core browser jobs
- Drift encrypted SQLite database for:
  - sessions
  - archive metadata
  - tray objects
  - comparison state
  - local embeddings references
  - mode preferences
  - feature flags
- vector index for research/object similarity
- encrypted blob cache for object snapshots/thumbnails

Sensitive-site exclusions should be implemented at collection time, not merely at retrieval time.

---

# 15. Sync

V0 can defer full Drift sync if needed.

V1 sync candidates:
- preferences
- modes
- saved comparisons
- saved objects
- archive metadata
- custom actions later

Do not sync:
- incognito data
- sensitive-site object cache by default
- raw page snapshots without explicit policy

Use end-to-end encryption where practical for private user state.

---

# 16. Extension Compatibility

Base on Chromium's extension APIs.

V0:
- support Manifest V3
- import installed extensions where feasible
- preserve common extension UX

Security:
- extension content must not be silently trusted by Drift object extraction
- differentiate page DOM from extension-injected DOM where possible
- object selectors should avoid leaking extension-private UI

---

# 17. Security Threat Model

Major threats:
- malicious pages poisoning object interpretation
- prompt injection in web content
- deceptive DOM structures
- malicious extension interference
- action-engine abuse
- credential exfiltration
- cloud context leakage
- UI spoofing
- clickjacking-like agent manipulation

Rules:
- webpage text is data, never trusted instructions
- models cannot directly execute browser operations
- Action Engine validates commands
- origin boundaries remain explicit
- passwords/payment data are never included in model context by default
- consequential actions require confirmation
- model-generated selectors must be constrained and validated

---

# 18. Observability

Measure:
- cold-tab memory reclaimed
- tab wake latency
- browser crash rate
- renderer crash rate
- object-detection precision/recall
- object selection correction rate
- compare completion rate
- field-extraction accuracy
- field provenance coverage
- action success/failure
- user undo/cancel rate
- cloud escalation rate
- local inference latency
- mode correction rate
- retention by feature cohort

Do not optimize merely for AI usage. Optimize for completed browser jobs.

---

# 19. Performance Budgets

Initial targets to validate experimentally:
- object overlays should not cause perceptible scroll jank
- page-understanding pipeline must be incremental, not reparse entire page continuously
- object recognition should prioritize viewport + near-viewport
- background object processing pauses under resource pressure
- local model inference must yield to active browsing
- cold-tab wake should feel near-instant for ordinary pages
- Drift overhead relative to baseline Chromium should be explicitly benchmarked

Exact numeric budgets should be established during prototype benchmarking rather than invented now.

---

# 20. Suggested Repository Shape

```text
/drift
  /chromium
  /shell
  /resource_manager
  /page_understanding
  /object_model
  /object_tray
  /compare_engine
  /research_engine
  /mode_orchestrator
  /action_engine
  /local_models
  /cloud_gateway
  /persistence
  /sync
  /security
  /telemetry
  /experiments
  /tests
```

---

# 21. Highest-Risk Technical Bets

1. reliable semantic object boundaries across arbitrary websites
2. cross-site field normalization without hallucination
3. same-object identity across heterogeneous listings
4. local inference quality within browser resource budgets
5. interaction overlays that never damage normal website behavior
6. safe action execution under hostile web content
7. proving reliability gains without Drift itself increasing memory pressure

These should be prototyped before large-scale UI investment.
