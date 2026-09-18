# Drift Browser — Capability Universe

Legend:
- **TS** = table stakes
- **TR** = trust/reliability
- **DIFF** = differentiator
- **MOAT** = intended moat
- **V0** = candidate for commercially usable MLP
- **V1** = scale version
- **Later** = post-V1

## A. Browser Core

| Capability | Class | Target |
|---|---|---|
| Chromium rendering compatibility | TS | V0 |
| Chrome-extension compatibility | TS | V0 |
| Address/search bar | TS | V0 |
| Bookmarks | TS | V0 |
| History | TS | V0 |
| Downloads | TS | V0 |
| Password/autofill integration | TS | V0 |
| Private/incognito mode | TS | V0 |
| Profiles | TS | V0/V1 |
| Pinned tabs | TS | V0 |
| Tab groups | TS | V0 |
| Horizontal tabs | TS | V0 |
| optional vertical tabs | TS | V1 |
| find in page | TS | V0 |
| page zoom | TS | V0 |
| PDF viewing | TS | V0 |
| media/PiP | TS | V1 |
| translation | TS | V1 |
| DevTools | TS | V0 |
| default search engine selection | TS | V0 |
| site permissions | TS | V0 |
| certificate/security indicators | TS | V0 |
| sync | TS | V1 |
| Chrome import | TS | V0 |
| crash restore | TR | V0 |

## B. Resource Intelligence

| Capability | Class | Target |
|---|---|---|
| Active tab state | TR | V0 |
| Warm tab state | TR | V0 |
| Cold/frozen tab state | TR | V0 |
| state visual encoding | TR | V0 |
| memory-pressure detection | TR | V0 |
| subtle memory warning | TR | V0 |
| automatic safe freezing | TR | V0 |
| tab-level RAM estimate | TR | V0/V1 |
| browser RAM dashboard | TR | V0 |
| heaviest-tab view | TR | V0 |
| recoverable-memory estimate | TR | V1 |
| crash-risk prediction | DIFF | V1 after validation |
| session-specific archive | TR | V0 |
| ML archive recommendation | DIFF | V1 |
| never auto-archive | principle | V0 |
| session restoration checkpointing | TR | V1 |
| scroll-position recovery | TR | V1 |
| form-state recovery where feasible | TR | Later |

## C. Object Understanding

| Capability | Class | Target |
|---|---|---|
| DOM object extraction | MOAT | V0 |
| accessibility-tree understanding | MOAT | V0 |
| schema.org / structured-data parsing | MOAT | V0 |
| visible-region semantic object detection | MOAT | V0 |
| text block detection | MOAT | V0 |
| product-card detection | MOAT | V0 |
| table detection | MOAT | V0 |
| article-section detection | MOAT | V0 |
| image detection | MOAT | V0 |
| code-block detection | MOAT | V1 |
| chart detection | MOAT | V1 |
| form detection | MOAT | V1 |
| video object detection | MOAT | V1 |
| company/person/entity detection | MOAT | V1 |
| local object classifier | MOAT | V0 |
| lightweight local semantic model | MOAT | V0/V1 |
| multimodal fallback | MOAT | V1 |
| object confidence | MOAT | V0 |
| object metadata schema | MOAT | V0 |

## D. Object Interaction

| Capability | Class | Target |
|---|---|---|
| explicit object picker | MOAT | V0 |
| modifier-click selection | MOAT | V0 |
| subtle mode outlines | DIFF | V0 |
| object contextual palette | MOAT | V0 |
| universal verbs: Ask/Compare/Extract/Save/Search/Act | MOAT | V0/V1 |
| temporary selected-object state | MOAT | V0 |
| multi-select in one page | MOAT | V0 |
| cross-tab selection | MOAT | V0 |
| object tray | MOAT | V0 |
| object drag into tray | MOAT | V0 |
| keyboard commands for object actions | DIFF | V0 |
| object preview / inspector | MOAT | V0/V1 |
| object metadata view | MOAT | V1 |

## E. Structured Extraction & Identity

| Capability | Class | Target |
|---|---|---|
| extract object fields | MOAT | V0 |
| normalize comparable products | MOAT | V0 |
| normalize tables across sites | MOAT | V1 |
| infer missing fields | DIFF | V1 |
| cross-site same-object matching | MOAT | V1 |
| product-model identity | MOAT | V0/V1 |
| company/entity identity | MOAT | V1 |
| source provenance for extracted fields | MOAT | V0 |
| confidence per field | MOAT | V1 |
| export CSV/Markdown | DIFF | V0 |
| copy structured table | DIFF | V0 |

## F. Compare Mode

