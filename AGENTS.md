# Repository Instructions

## Identity

Plywise is a **C++ chess systems project with a React web interface**.

The primary product is a web-first, open-source chess intelligence system for completed games.
Single-game analysis stays free. Browser Stockfish supplies the default compute, while C++ remains
the source of chess truth.

The existing local application is a verified reference implementation during the web migration.

## Required reading

Before product, frontend, or architecture work, read:

1. `ground-truth/00-READ-FIRST.md`
2. `ground-truth/PRODUCT_VISION.md`
3. `ground-truth/WEB_ROADMAP.md`
4. `ground-truth/WEB_ARCHITECTURE.md`
5. `ground-truth/ENGINE_BENCHMARK_PLAN.md` for engine work
6. `ground-truth/SYSTEM_ARCHITECTURE.md`
7. `ground-truth/CPP_RUNTIME.md`
8. The relevant feature spec
9. `ground-truth/DESIGN_SYSTEM.md`
10. `ground-truth/ENGINEERING_RULES.md`
11. `ground-truth/VISUAL_QA.md`

Treat `ground-truth/` as authoritative. Dated audit, review, migration, and implementation reports
are historical evidence. Never use `archive/` as current requirements.

## Product contract

- Analyze completed games only.
- Keep one-game-at-a-time analysis free.
- Let guests complete a review without creating an account.
- Use accounts for saved history, sync, and personal intelligence.
- Charge, if ever necessary, only for material cost or genuinely distinct value.
- Never collect Chess.com passwords, cookies, or private session data.
- Keep the browser extension blocked until its Chess.com boundary is authorized.
- Make every personal insight traceable to games, positions, versions, and sample size.

## Ownership boundary

C++ owns:

- completed-game import, refresh, and normalization;
- PGN, SAN, FEN, board reconstruction, and legal moves;
- canonical games and variation validation;
- review assembly and browser-engine observation validation;
- server Stockfish lifecycle, queues, workers, cancellation, and real progress;
- classification, openings, patterns, personal intelligence, and practice selection;
- persistence contracts, version compatibility, and authorization decisions;
- future coaching-provider interfaces.

React owns:

- layout, navigation, rendering, and board interaction intent;
- guest and account presentation;
- browser Stockfish worker lifecycle;
- progress, settings, charts, and educational UI;
- accessibility, themes, keyboard controls, and responsive behavior;
- transient UI state that does not define chess truth.

Never duplicate move legality, classification, opening truth, pattern truth, or player-profile truth
in TypeScript. Browser engine output is untrusted until C++ validates and processes it.

## Existing behavior to protect

Preserve the working flow:

- Import a completed Chess.com game URL or PGN.
- Show the canonical game and playthrough.
- Launch analysis explicitly.
- Show real progress.
- Navigate moves and reveal a best move.
- Retry a mistake.
- Create and extend legal variations.
- Return to canonical review.
- Recover persisted work after restart.

## Product defaults

- Home is the launch view.
- Guest analysis is a complete first-run path.
- Analysis begins only after explicit user action.
- Browser Quick analysis is the free compatibility default once verified.
- Progress reflects actual stages, never a timer.
- Analysis is board-first with a contextual inspector.
- Engine detail is progressively disclosed.
- Variations never edit canonical games.
- Theme follows the browser/system preference by default.
- Accent remains navy blue.
- Explanations are deterministic now, with a future optional coaching-provider boundary.

## Design constraints

The interface should feel calm, intelligent, premium, and workstation-like while working on desktop
and mobile web.

Do not produce:

- a ChatGPT clone;
- a generic AI dashboard or card farm;
- fake progress or fake intelligence;
- excessive gradients, glass, glow, pills, or giant whitespace;
- copied Chess.com layouts, icons, sounds, board palette, copy, or classification glyphs;
- live-game assistance;
- a sweeping frontend rewrite that breaks C++ behavior.

## Working method

1. Resolve the GitHub issue and acceptance criteria.
2. Inspect the repository and current uncommitted work.
3. Start from the latest remote default branch.
4. Create a focused `codex/` feature branch.
5. Run the relevant current tests before changing behavior.
6. Implement one working vertical slice.
7. Protect the C++/React boundary with typed contracts.
8. Run tests, security checks, and real-browser verification in proportion to risk.
9. Inspect and stage only the intended diff.
10. Push the branch and open a draft PR for user review.
11. Report files, contracts, tests, screenshots, risks, and the next issue.

## Git rules

- Create a new branch for every coherent feature.
- Open a PR after implementation and testing.
- Do not merge the PR; the user acts as engineering manager.
- Do not delete or overwrite unrelated uncommitted work.
- Do not use broad staging in a mixed worktree.
- Keep `archive/` ignored and local.
- Use issues, milestones, and labels to keep the six-phase roadmap visible.
