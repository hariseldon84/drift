# Drift Browser — Idea Record

Drift is a browser that understands what's inside pages, not just what pages say — and lets you work across them as if the web were one structured database.

## Purpose
This file captures the product idea and the decisions made during the product grilling process before formal PRD creation.

## Product Identity
**Name:** Drift / Drift Browser

**Core positioning:**  
Drift is a familiar, Chrome-class browser that understands what the user sees and helps the user act on it.

**Core moat direction:**  
**Object Understanding + Action Engine**

**Design philosophy:**  
Normal browsing remains normal. Intelligence appears subtly when useful and becomes powerful only when invited.

---

# 1. Product Foundation

## Chrome-class baseline
Drift must not force a user switching from Chrome to give up core browser jobs. Feature parity should be measured by **JTBD**, not by exact UI replication.

Expected baseline includes:
- Chromium-class website compatibility
- Chrome-extension compatibility wherever feasible
- bookmarks
- history
- profiles
- password management / autofill
- downloads
- private/incognito browsing
- tab groups
- pinned tabs
- sync
- devtools
- translation
- media handling
- import from Chrome
- session restoration
- performance controls
- search-engine choice
- familiar keyboard shortcuts

Dia should also be treated as a technology-parity benchmark for next-generation browser expectations, especially around browser AI, memory, tab organization, and contextual assistance.

---

# 2. Reliability and Resource Intelligence

## The problem
Browser clutter is not only a UX issue. For heavy users it creates:
- lost context
- microscopic / unmanageable tab bars
- excessive scrolling
- memory pressure
- slowdown
- browser instability or tab crashes
- fear of opening more tabs

## Drift tab resource states
Tabs remain conventional tabs. Drift adds invisible or lightly visible states underneath them:

1. **Active**
   - currently in use
   - full resource priority
   - normal/high visibility

2. **Warm**
   - recently used
   - fast to restore
   - slightly reduced resource priority
   - visually distinguishable but not intrusive

3. **Cold**
   - inactive / frozen / mostly dormant
   - minimal CPU and memory consumption
   - visually dimmed
   - instantly recoverable when clicked

4. **Archived**
   - removed from the visible tab strip
   - recoverable from the archive for that session
   - **never auto-archived**
   - archive is human-triggered and browser-suggested

## Archive rules
- Drift may use deterministic heuristics and ML to suggest tabs that appear stale / low-value.
- The browser should recommend archiving before severe clutter.
- **Cold → Archived is always human-triggered.**
- Archive is **session-specific by default**.
- Global search is simply browser history search across everything browsed.
- Archive is not a replacement for history.

## Memory-pressure behaviour
Drift should make browser resource use legible.

It should be able to show:
- total browser RAM usage
- heavy tabs
- memory pressure
- approximate recoverable memory
- resource use by tab / group / session where useful
- instability risk if technically credible

Behaviour:
- before memory pressure becomes dangerous (conceptually around ~80% of an internal danger threshold), show a subtle non-modal recommendation
- once actual danger is reached, automatically freeze low-priority inactive tabs
- never interrupt or unload currently active work where avoidable
- automatic intervention should be reversible

**Principle:** Warn early. Act only when necessary. Never interrupt active work.

The browser should make it feel safe to keep many tabs open:
**Keep everything. Stay fast. Don't crash.**

---

# 3. Browsing State Continuity

Drift should offer an optional enhanced continuity mode.

Potentially preserve:
- open tabs
- tab order
- groups
- pinned state
- scroll position where technically possible
- active/warm/cold state
- recently closed tabs
- crash-recovery checkpoints
- pending downloads where possible

This is a reliability feature, not a unique innovation claim.

---

# 4. Primary Innovation: Object-Level Web Interaction

Traditional browsers primarily treat webpages as documents.

Drift should treat a webpage as a collection of **meaningful objects that can be understood and acted upon**.

Examples:
- text passages
- products
- prices
- people
- companies
- images
- charts
- tables
- table rows
- forms
- code blocks
- videos
- embedded documents
- article sections
- structured data
- listings
- buttons / controls when safe to expose

## Object Recognition
Drift should automatically understand meaningful objects in a webpage using a hierarchy such as:
1. DOM / accessibility tree
2. structured metadata / schema
3. deterministic parsers
4. local ML models
5. local lightweight multimodal / language model
6. cloud model only when materially necessary

Object recognition may run quietly in the background.

**Recognition can be always-on. Interaction should be intent-triggered.**

Normal browsing must not be visually distorted merely because Drift can recognize page objects.

---

# 5. Universal Object Actions

When a user explicitly selects an object, Drift can show a compact contextual palette.

