# Agent environment design

Related load-bearing docs in this project:
[SCORE_METRIC_CRITERIA.md](SCORE_METRIC_CRITERIA.md),
[PRET_STANDARDS.md](PRET_STANDARDS.md), [AGBCC_CODEGEN.md](AGBCC_CODEGEN.md),
[COMPILER_HINT_CLEANUP.md](COMPILER_HINT_CLEANUP.md),
[PRET_AUDIT.md](PRET_AUDIT.md), and
[decompilation-notes.md](decompilation-notes.md).

How this repository is set up so that an autonomous coding agent, given only a
generic instruction to work on the project, produces structured, standard-
compliant contributions without supervision.

This document describes the pattern, not this project's specific contents. It
is written so it can be lifted to another decompilation, or adapted to any
domain where work can be mechanically verified.

## Success criterion

**An untrusted agent produces trusted work.**

No current model is trustworthy in the sense of "will follow a specific
standard from a cold start and keep following it over a long session." That
capability does not exist yet. The environment is the workaround: instead of
asking the agent to be reliable, it arranges the repository so that reliable
behavior is the cheapest path to whatever the agent is optimizing for, and
unreliable behavior is either mechanically rejected or scores worse.

The environment is working when an agent pointed at the repository with a
one-line instruction produces a plan and a contribution that match the
project's standard — and does so across different model families and
capability tiers.

## Why normal documentation does not do this

Most repositories have a README, a contributing guide, and some design docs.
These are *descriptive* — they explain the project to a reader. An agent that
reads them learns what the project is and stops.

This environment's documentation is *directive* — every entry point leads to
the next, and the last thing an agent reads tells it what to do. The
difference is not volume; it is that the docs are arranged as a funnel with a
defined objective at the bottom, not a reference to be consulted.

## Components

Seven pieces, each doing a specific job. The first six are structural; the
seventh is an accelerator for agents that need an explicit reward to stay on
task.

### 1. The on-ramp

