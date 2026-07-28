# Engineering Rules

## Repository first

Inspect the real repository, current branch, uncommitted work, tests, and deployed contracts before
choosing dependencies or changing architecture.

Canonical requirements live in `ground-truth/`. Dated audit and implementation reports remain
historical evidence.

## Delivery

- Create one `codex/` branch per coherent feature or phase slice.
- Start new feature branches from the latest remote default branch.
- Tie work to a GitHub issue with acceptance criteria.
- Keep commits scoped and easy to review.
- Open a draft pull request after implementation and verification.
- Do not merge the PR; the user reviews it as engineering manager.
- Do not stage unrelated user work.

## Refactoring

- Work in incremental, runnable vertical slices.
- Preserve the verified local analysis flow while the web path is introduced.
- Separate interface extraction from behavior migration where practical.
- Avoid changing both sides of an API without contract and end-to-end tests.
- Remove an old path only after its replacement is verified.
- Do not maintain local and hosted implementations without a clear shared boundary and removal plan.

## C++

C++ owns chess-domain truth: import normalization, PGN/SAN/FEN, legality, review assembly,
classification, openings, patterns, variations, player modeling, and server Stockfish orchestration.

Use RAII, explicit ownership, cooperative cancellation, checked external parsing, structured errors,
versioned data, bounded queues, and measured performance.

## React

React renders typed domain state, starts browser workers, and sends user intent. It does not
reimplement chess algorithms or turn unvalidated engine output into product truth.

Use centralized design tokens, accessible semantics, responsive layouts, and explicit guest,
account, loading, partial, offline, expired, and failure states.

## Browser engine

- Run Stockfish off the UI thread.
- Package executable code with the application; do not load remote code dynamically.
- Record engine version and full analysis profile.
- Bound memory, threads, time, and submitted payload size.
- Support cancellation and worker restart.
- Treat results as untrusted until C++ validation.
- Keep a single-threaded compatibility path.

## APIs

- Version breaking contracts.
- Use explicit status enums and opaque IDs.
- Require account or guest ownership on every user record.
- Use idempotency keys for imports, jobs, and result submission.
- Return structured error codes.
- Never parse human-readable logs in React.
- Keep secrets and privileged database credentials server-side.

## Data

- Canonical games are immutable.
- Analyses are versioned.
- Variations never rewrite canonical history.
- Account data is private by default.
- Guest retention is short and disclosed.
- Export and deletion are tested.
- Database migrations are reversible when feasible and always backed up before destructive changes.
- Production data never appears in fixtures, screenshots, or logs.

## Security and privacy

- Authenticate first and authorize every object.
- Test cross-account denial explicitly.
- Validate URLs, redirects, PGNs, engine observations, and upstream responses.
- Apply request, payload, queue, and compute limits.
- Do not collect Chess.com credentials or private session data.
- Do not analyze games in progress.
- Use least-privilege extension permissions.
- Store secrets only in approved local or deployment secret stores.
- Document telemetry separately and keep it opt-in when it contains chess or learning data.

## Testing

Required coverage grows with the web path:

- C++ correctness and contract tests;
- browser/server review parity fixtures;
- tenant-isolation and authorization tests;
- persistence migrations, retry, and idempotency;
- browser worker cancellation and compatibility;
- critical web flow E2E;
- theme, keyboard, responsive, and accessibility checks;
- sanitizer and race workflows;
- cost and queue benchmarks.

## Visual verification

Run the real product in both themes and supported widths. Verify guest, account, saved, expired,
offline, analyzing, completed, failed, and insufficient-evidence states.

A compiler cannot verify hierarchy, trust, clarity, or calm.
