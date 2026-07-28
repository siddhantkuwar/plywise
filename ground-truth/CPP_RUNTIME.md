# C++ Runtime

## Role

The C++ runtime remains the technical center of Plywise. Moving to the web changes deployment and
persistence; it does not move chess-domain truth into React.

The runtime supports two analysis sources:

1. validated Stockfish observations produced by an approved browser worker;
2. Stockfish processes supervised by hosted C++ workers for optional deep or batch analysis.

Both paths produce the same versioned review contract.

## Domain subsystems

### Game ingestion

Accept a completed-game URL or PGN, obtain permitted public data, normalize it, validate the
variant, deduplicate it, and persist an immutable canonical record.

The runtime never accepts a Chess.com password, authentication cookie, or private session token.

### Chess core

- Position representation.
- Legal move generation and make/unmake.
- Check, mate, and draw-state support.
- SAN/UCI conversion.
- FEN parse and serialization.
- PGN reconstruction.
- Attack maps.
- Variation validation.

Correctness remains more important than representation novelty. Preserve perft, fixtures, and
round-trip tests.

### Browser-analysis validation

Browser engine output is untrusted network input. Validate:

- game and position identity;
- engine/version allowlist or explicit unknown labeling;
- search-profile bounds;
- score and mate normalization;
- legal best moves and principal variations;
- MultiPV count and result completeness;
- size, time, and submission limits;
- idempotency key and analysis-run ownership.

Invalid observations never become canonical move assessments.

### Server Stockfish manager

- Spawn and supervise processes.
- Perform UCI handshake and apply options.
- Parse `info` and `bestmove`.
- Normalize centipawn and mate scores.
- Support bounded MultiPV.
- Cancel and restart safely.
- Persist engine version and settings.
- Isolate failures to one worker/job.

### Review pipeline

- Reconstruct positions.
- Accept validated engine observations.
- Run independent move classification.
- Recognize openings.
- Detect patterns and weaknesses.
- Assemble deterministic explanations.
- Persist model, detector, engine, and configuration versions.

### Scheduler

The server scheduler supports explicit deep and batch work with:

- account-aware admission limits;
- bounded concurrency;
- fair scheduling;
- priorities, retries, cancellation, and graceful shutdown;
- idempotent job submission;
- per-account and global usage budgets;
- real progress events.

Browser-only analysis is not placed in the server compute queue.

### Persistence

Application services depend on repository interfaces rather than a single filesystem event log.

The hosted implementation begins with PostgreSQL for multi-user lookup and constraints. Versioned
events may remain useful for job history, analysis audit, recovery, and deterministic replay.

Required properties:

- account ownership on durable records;
- schema and event versions;
- checksums or integrity constraints where appropriate;
- transactional canonical imports;
- idempotent analysis writes;
- backup and restoration;
- export and complete deletion;
- compatibility rules for reanalysis.

## Guest mode

Guest mode should minimize server state:

- temporary opaque session identity;
- short retention with an explicit expiry;
- analysis performed in the browser by default;
- no advertising or unrelated tracking;
- deliberate account conversion to retain work;
- deletion at expiry or user request.

Where practical, guest PGN and intermediate engine data remain on the device.

## Concurrency and cost

- Bound every queue.
- Avoid oversubscribing Stockfish threads.
- Measure queue time separately from engine time.
- Record CPU time and cache effectiveness by engine profile.
- Enforce cost ceilings before public server analysis.
- Reserve interactive capacity.
- Keep browser work cancellable and off the main UI thread.

## Determinism

Every review stores:

- canonical game version;
- engine identity and build;
- browser or server execution source;
- depth, nodes, time, threads, hash, and MultiPV;
- classifier and detector versions;
- opening source/version;
- explanation provider/version.

## Security tests

- Cross-account read and write denial.
- Forged guest/account identifiers.
- Malformed PGN and oversized inputs.
- Invalid and adversarial browser engine output.
- Illegal principal variations.
- Duplicate and replayed submissions.
- Queue flooding and cancellation races.
- Database retry/idempotency behavior.
- Import redirects and upstream size limits.
- Export and deletion authorization.

## Correctness and performance tests

Retain PGN fixtures, SAN/FEN round trips, perft, legal/illegal variations, UCI parser fixtures,
process lifecycle, cache compatibility, classification fixtures, storage recovery, import-to-review
smoke tests, and benchmarks.

Add browser/server parity fixtures so the same engine observations create the same Plywise review
regardless of where Stockfish ran.
