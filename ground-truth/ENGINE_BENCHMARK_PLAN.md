# Browser Engine Benchmark Plan

## Purpose

Choose evidence-backed Quick and Balanced browser analysis profiles before making browser Stockfish
the free default.

This plan does not assume that maximum depth is the best user experience. It measures review quality,
time, compatibility, download size, CPU use, memory, and cancellation.

## Implementations to compare

- Current native C++/Stockfish reference pipeline.
- Stockfish.js lite single-threaded.
- Stockfish.js lite multithreaded when cross-origin isolation is available.
- A full browser build only if its download and startup cost remain credible.
- Hosted native Stockfish as the Deep reference.

Record exact source revision, distributed license files, binary hash, and engine-reported version.

## Fixtures

Use committed or redistributable fixtures only:

- short tactical game;
- ordinary 35–45 move rapid game;
- quiet positional game;
- long endgame;
- malformed and unsupported PGN controls;
- positions with mate scores, promotions, castling, en passant, and repeated positions.

Do not use private user games as benchmark fixtures.

## Device and browser matrix

- Current Apple silicon development Mac.
- Lower-powered laptop when available.
- Current Chrome, Firefox, and Safari.
- Current iOS Safari and Android Chrome when available.
- Single-thread fallback everywhere.
- Multithread path only when capability and headers are present.

## Candidate profiles

Initial hypotheses, not product defaults:

### Quick

- Lite single-threaded build.
- One principal variation.
- Conservative fixed depth or time budget.
- Analyze all required positions with an aggressive cache.

### Balanced

- Benchmark-selected lite or full build.
- One or limited MultiPV.
- Higher bounded depth/time.
- Optional additional thread only when evidence justifies it.

### Deep reference

- Native hosted Stockfish.
- Stronger search and optional MultiPV.
- Used to compare classification stability, not to force exact centipawn equality.

## Measurements

For every run record:

- game length and positions evaluated;
- engine build and profile;
- download and startup time;
- wall time and engine time;
- nodes and achieved depth;
- peak worker memory when observable;
- browser responsiveness;
- cancellation latency;
- worker restart behavior;
- review completion rate;
- best-move agreement with the Deep reference;
- classification agreement and meaningful disagreements;
- cache hits and repeated-run speed;
- device temperature/battery observations when practical.

## Quality rules

- Do not compare Plywise labels to Chess.com's proprietary labels.
- Evaluate stability against Plywise's versioned native reference.
- Investigate disagreements at positions, not only aggregate percentages.
- A faster profile is acceptable if important-mistake selection and educational conclusions remain
  stable.
- Unsupported capability must fall back honestly.

## Performance targets to validate

Targets remain provisional until the first run:

- Worker startup does not block interaction.
- Cancellation is visible promptly.
- The browser remains usable while analysis runs.
- Quick completes an ordinary game within a tolerable review wait on ordinary hardware.
- Balanced improves meaningful review stability enough to justify its extra cost.
- Browser output payloads stay within a documented server limit.

## Output

Add a dated result table under `artifacts/benchmarks/` during the implementation spike, keep private
game inputs out of Git, and summarize the selected profiles and tradeoffs in a focused PR.

No engine profile becomes a product default until results are repeatable.