Potential universal verbs:
- Ask
- Compare
- Extract
- Save
- Search
- Act

The exact actions should adapt to the object type.

Examples:
- table → extract, clean, compare, export, chart
- product → compare, find alternatives, extract specifications, price research
- chart → explain, extract values, compare
- paragraph → summarize, translate, verify, search evidence
- image → search similar, extract text, save
- form → autofill, validate
- video → transcript, key moments, notes
- code → explain, copy cleanly, compare versions

The number of top-level actions should remain small and consistent.

---

# 6. Selection Sets and Object Tray

Users should be able to select multiple objects across multiple tabs.

Example:
- Product A from Amazon
- Product B from another retailer
- Product C from manufacturer website

They can then act on the set:
- compare
- extract common fields
- identify missing data
- search for alternatives
- summarize differences
- export

## Object Tray
A temporary object tray may make cross-tab selection tangible.

The user can place:
- products
- charts
- text passages
- images
- rows
- hotels
- research findings
- other recognized objects

into a temporary collection.

Default behaviour:
- tray is temporary
- persists across tabs in the current session
- disappears when the session ends unless explicitly saved

---

# 7. Structured Cross-Page Extraction

Drift should be able to normalize comparable objects from different websites into one structured view.

Example:
| Hotel | Price | Rating | Location | Cancellation |
|---|---:|---:|---|---|

or:
| Laptop | CPU | RAM | Weight | Battery | Price |

Extraction should prefer:
- DOM / structured metadata
- deterministic extraction
- local models
over unnecessary cloud calls.

---

# 8. Object Identity

Drift should attempt to understand when different webpages refer to the same real-world object.

Examples:
- same laptop model across three stores
- same company across article + company site
- same hotel across multiple booking sites
- same research paper across publisher + preprint
- same product with slightly different listing names

Confidence should exist internally.

External confidence indicators should appear only when uncertainty matters.

---

# 9. Live Objects

A user may optionally save an object as a **live object** rather than only saving a URL or static snapshot.

Examples:
- a product and its current price
- a stock price
- a flight
- a hotel offer
- an article
- a research item

Live objects may be refreshed later.

This capability should be optional and transparent.

---

# 10. Modes

Drift must preserve standard browsing.

## Normal Mode
Default.

- conventional browsing
- minimal intervention
- object understanding may run quietly
- no forced overlays
- no accidental drag/select behaviour

## Compare Mode
Optimized for:
- products
- services
- alternatives
- travel listings
- pricing/specification comparison
- structured multi-object extraction

Potential tools:
- Select
- Compare
- Price
- Specs
- Reviews
- Missing Fields
- Export

## Research Mode
Optimized for:
- articles
- papers
- reports
- documentation
- source comparison

Potential capabilities:
- collect sources
- detect duplicates
- identify new information
- contradictions
- evidence extraction
- citation / source views
- export

## Auto Pilot
A user explicitly chooses Auto Pilot.

Once activated:
- Drift may automatically infer the appropriate mode
- mode selection can use deterministic logic / local ML / lightweight local model
- high-confidence mode switches may happen automatically
- uncertain cases should not constantly interrupt users
- user can correct Auto Pilot
- corrections may improve local personalization
- Drift should visibly but subtly show the currently active inferred mode

**Top-level mode choice remains human-controlled.**
Automatic switching only occurs because the user explicitly selected Auto Pilot.

## Mode UI
When a mode is active:
- eligible objects may receive subtle outlines / annotations
- a contextual toolbar may appear
- normal webpage interactions should still remain predictable

Modes may internally combine capabilities even if the user sees one simple mode.

User-defined modes can exist later, not initially.

---

# 11. Information Novelty and Duplication

## Duplicate-information detection
Drift may identify pages or passages that substantially repeat information already read in the current research context.

Examples:
- 7 articles repeat the same facts
- a product listing adds no new specification
- a research source is materially duplicative

## What's New
One potentially powerful feature:

**Show me only what I don't already know.**

In Research Mode, Drift may highlight information that appears new relative to:
- selected sources
- current research set
- already-read relevant tabs

This should be transparent about uncertainty and source scope.

---

# 12. Browser Context

"Ask" should work across scoped browser context.

Possible scopes:
- current object
- current page
- selected objects
- selected tabs
- current selection set

Drift should not silently send the entire browsing session to a cloud model.

The user should understand what context is being used.

---

# 13. Action Engine

Drift should not only answer questions. It should be able to take action.

Examples:
- open the selected result
- reorganize tabs
- close duplicate tabs
- download selected PDFs
- find the same product elsewhere
- extract data
- fill form fields
- navigate
- perform safe browser-local actions

