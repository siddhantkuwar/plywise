# System Architecture

## Target

```text
React web application
  guest flow, account flow, board, review, practice, settings
        │                         │
        │ typed HTTPS/events      └── browser Stockfish Web Worker
        │                               free default compute
        ▼
C++ application service
  import, validation, review assembly, classifications, profiles
        │                         │
        │                         └── optional C++/Stockfish workers
        │                               deep or batch compute
        ▼
Persistence interfaces
  PostgreSQL projections, versioned analysis records, job events, backups
```

The existing loopback application remains a supported development and reference mode. The website
becomes the primary user-facing product.

## Ownership

### React

React owns:

- routes, layouts, themes, and accessible interaction;
- board rendering and user move intent;
- guest/account onboarding;
- progress presentation;
- transient disclosure and selection state;
- starting and stopping a browser engine worker.

React does not decide move legality, classification, opening truth, pattern truth, or player-profile
truth.

### Browser engine worker

The browser worker produces versioned Stockfish observations for free analysis:

- position;
- engine identity and build;
- search profile and limits;
- evaluation, nodes, time, MultiPV, and principal variations;
- cancellation and failure state.

These observations are untrusted input to the hosted service. The C++ boundary validates positions,
legal principal variations, configuration limits, and completeness before producing a canonical
Plywise review.

### C++ application and domain

C++ owns:

- public completed-game import and PGN normalization;
- SAN, FEN, board reconstruction, and legal moves;
- canonical game identity and immutable move history;
- review assembly, classification, opening recognition, patterns, and explanations;
- variation validation;
- profile, weakness, practice, and confidence aggregation;
- server Stockfish lifecycle and scheduling;
- version compatibility and deterministic persistence contracts.

### Persistence

The hosted system uses explicit repositories for games, analyses, profiles, drills, settings, and
jobs. PostgreSQL is the initial multi-user projection store.

The existing append-only event model may continue where it provides useful recovery, audit, and
replay behavior. The application layer must no longer assume that one filesystem directory
represents the only user.

## Identity and tenancy

- Guest analysis receives a short-lived anonymous session identifier.
- Account requests carry a verified identity from the authentication provider.
- Every durable user-owned record has an account ID.
- Authorization is enforced in the C++ service before reading or mutating records.
- Database row-level controls may provide defense in depth but do not replace application checks.
- Public game data is not automatically public inside Plywise.

## Main use cases

- ImportCompletedGame
- StartGuestAnalysis
- SubmitBrowserEngineObservations
- StartServerAnalysis
- CancelAnalysis
- GetReview
- SaveGuestReviewToAccount
- CreateVariation / ExtendVariation
- GetPlayerProfile
- GeneratePracticeQueue
- ExportAccountData
- DeleteAccountData

## Canonical game and analyses

```text
ImportedGame (immutable canonical moves)
├── AnalysisRun (engine + configuration + classifier versions)
│   └── MoveAssessments
├── ReviewAttempts
└── VariationTree
```

Reanalysis creates a new versioned run or deliberately supersedes one under an explicit
compatibility rule. It never silently changes the imported game.

## Progress

Progress is event driven for browser and server analysis:

```text
queued
preparing
reconstructing
evaluating
validating
classifying
detecting_patterns
persisting
completed
cancelled
failed
```

Completed units and totals represent real positions or stages. Timers must not imitate progress.

## Failure boundaries

- Browser engine failure cannot lose the imported game.
- Server engine failure cannot crash the web application.
- Import failure cannot create a partial canonical record.
- Cancellation reaches an explicit final state.
- Malformed PGN and engine output produce structured errors.
- A stale job cannot replace a newer run.
- One account cannot address another account's records.
- Database or queue retries are idempotent.
- Guest expiration is explained before local work is discarded.
