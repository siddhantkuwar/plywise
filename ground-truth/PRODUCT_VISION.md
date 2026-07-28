# Product Vision

## Definition

Plywise is a personal chess intelligence system for completed games.

It gives anyone a free, understandable Stockfish review without requiring a desktop installation or
subscription. Over time, an optional account turns separate reviews into a model of the player's
habits, strengths, weaknesses, openings, and pattern knowledge.

It is not merely an evaluation bar, a chatbot with a knight icon, or a replacement place to play
chess.

## Users

- Players frustrated by limits around basic post-game analysis.
- Beginners who need conclusions before engine notation.
- Improving players who want patterns across games, not isolated verdicts.
- Club players and streamers who want engine control and reproducible evidence.
- Developers evaluating a serious open-source C++ and web systems project.

## Promise

A user can:

- Analyze one completed game at a time for free.
- Start as a guest and create an account only when saving becomes useful.
- Import a public completed-game link or paste PGN.
- Choose a clear engine profile without learning every UCI option.
- Review classifications, opening, evaluation, accuracy, and best continuations.
- Retry a mistake before revealing the answer.
- Branch from a historical position without changing the canonical game.
- Return later to saved reviews from another device.
- See recurring evidence across games.
- Practice positions selected from their own mistakes.
- Export their data and delete their account.

## Differentiation

Raw Stockfish analysis is widely available and is not the moat.

Plywise becomes useful when it can say, with linked evidence:

- which mistakes recur;
- when in the game they tend to happen;
- whether practice is transferring into later games;
- what the player should focus on next;
- how confident the system is given sample size and recency.

Every insight must remain deterministic, inspectable, and versioned. A future language model may
improve wording or lesson sequencing, but never defines legality, evaluation, history, or evidence.

## Product model

### Always free

- One completed game analyzed at a time.
- Guest Quick and Balanced browser analysis when the device supports it.
- Full board-first review, best moves, retry, and variation.
- Honest engine and classifier metadata.

### Account features

- Saved history and cross-device sync.
- Connected public chess profile.
- Personal intelligence, progress, and practice history.
- Export, deletion, and privacy controls.

### Possible sustainable features

If paid features are ever introduced, they must correspond to material cost or distinct value, such
as deep server analysis, large batch queues, advanced longitudinal intelligence, or expensive
coaching infrastructure. Basic completed-game review is not placed behind that boundary.

## Non-goals

- Online chess play.
- Live-game assistance.
- Social feeds or generic community features.
- Collecting Chess.com credentials or session data.
- Copying Chess.com's design, language, assets, classifications, or proprietary algorithms.
- A cloud LLM as the source of chess truth.
- Making users create an account before their first review.
- Rewriting the C++ domain in TypeScript.

## Success

- A new user completes a useful review without instructions or account creation.
- Analysis never starts for a game in progress.
- A returning user receives a useful insight unavailable from a single isolated review.
- Every recommendation links to real games and positions.
- Guest compute remains economically sustainable.
- Account data remains isolated, exportable, and deletable.
- The web product preserves the verified C++ analysis behavior.