## Permission tiers
1. **Read / analyze**
   - automatic within explicit scope/mode

2. **Modify browser state**
   - automatic where reversible and low risk

3. **Interact with websites**
   - lightweight confirmation when meaningful

4. **Consequential external actions**
   - submit
   - send
   - purchase
   - delete
   - irreversible or financially meaningful action
   - always require explicit confirmation

## Action visibility
Drift should provide a subtle indicator of what it is doing:
- reading selected tabs
- comparing objects
- extracting a table
- filling a form
- performing an action

Avoid noisy agent logs in normal use.

---

# 14. Action Composition

Actions should be composable.

Example:
Select 4 products
→ compare
→ filter under budget
→ find missing specifications
→ search other sites
→ export

Prefer chains of understandable browser-native operations over hiding everything inside one opaque giant prompt.

---

# 15. Local-First Intelligence

**Default principle: Local first. Cloud only when the task actually requires it.**

Potentially local:
- tab-state prediction
- archive recommendations
- resource prediction
- page-object recognition
- structured extraction
- mode detection
- duplicate detection
- similarity
- object identity candidate generation
- local personalization
- UI routing decisions

Potentially cloud:
- deep synthesis
- difficult multimodal reasoning
- broad web research
- complex generation
- advanced agentic planning

Possible inference hierarchy:
**Rules → classical ML → local small model → cloud model**

Do not use GenAI where deterministic or conventional ML methods are better.

---

# 16. Result Presentation

The browser should select the least disruptive presentation surface.

Examples:
- quick answer → side panel
- tiny contextual action → overlay
- substantial comparison → structured new tab
- persistent output → save/export

Routing can be based on:
1. deterministic rules
2. local ML classifier
3. lightweight local model
4. cloud model only if truly needed

---

# 17. Extensibility

Future Drift actions could be extensible.

Examples:
- Send to Figma
- Create Jira issue
- Add to CRM
- Open in Photoshop
- Run custom agent

First versions should prioritize first-party actions.

Later:
- action / skill SDK
- third-party skills
- custom modes
- external agents
- perhaps MCP / CLI integrations

---

# 18. Browser Memory

Drift may maintain local-first browser memory such as:
- previously seen page
- previously compared product
- duplicate article detection
- earlier cheaper listing
- prior selected objects
- relevant research context

This is not intended to become an opaque "memory of the user's whole life."

Memory should be:
- local-first
- user-controllable
- scope-aware
- transparent
- deletable

---

# 19. Chrome and Dia Parity

Drift should benchmark against:
- Chrome for baseline browser completeness and ecosystem compatibility
- Dia for next-generation browser expectations

The rule:
**Never be meaningfully worse than Chrome at conventional browsing. Never feel technologically behind Dia at next-generation browsing. Differentiate beyond both.**

Parity does not mean copying UI.

---

# 20. Switching

Drift should minimize switching friction.

Aim to import where feasible:
- bookmarks
- passwords
- history
- open tabs
- pinned tabs
- profiles
- extension configuration where technically possible

Chrome extension compatibility is strategically important.

---

# 21. Target User and Commercial Model

Initial target:
**Consumer / power users first**

Not enterprise-first.

Possible commercial model:
- free core browser
- paid Pro tier for advanced intelligence / cloud usage / premium modes / higher limits

Exact packaging remains unfrozen.

---

# 22. Product Promise

Current leading category definition:

> **Drift is the browser that understands what you see and helps you act on it.**

Shorter innovation expression:

> **Act on anything you see.**

Reliability promise underneath:

> **Keep everything. Stay fast. Don't crash.**

---

# 23. Non-Goals / Avoidances

Drift should NOT initially become:
- a replacement operating system
- an email client
- a workspace suite
- a Notion replacement
- a VPN bundle
- a giant always-on AI dashboard
- a prompt-first browser
- a radical replacement for tabs
- a product where every webpage element accidentally becomes draggable
- an enterprise governance platform in V0
- a model playground exposing dozens of AI-provider controls

---

# 24. Open Questions Remaining Before PRD

Still to freeze through product validation and architecture work:
- exact V0 feature set
- first killer workflow
- which modes ship in V0
- exact system resource thresholds and OS-specific implementation
- object-recognition taxonomy
- local model stack
- cloud-model policy
- extension compatibility boundaries
- sync architecture
- account model
- saved-object persistence
- security model
- monetization packaging
- telemetry / privacy policy
- Mac vs Windows launch sequence
- measurable competitive benchmarks
- performance budgets
- required browser-engine modifications
- rollout and experimentation strategy

A PRD should be created only after these are sufficiently frozen.
