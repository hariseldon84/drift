# Drift Browser — V0 MLP (Commercially Usable Minimal Lovable Product)

## Goal
V0 must be a browser that a real power user can install as a daily browser and reasonably pay for.

It cannot be a technology demo.

The product must have:
1. credible browser completeness
2. one clearly lovable differentiated workflow
3. noticeably better reliability visibility
4. low switching friction

## V0 Product Promise

> **Drift is a familiar browser that lets you select things from across the web and turn them into useful actions.**

V0 should intentionally narrow the moat to one killer workflow:
**Compare Mode for products / comparable listings.**

Why:
- visually obvious
- easy to demo
- naturally cross-site
- measurable
- object recognition is directly useful
- structured extraction creates immediate value
- does not require users to learn prompts
- provides a path into the generalized object platform

## V0 Killer Experience

Scenario:
1. user opens three laptop pages across Amazon, Flipkart and a manufacturer site
2. turns on Compare Mode
3. Drift outlines the product object on each page
4. user clicks `+ Compare` or drags each product into the tray
5. tray follows across tabs
6. Drift extracts:
   - product name
   - model
   - price
   - CPU
   - RAM
   - storage
   - display
   - weight
   - battery where available
   - source
7. Drift opens a structured comparison tab
8. missing fields are shown as missing, not invented
9. user can sort/filter
10. user can copy/export the comparison
11. optional Pro action: "Find missing specs" / "Explain differences"

This should feel substantially faster than copying links into ChatGPT.

## V0 Required Browser Baseline

### Core
- Chromium compatibility
- Mac + Windows if feasible; if sequencing is needed, choose one deliberately
- Chrome extension compatibility
- address/search bar
- tabs
- pinned tabs
- basic tab groups
- bookmarks
- history
- downloads
- private mode
- passwords/autofill via secure integration
- site permissions
- DevTools
- Chrome import
- crash restore

### Migration
One-click-ish import:
- bookmarks
- history
- passwords where platform/Chromium APIs allow
- pinned tabs
- extensions list / install migration where feasible

## V0 Reliability Layer
- Active / Warm / Cold tab states
- subtle visual state
- automatic safe freezing
- browser memory-health panel
- heavy-tab list
- memory-pressure recommendation
- session-specific archive
- manual archive
- recovery after restart/crash

Do **not** ship "crash prediction" unless validated.

## V0 Object Platform
Support only high-value V0 object types:
- product/listing
- text passage
- table
- image
- link/document

Product/listing gets the deepest support.

## V0 Interaction
- Normal Mode
- Compare Mode
- explicit object picker
- modifier-click selection
- subtle outlines only while Compare Mode / picker is active
- object tray
- cross-tab selection
- remove/reorder tray items
- object inspector basic view
- contextual actions:
  - Compare
  - Extract
  - Search
  - Ask
  - Copy/Export

## V0 Intelligence
Local:
- page candidate extraction
- product-card detection
- object type classifier
- basic field normalization
- duplicate URL/tab detection
- mode UI routing
- resource classifier

Cloud:
- hard extraction fallback
- deep explanation
- "find missing fields" if shipped
- natural-language comparison interpretation

## V0 Security / Privacy
- local-first label in settings
- clear cloud escalation indicator
- sensitive-site exclusion
- incognito has no Drift memory
- do not send passwords/payment fields
- read-only scope by default
- no autonomous purchase/submission

## V0 Monetization
Recommended structure:

### Free
- full browser
- resource intelligence
- basic Compare Mode
- limited object comparisons/month or limited advanced cloud actions
- local extraction
- Chrome extensions

### Drift Pro
- higher/unlimited advanced comparisons subject to fair use
- cloud-assisted missing-field research
- advanced structured extraction
- richer Ask on selection sets
- saved comparisons
- early Research Mode beta

Pricing should be tested, not frozen here.

## Explicit V0 Cuts
Not V0:
- Research Mode as full system
- Auto Pilot
- custom modes
- external agents/MCP
- live objects
- cross-site persistent object memory
- enterprise controls
- VPN
- email
- workspace
- fully autonomous agent
- purchases / submissions
- deep contradiction detection
- "what's new relative to everything I read"
- broad user-defined automation

## V0 Success Metrics

### Browser viability
- daily crash/renderer-crash rate competitive with Chromium baseline
- startup and navigation performance within acceptable delta to Chrome
- extension compatibility satisfaction
- successful Chrome import rate

### Differentiated workflow
- % of activated users who complete first comparison
- time-to-first-comparison
- number of sites represented per comparison
- extraction correction rate
- comparison export/copy rate
- repeat Compare Mode use within 7 days

### Commercial
- free → Pro conversion
- 4-week retention among Compare Mode users
- paid-user weekly comparison frequency
- cloud cost per paid user

## V0 Litmus Test
A user should be able to say:

> "I switched because Drift feels like Chrome, but comparing anything across the web is dramatically easier — and it handles my ridiculous number of tabs better."

If users instead say:
> "It's Chrome with an AI panel,"
V0 has failed.