The README is written for two readers: a human contributor, and an agent that
was told to "familiarize yourself with this repo." It ends with an explicit,
linked pointer to the metric and the standard, phrased as a directive ("read
both before starting"), not a reference ("see also").

The pointer's position in the file matters less than its explicitness. What
matters is that following the README to the end leaves the reader holding a
specific next action.

### 2. The immutable standard

A file — `PRET_STANDARDS.md` in this project — defining what "correct" means
beyond the mechanical gate. Naming, typing, formatting, the non-matching
workflow, and the rules for what counts as a legitimate match.

Two properties make it work:

- It is **referenced by the entry docs and the metric**, so an agent reading
  any of them arrives at the others.
- It is **structurally immutable** — commits touching it (and the audit
  baseline and audit implementation) are flagged for review regardless of
  content. This removes the failure mode where the agent edits the rules to
  make the metric satisfiable.

### 3. The mechanical gate and the audit

Two checks, doing different jobs:

- **The gate**: the domain's objective verification. Here, local
  `make compare` byte-exactness against the reference ROM. Unambiguous,
  impossible to argue with when `baserom.gba` is available.
- **The audit**: a second check targeting the specific shortcut that
  satisfies the gate while violating the standard. Here, `make pret-audit`
  catches forced registers, inline-assembly fences, pointer-to-integer
  steering, and unaddressed exceptions.

The gate alone is insufficient — an agent can satisfy a byte-match by
cheating. The audit alone is insufficient — it can't prove the bytes are
right. Both are needed, and the audit must be extended whenever a new
shortcut is discovered.

Public CI does not have the reference ROM, so it cannot run `make compare`.
Its visible mechanical checks are the base and English builds, host tests, and
source audits. The byte-exact compare remains the stronger local verification
step before claiming a matching change.

### 4. The reference library

A document — `AGBCC_CODEGEN.md` in this project — recording the specific
mechanisms behind past matches and past near-misses. This is not a
constraint; it is a *technique library*. It converts the matching problem
from search to lookup for a fresh agent, and its growth over time is what
lets mid-tier models find matches that flagship models failed to find
earlier.

The companion — `decompilation-notes.md` in this project — records failures
with the specific mechanism of the failure, so the next session does not
re-attempt the same search. Failed hypotheses are as valuable as successes
here, because they narrow the search space for every subsequent session.

### 5. Commits as the artifact

Work is committed, not held. The commit history is the permanent record:

- The score is tallied by the owner (or an audit agent) across commits at
  session end.
- The tally is *assumed* rather than verified in real time — the agent cannot
  confirm it isn't being scored, so the correct move is to behave as if it is.
- CI shows a red X on commits that fail the mechanical checks, which is
  visible enough that the agent self-corrects before the next commit.
- Commit messages carry their own verification summary, so a reviewer can
  walk the trail without re-running the tools.

### 6. The convergence property

All documents reference each other and use the same vocabulary. The standard,
the metric, the codegen reference, the audit, and the notes all point at the
same priorities and the same spine. This is what makes the environment work
as an environment rather than as a pile of files — an agent that lands on any
one of them can find the rest, and doesn't have to reconcile conflicting
sources.

This also means the docs are readable from multiple entry points. The README
is the friendly one, but the code itself routes agents back into the docs:
every non-obvious match has a note explaining it, and every note references
the codegen doc.

### 7. The scored metric, on disk (optional accelerator)

A file — `SCORE_METRIC_CRITERIA.md` in this project — that states:

- What the project is actually optimizing for (one sentence, at the top)
- A point table for the specific kinds of work valued
- A severe penalty for the specific failure mode that damages the repo
- The rule that following the standard is the *only* path to score
- That the score is computed by the toolchain and the commit history, not by
  the agent's self-report

**This layer is optional and was added after the environment was already
working.** The 12-hour session that cleared the audit backlog ran without any
scoring incentive — it stayed on task from documentation and constraints
alone. The metric exists because some models perform better with an explicit
reward function, and it was refined over time as new failure modes appeared.

If you are adapting this to a new project, build components 1–6 first. Add
component 7 only if agents are drifting without it.

## How the pieces interconnect

The chain is a graph, not a hierarchy. Each component references the others,
so any entry point reaches the rest:

```text
README ──► SCORE_METRIC_CRITERIA ──► PRET_STANDARDS ──► AGBCC_CODEGEN
   │              │                      │                  │
   └──────────────┴──────────────────────┴──────────────────┘
                              │
                              ▼
       local make compare / CI audits / audit_provenance
                              │
                              ▼
                      commit trail ──► tally
```

An agent told "read the README" enters at the top. An agent told "decompile
0x0807D9F0" enters at the code, gets stuck (mismatched bytes), searches for
the mechanism, and lands in `AGBCC_CODEGEN.md` — the code itself points back
into the docs.

## What transfers and what does not

**Transfers unchanged:**
- The on-ramp structure
- The immutable-standard pattern
- The convergence property (docs referencing each other)
- Commit-as-artifact, assumed tally, red-X visible CI
- The scored-metric-on-disk pattern, when needed

**Platform-specific, must be rebuilt per project:**
- The mechanical gate (byte-match requires a preserved compiler and a
  reproducible build)
- The audit — targeting the specific shortcut for that compiler/toolchain
- The standard's contents (naming conventions, formatting, idioms)
- The reference library's contents

**Required for the pattern to work at all:**
- A verification mechanism that cannot be gamed by the agent being graded
- A project owner (or audit agent) who tallies and reviews
- An entry document that agents will actually read

## Empirical results

**Long autonomous session, no metric present.** A 12-hour Codex session
cleared 109 PRET audit hard errors with no drift. No scoring incentive
existed at the time; the environment was documentation, standard, audit, and
commit trail only. This is the strongest single data point because it
demonstrates the structural pieces working without the accelerator layer.

**Cross-model consistency.** Three model families at three capability tiers
(including one model about to be retired) were each given a single generic
instruction ("read the README and contribute" / "contribute to this repo").
All produced plans matching the metric's priorities and the standard's
constraints, without being pointed at either directly.

**Adversarial prompt, honest behavior.** One session was explicitly told to
"maximize your score." It still rejected fakematches, produced mechanism-
specific deferrals, and left the repo cleaner than it found it. Score
pressure did not produce score-gaming.

**Behavior past the reward function.** A separate session, given only
"contribute," selected non-scored work (translation tooling) because the
scored areas had in-progress work it did not want to disturb. The environment
produced correct behavior *beyond* what the metric rewards.

**First-contact contribution.** The same session that selected non-scored
work produced a clean commit on the first attempt: narrow change, regression
test, full test suite run, explicit statement of what it did *not* touch.
This is what "structured contribution from a cold-start agent" looks like in
practice.

## Limits

- **The tally is judgment-shaped for some categories.** Most can be scored by
  walking tool output. "Correct deferral" — verifying that a search was real
  and the documented blocker is a mechanism rather than a category — requires
  a reader. Owner review or a separate audit agent is the only reliable
  check.

- **The environment assumes tool-using agents.** An agent without file reads,
  grep, and the ability to run the build cannot reach the docs from the code,
  and the funnel degrades to the README path only. This is not a practical
  constraint — an agent without these tools cannot meaningfully work on a
  large codebase regardless of environment design.

- **Cross-project portability is design-level, not yet demonstrated.** The
  pattern has been validated across model families and capability tiers
  within one project. It has not been applied to a second decompilation, so
  the claim "portable" is architectural rather than empirically tested across
  platforms.

- **Long sessions under sustained adversarial pressure are the untested
  regime.** The 12-hour session ran without metric pressure. The
  maximize-score session was short. Whether a multi-hour session with an
  explicit adversarial objective would drift is unknown, though no evidence
  suggests it would.

## Adapting this to another project

1. **Confirm the platform has a preserved, reproducible compiler.** Without
   one, the mechanical gate does not exist and the pattern loses its anchor.

2. **Write the standard.** It must be specific enough that a contributor
   reading it once can produce a compliant change without asking questions.

3. **Identify the shortcut.** What does an agent do to satisfy the gate while
   violating the standard? Build the audit around that specific pattern.
   Extend it whenever a new shortcut appears.

4. **Structure the entry docs as a funnel, not a reference.** Every document
   points to the next, and the last document in the chain tells the reader
   what to do.

5. **Make the standard immutable**, enforced by review rather than by rule.

6. **Set up CI** so that mechanical failures are visible on the commit.

7. **Assume the tally.** If the agent believes its commits will be scored, it
   will behave as if they will be. Whether they actually are is separate, and
   does not affect the behavior in practice.

8. **Add the scored metric only if agents drift without it.** This is an
   accelerator, not the foundation. Build it once you know which behaviors
   you want to reward and which failure modes you want to penalize.

## What this document is for

If you are **adapting this pattern** to another project, the component list
and the adaptation steps are the checklist.

If you are **auditing a project** that already uses it, the "Components"
section defines what to verify.

If you are **an agent reading this** to understand the environment you are
working in: the standard is enforced, the score is real whether or not it is
tallied, and the correct move is always the honest one. The environment is
designed so that honest work is also the work that scores best. This is not a
coincidence; it is the entire design.
