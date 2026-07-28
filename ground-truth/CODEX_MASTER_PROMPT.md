# Codex Master Prompt

Run this from the repository root.

---

You are evolving Plywise into a web-first, open-source chess intelligence system. Preserve the
working local application while building the hosted path incrementally.

## Read first

Read:

1. root `AGENTS.md`;
2. `ground-truth/00-READ-FIRST.md`;
3. `ground-truth/PRODUCT_VISION.md`;
4. `ground-truth/WEB_ROADMAP.md`;
5. `ground-truth/WEB_ARCHITECTURE.md`;
6. `ground-truth/ENGINE_BENCHMARK_PLAN.md` for engine work;
7. `ground-truth/SYSTEM_ARCHITECTURE.md`;
8. the relevant feature specification;
9. `ground-truth/DESIGN_SYSTEM.md`;
10. `ground-truth/ENGINEERING_RULES.md`;
11. `ground-truth/VISUAL_QA.md`.

Inspect the real branch, status, code, API contracts, tests, and current app before editing.

## Product contract

- Completed single-game analysis stays free.
- The website is the primary product.
- Guest analysis does not require an account.
- Accounts add saving, synchronization, and personal intelligence.
- Payment, if ever introduced, funds real cost or differentiated value.
- Live-game assistance is prohibited.
- The extension remains blocked until its Chess.com integration is authorized.

## Preserve the C++ boundary

C++ owns import normalization, PGN/SAN/FEN, legality, review assembly, classification, opening
recognition, pattern detection, variations, player modeling, and server engine orchestration.

React owns layout, rendering, browser-worker lifecycle, accessibility, and user intent.

Browser Stockfish output is untrusted input. It becomes a Plywise review only after typed C++
validation and domain processing.

Do not create a TypeScript chess engine, generic AI dashboard, chat interface, fake progress, or
unversioned client-side product truth.

## Delivery workflow

1. Resolve the issue and acceptance criteria.
2. Confirm the latest remote default branch.
3. Create a focused `codex/` feature branch.
4. Record existing uncommitted work.
5. Implement one working vertical slice.
6. Run proportional tests and real-browser QA.
7. Inspect the complete diff.
8. Commit and push only the intended scope.
9. Open a draft PR for user review.

Never merge the PR on the user's behalf.

## Current sequencing

### Phase 0

Product definition, architecture, benchmarks, policy, licensing, backlog, and web migration seams.

### Phase 1

Deployable React frontend, hosted C++ service, PostgreSQL repository boundary, authentication,
tenant isolation, and one saved online review.

### Phase 2

Free guest analysis, browser Stockfish, versioned result submission, account history, privacy
controls, and private-alpha verification.

Later phases add personal intelligence, an authorized browser companion, sustainable optional
features, and public hardening. Follow `WEB_ROADMAP.md`.

## Verification

Protect the existing local flow:

completed-game import → explicit Analyze → real progress → completed review → navigation → best
move → retry → legal variation → canonical return → restart persistence.

For web work, additionally verify:

- guest and account flows;
- cross-account denial;
- browser/server review parity;
- idempotent import and submission;
- cancellation and stale-job handling;
- export and deletion;
- both themes and supported widths;
- keyboard and accessibility;
- no live-game analysis path.

## Reporting

Every PR reports:

- issue and user outcome;
- files and contracts changed;
- C++/React ownership decisions;
- tests and browser evidence;
- security and privacy impact;
- performance or cost evidence;
- gaps, blockers, and next issue.

---
