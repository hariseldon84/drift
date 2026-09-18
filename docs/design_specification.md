# Drift — Design Specification

> Source of truth for all visual and brand decisions.
> Derived from: designinspirationstory.md + grilling sessions (2026-09-15) + confirmed against built mockups in design_inspirations/.

---

## The Core Metaphor

**The Shimla Cafe.** The user is on a mountain road — the chaotic, potholed, beautiful web. Drift is the warm cafe they find. Inside: calm, organized, enveloping warmth. Through the windows: the crisp, golden, foresty world outside. The UI is the cafe interior. Intelligence moments are light coming through the window.

---

## Color Palette

All hex values confirmed against built mockups in design_inspirations/.

### Day Mode (default)

| Token | Hex | Role | Emotional anchor |
|---|---|---|---|
| `--drift-bg` | `#E7E1D6` | Page/canvas background | Warm sandy earth outside the cafe |
| `--drift-base` | `#FAF7F2` | Browser frame, primary surface | Interior plaster of an old Shimla colonial building |
| `--drift-surface` | `#F2EDE4` | Cards, panels, elevated surfaces | The wooden cafe table |
| `--drift-muted` | `#C6BBAD` | Borders, dividers, inactive elements | Faded wood grain |
| `--drift-control` | `#D6C1A4` | Window controls, subtle UI chrome | Warm stone, unobtrusive |
| `--drift-text` | `#1C1410` | Primary text | Old ink on a menu card — never pure black |
| `--drift-text-secondary` | `#7A6E65` | Secondary text, labels | Faded print |
| `--drift-amber` | `#D4820A` | Intelligence accent — ONLY when Drift activates | The 8am February sun through the window |
| `--drift-amber-light` | `#E8960C` | Intelligence accent hover/highlight | Brighter sunlight on a surface |
| `--drift-green` | `#2D5016` | Structure accent — navigation, confirmations | The forest that frames the road |
| `--drift-green-mid` | `#3A6B1A` | Structure hover states | Lighter canopy |

### Evening / Night Mode

| Token | Hex | Role | Emotional anchor |
|---|---|---|---|
| `--drift-night-bg` | `#1E1A16` | Page/canvas | Old wood, warm shadow — not cold grey |
| `--drift-night-base` | `#2A2420` | Browser frame | The cafe at dusk, lamp-lit |
| `--drift-night-surface` | `#332E28` | Cards, panels | Warm dark wood |
| `--drift-night-text` | `#EDE8E0` | Primary text | Warm off-white |
| `--drift-night-amber` | `#C4780A` | Intelligence accent (muted for dark) | Candlelight, not sunlight |
| `--drift-night-silver` | `#E8E4DC` | Moonlight accent | The moon through the window |
| `--drift-night-green` | `#4A7A2A` | Structure accent (muted) | Forest at dusk |

### Never use
- Corporate blue
- Neon, purple, cyan, electric green
- Cold grey (reads anxious, clinical — the opposite of Drift)
- Pure `#000000` or pure `#ffffff`
- Nocturne design system's default blurple `#9184d9` — that's scaffolding, not Drift

---

## Typography

### Typeface roles

| Context | Style | Specific fonts | Emotional anchor |
|---|---|---|---|
| UI chrome, labels, navigation, data fields | Humanist sans-serif | Inter | Clean, warm, frictionless daily use |
| Intelligence moments, display headings, comparison results, research summaries | Warm serif | **Lora** (locked 2026-09-16) | A well-printed cafe menu card — literary, timeless |
| Extracted object fields, structured data | Humanist sans-serif, slightly smaller | Inter | Precise, readable, organized |

### Rules
- Warm serif appears **only** when Drift surfaces something meaningful. It is the visual mark of intelligence. Never use it for chrome, navigation, or settings.
- Line height: 1.6 minimum. The text breathes.
- Never bold-heavy hierarchy. Warmth over aggression.
- No geometric sans (DM Sans, Futura). No decorative or script fonts.

---

## Design System Base

Mockups built on **Nocturne** design system (`design_inspirations/UI mockups for design system/_ds/`). It provides component architecture and spacing/radius tokens. Drift overrides Nocturne's dark palette entirely.