| Capability | Class | Target |
|---|---|---|
| explicit Compare Mode | DIFF | V0 |
| compare selected objects | MOAT | V0 |
| suggested common fields | MOAT | V0 |
| structured comparison tab | MOAT | V0 |
| missing-data detection | DIFF | V0/V1 |
| search missing fields | DIFF | V1 |
| filters/sort | DIFF | V0 |
| save comparison | DIFF | V1 |
| price/spec/review actions | DIFF | V0/V1 |
| user-defined comparison fields | DIFF | V1 |

## G. Research Mode

| Capability | Class | Target |
|---|---|---|
| explicit Research Mode | DIFF | V0/V1 |
| collect passages/sources | MOAT | V0/V1 |
| source tray | MOAT | V0/V1 |
| semantic duplicate detection | DIFF | V1 |
| "what is new" highlighting | DIFF | V1 |
| contradiction detection | DIFF | V1 |
| evidence comparison | DIFF | V1 |
| source provenance | MOAT | V0/V1 |
| research export | DIFF | V1 |
| citation-oriented output | DIFF | V1 |

## H. Auto Pilot

| Capability | Class | Target |
|---|---|---|
| Auto Pilot as explicit mode | DIFF | V1 |
| local mode inference | DIFF | V1 |
| confidence thresholds | DIFF | V1 |
| mode-change indicator | DIFF | V1 |
| user correction | DIFF | V1 |
| local preference learning | DIFF | Later |
| internally composable modes | DIFF | Later |

## I. Ask / Reasoning

| Capability | Class | Target |
|---|---|---|
| ask about selected object | TS/DIFF | V0 |
| ask about selected objects | DIFF | V0 |
| ask about selected tabs | TS | V1 |
| summarize object set | DIFF | V0 |
| cloud deep synthesis | TS | V0 |
| local lightweight reasoning | DIFF | V0/V1 |
| visible context scope | TR | V0 |

## J. Action Engine

| Capability | Class | Target |
|---|---|---|
| browser-local reversible actions | MOAT | V0 |
| open/focus selected result | MOAT | V0 |
| reorganize tabs | DIFF | V0 |
| close duplicates | DIFF | V1 |
| batch download selected links/docs | MOAT | V0/V1 |
| search selected object elsewhere | MOAT | V0 |
| autofill selected form | MOAT | V1 |
| navigate website | TS in agentic category | V1 |
| execute external website action | DIFF | V1 |
| explicit approval for send/submit/purchase/delete | TR | V1 |
| action progress indicator | TR | V0 |
| interrupt/take control | TR | V1 |
| action history/audit | TR | V1 |
| composable action pipeline | MOAT | V1 |

## K. Information Novelty

| Capability | Class | Target |
|---|---|---|
| URL duplicate detection | TS | V0 |
| semantic duplicate detection | DIFF | V1 |
| duplicate passage detection | DIFF | V1 |
| "new relative to selected sources" | MOAT candidate | V1 |
| "new relative to read history" | MOAT candidate | Later |
| novelty confidence | DIFF | V1 |

## L. Live Objects & Memory

| Capability | Class | Target |
|---|---|---|
| save object snapshot | DIFF | V1 |
| live object | DIFF | V1/Later |
| refresh live object | DIFF | Later |
| browser remembers prior comparison | DIFF | Later |
| local object memory | DIFF | V1 |
| delete memory | TR | V1 |
| sensitive-site exclusions | TR | V1 |
| object history | DIFF | V1 |

## M. Extensibility

| Capability | Class | Target |
|---|---|---|
| Chrome extensions | TS | V0 |
| Drift action API | MOAT | Later |
| custom actions | DIFF | Later |
| custom modes | DIFF | Later |
| skills marketplace | DIFF | Later |
| MCP server | TS/strategic | Later |
| browser CLI | strategic | Later |
| external agent access | strategic | Later |

## N. Privacy / Security

| Capability | Class | Target |
|---|---|---|
| local-first inference | TR/DIFF | V0 |
| context-scope indicator | TR | V0 |
| site permission controls | TS | V0 |
| sensitive-site AI exclusion | TR | V0 |
| local encrypted object cache | TR | V0 |
| explicit cloud-send boundary | TR | V0 |
| action permission tiers | TR | V1 |
| consequential-action confirmation | TR | V1 |
| telemetry controls | TR | V0 |
| incognito isolation | TS | V0 |

## O. Monetization

| Capability | Class | Target |
|---|---|---|
| free core browser | GTM | V0 |
| Pro cloud intelligence limits | GTM | V0 |
| paid advanced comparison/research quotas | GTM | V0/V1 |
| premium local models if licensing requires | GTM | V1 |
| enterprise tier | GTM | Later |

## P. Explicit Non-Goals for V0/V1
- email client
- VPN
- Notion-style workspace
- radical replacement of tabs
- enterprise browser governance
- always-on agent
- automatic consequential actions
- custom mode builder
- model marketplace
- full operating-system concept
