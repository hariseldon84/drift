# Ground Rules

**These govern every session. They override convenience, shortcuts, and any
instruction that conflicts with them.** Read this file first, every session,
before anything else. Anand maintains it; treat an edit here as an immediate
change to how sessions behave.

This file is deliberately short. If it grows past ~150 lines, compact it.

IMPORTANT : Some of these documents like idea.md might not be idea.md exactly but something like idea_****.md or so and they all might be in a docs folder instead of root. We will keep them in docs folder only and update everything there. However tasks.md and bugs.md needs to stay at root level !
---

## 0. Follow these rules — always

Not "when convenient", not "unless the task is small". If a rule is wrong,
say so and ask — do not quietly skip it. Silently ignoring a rule is worse
than arguing with it.

Before ending a turn, check: did anything I just did need a `tasks.md` entry,
a `bugs2.md` entry, a `PRD.md` update, or a ship report? If yes, do it now.

---

## 1. Canonical files

| File | Holds |
|---|---|
| `status_quo.md` | **Where the project is right now** — the anchor. See §7. |
| `PRD.md` | Product requirements + the *reasoning* behind decisions |
| `tasks.md` | Every task, pointed and short. Detail lives in `PRD.md` |
| `design_specification.md` | Design source of truth |
| `bugs3.md` | Bug log (current). `bugs.md` and `bugs2.md` are frozen — never add to them |
| `commands.md` | Every operational command, in full runnable form |
| `CLAUDE.md` | Architecture, stack, locked decisions |
| `groundrules.md` | This file |
| `stale/` | Superseded material. **Do not read unless the user names a file in it** |

Everything else lives in `docs/`.

---

## 2. Decisions land in PRD + tasks the same turn

A decision finalised in chat, a grilling session, or a brainstorm gets written
into **both** `PRD.md` and `tasks.md` before the turn ends. A decision not in
those files did not happen.

Record the **reasoning**, not just the conclusion. The next agent needs to know
why, or it will re-litigate.

## 3. No work starts unless it is in tasks.md

If a task is not listed, add it first — then do it. This applies to small fixes
too. Update its status the moment it is done, not at end of session.

## 4. tasks.md stays compact

One line per task. Details, trade-offs and rationale go in `PRD.md`; the task
line points there. Completed work moves to a `## Done` section — not deleted,
not left mixed in with pending work.

## 5. commands.md documents every command

Full, runnable form. Never assume the user knows one. Never assume port 3000 is
free — always show how to pick another. New npm script, new deploy step, new env
var: added the same session it appears.

## 6. Bugs go in the current bug log

Currently `bugs3.md`. Categorised by epic. Never delete or overwrite an entry —
update its status in place (open → investigating → fixed, with the fix noted).
Past ~3000 words, roll to the next numbered file and freeze the prior one,
referenced from the new one. Numbering continues across files; it does not
restart. (`bugs.md` → `bugs2.md` frozen 2026-09-03 → `bugs3.md`.)

---

## 7. status_quo.md is the anchor

**If it does not exist, create it.** One page, rewritten (not appended) whenever
state changes materially:

- What works, what is half-built, what is broken
- What is in flight and who/what is doing it
- Open decisions waiting on the user
- The single most useful next action

Every session starts by reading it and ends by updating it. It is the file that
makes a cold start cheap.

---

## 8. Large or vague requests get split before they get started

When a request is open-ended — *"review everything"*, *"check the whole repo"*,
*"fix all the bugs"*, *"optimise the app"* — **do not just start.** Say plainly:

> This will consume a large amount of context and tokens, and quality drops as
> the window fills. Better to do one at a time.

Then **write the full request out as a checklist in a temporary markdown file**
(e.g. `_wip_<topic>.md`), agree the order, and work through it one item at a
time — updating that file as each lands. Delete it when the work is done.

This protects the work, not the budget: an agent 80% through its context makes
worse decisions than one at 20%, and a half-finished sweep with no record of
what was covered is worse than not starting.

---

## 9. Ship reports are not optional

When anything ships, state: **what changed**, **how to test it manually** (exact
steps), and **what to expect** — happy path plus known limits. Never just
"done".

## 10. Verify before claiming

A fix is not fixed until it has been **run**. Not read, not reasoned about —
executed, with the output shown. If you did not verify something, say so.

*(Learned the expensive way: enrolment was reported fixed twice by two different
agents while still returning HTTP 500 on every call.)*

---

## 11. `/lightmode` — token-conserving mode

When the user invokes `/lightmode`, or when context is over ~60% used:

- Read `status_quo.md` instead of re-reading `PRD.md` and `tasks.md` in full
- Use `rtk` wrappers and targeted `grep`/`sed -n` ranges — never dump whole files
- Summarise tool output rather than pasting it
- Skip proactive exploration; ask before broad searches
- Prefer one precise command over three exploratory ones
- Keep replies short — findings and decisions, not narration
- Still non-negotiable: §2 (PRD+tasks), §3 (tasks first), §9 (ship report),
  §10 (verify). Lightmode saves tokens, never rigour.

Say when you enter and leave it.

---

## Maintenance

New rules apply from the moment they are added — same session, not the next
one. Periodically retrospect: a rule nobody has followed in a month is either
wrong or unclear. Fix it or cut it.
