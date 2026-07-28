# Read First

## Product in one sentence

Plywise is a web-first, open-source chess intelligence system that keeps completed single-game
analysis free and turns a player's history into evidence-backed review and practice.

## Product principles

1. Completed single-game analysis stays free.
2. The website is the primary product.
3. Guest analysis does not require an account.
4. Accounts add saved history, synchronization, and personalization.
5. Payment, if introduced, funds costly compute or genuinely distinct intelligence—not basic
   access to a game review.
6. C++ remains the source of chess truth.
7. React renders structured state and sends user intent.
8. Browser Stockfish is the default free-compute path.
9. Server Stockfish is optional for deeper, batch, or otherwise costly work.
10. Imported games are immutable; variations are separate branches.
11. Analysis starts only after a completed game and an explicit user action.
12. Every insight must be traceable to games, positions, versions, and sample size.
13. Chess.com passwords, session cookies, and private account data are never collected.
14. Historical reports remain dated evidence and are not rewritten as current verification.

## Product areas

### Free analysis

Import a completed game, choose an engine profile, observe real progress, review every move, retry a
mistake, and explore legal variations.

### Personal intelligence

Connect reviews across time to identify recurring weaknesses, opening habits, game-phase problems,
and positions that are worth practicing.

### Practice

Build a small, evidence-backed queue from the player's own games. Track attempts and improvement
without fabricating mastery from weak samples.

### Accounts and sync

Accounts save games, analyses, settings, and training history across devices. Guest mode remains a
complete path for one-off analysis.

### Browser companion

A future extension may offer a user-initiated Plywise review after a completed game. It remains
blocked until the Chess.com integration boundary is clarified in writing.

## Canonical web plan

- `WEB_ROADMAP.md`: phases, deliverables, and exit criteria.
- `WEB_ARCHITECTURE.md`: browser, C++, persistence, identity, and engine boundaries.
- `ENGINE_BENCHMARK_PLAN.md`: browser/native comparison and profile-selection evidence.
- `PRODUCT_VISION.md`: users, promise, differentiation, and success.
- `SYSTEM_ARCHITECTURE.md`: enduring ownership and service layers.
- `ENGINEERING_RULES.md`: implementation, testing, privacy, and Git expectations.

## Current phase

Phase 0: define the web product, document the architecture, benchmark engine choices, establish the
GitHub backlog, clarify licensing, and prepare the Chess.com authorization request.

The local application remains the verified reference implementation while the web vertical slice is
built incrementally.