- Use Nocturne component classes: `.btn`, `.card`, `.nav`, `.dialog`, `.table`, `.tag`
- Override all color tokens with Drift's warm palette above
- Icons: Phosphor (as per Nocturne spec)
- Spacing: 8px base grid, density 0.7×
- Radius: `4px / 8px / 14px` (sm/md/lg)

---

## Motion

- **Pace:** Deliberate and confident. Under 300ms for most transitions.
- **Easing:** Always decelerate into the final state (`ease-out`). Things arrive, never snap or bounce.
- **Intelligence reveals:** 350–400ms, soft fade + 2–4px upward drift. Like morning light gradually illuminating a surface.
- **Panel retreat:** 200ms ease-in. Quiet exit, faster than arrival.
- **Ambient pulses** (memory warning, pre-crash): 2–3s ease-in-out opacity cycle. Never demanding.

### Never
- Bounce or spring physics
- Snappy cuts
- Looping animations in idle state
- Any animation demanding attention without user invitation
- Red for any state — Drift uses amber-gold for caution, never alarm colors

---

## Layout and Density

- **Idle / normal browsing:** Minimal chrome. Thinner tab bar than Chrome, quieter address bar than Safari. Web page owns the space. Drift is nearly invisible.
- **Intelligence activated:** Panels expand from right (Object Panel, Object Library) or rise from bottom (Compare Tray, memory warnings, Research nudge). Retreat when done.
- **Whitespace:** Generous by default. Density is earned by user intent.
- **Grid:** 8px base. Panel widths: 340px (Object Panel), 360px (Object Library). Padding: 24–48px in panels.

---

## Brand Voice (UI copy)

- Warm, brief, to the point. Like a good cafe waiter: brings the right thing, no speech.
- No exclamation marks. No notification-speak.
- Direct and calm even for anxiety-inducing states (memory warnings, crashes).

| Situation | Copy |
|---|---|
| Empty state | "Nothing here yet. Start browsing and Drift will find what matters." |
| Action confirmation | "Add the Sony to cart on Amazon?" |
| Memory critical | "Drift is carrying a lot right now. Closing a few tabs will keep things smooth." |
| Pre-crash | "This tab is working hard. Reload or close it to keep things smooth." |
| Research detection | "You've been researching headphones. Drift has been reading across 5 tabs. Want a feed of what's new?" |
| Local intelligence | "Drift is thinking on your device. Your data stays here." |

---

## What Drift Must Never Look Like

- Glassmorphism / frosted blur effects
- Dark-by-default base state (evening mode is opt-in, not default)
- Cluttered toolbars
- Cartoon / bubbly / rounded-to-excess shapes
- Any aesthetic that reads "AI product from 2023" (neon, purple gradients, matrix green)
- Corporate, cold, clinical

---

## Screens Built (as of 2026-09-15)

### Set 1 — Compare Mode (11 screens)
`design_inspirations/Drift - Compare Mode Mockups.html`

| # | Screen |
|---|---|
| 1 | New Tab / Home |
| 2 | Browsing a product page (normal state) |
| 3 | Object recognition moment |
| 4 | Object panel open |
| 5 | Compare initiated |
| 6 | Cross-tab selection |
| 7 | Compare view (flagship) |
| 8 | Action suggestion |
| 9 | Action confirmation |
| 10 | Evening mode (compare view) |
| 11 | Onboarding / first launch |

### Set 2 — Research Mode + Memory + USPs (9 screens)
`design_inspirations/Drift - Research and Memory Mockups.html`

| # | Screen |
|---|---|
| 1 | Research Mode: detection nudge |
| 2 | Research Mode: full timeline feed |
| 3 | Memory: approaching limit (ambient pulse) |
| 4 | Memory: critical state (warm card) |
| 5 | Pre-crash: Stage 1 (tab pulse) |
| 6 | Pre-crash: Stage 2 (bottom tray escalation) |
| 7 | Object Library panel |
| 8 | Local-first indicator hover tooltip |
| 9 | Local-first status screen |

---

## Open Design Decisions

- ~~Exact serif typeface~~ — **Lora locked 2026-09-16**
- Design system folder naming inconsistency ("UI mockups for design system" vs "UI mockups for design system2") — consolidate before handoff to engineers
