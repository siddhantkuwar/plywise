# Hosted Data Model and Repository Seams

## Purpose

Define the smallest persistence boundary needed to move the verified local runtime into a
multi-user website without moving chess truth into TypeScript.

This is a Phase 1 contract. It does not claim PostgreSQL or accounts exist today.

## Current constraint

`pct::app::Repository` currently projects every game, analysis, drill, profile, job state, Chess.com
record, variation, and review attempt from one `EventLog`. The object is cohesive for a local
single-user process, but it has no account boundary and assumes one filesystem-backed data owner.

The first refactor must preserve this implementation behind explicit application-facing
interfaces. It must not replace working local storage and hosted storage in the same change.

## Identity vocabulary

- `AccountId`: opaque stable identifier derived from a verified authentication subject.
- `GuestId`: opaque random identifier with an expiry and no predictable relationship to a browser.
- `OwnerId`: tagged account or guest identity used on every user-owned aggregate.
- `GameId`: opaque internal identifier; upstream URLs and Chess.com IDs are attributes, not primary
  authorization keys.
- `AnalysisRunId`: immutable versioned run identity.
- `IdempotencyKey`: caller-generated opaque key scoped to owner, operation, and expiry.

Raw database sequence values never cross the API boundary.

## Repository boundary

Use cohesive repositories around transaction boundaries:

- `IGameRepository`
  - create or return an immutable canonical game for an owner and canonical import key;
  - read/list only inside an owner scope;
  - attach provenance without changing canonical moves;
  - transfer a guest-owned game to an account idempotently.
- `IAnalysisRepository`
  - create a versioned run for an owned game;
  - append validated position observations in sequence;
  - atomically finalize the canonical review;
  - never let a stale run supersede a newer compatible run.
- `IJobRepository`
  - enqueue, claim with a lease, heartbeat, cancel, finish, and recover jobs;
  - enforce per-owner and global admission limits;
  - make terminal transitions idempotent.
- `IIntelligenceRepository`
  - store versioned evidence, profile projections, practice items, and outcomes;
  - preserve links back to analysis run, game, ply, and classifier version.
- `ISettingsRepository`
  - read and update account-scoped presentation and engine preferences;
  - keep secrets and deployment settings outside user settings.

Do not create one interface per table. `SaveGuestReviewToAccount`, analysis finalization, and account
deletion require transactions across related records and should be explicit application use cases.

## PostgreSQL records

### Identity and ownership

- `accounts(id, auth_provider, auth_subject, created_at, deletion_requested_at)`
- `guest_sessions(id, token_hash, created_at, expires_at, claimed_by_account_id)`
- unique `(auth_provider, auth_subject)`
- no password or raw guest token storage

### Games

- `games(id, canonical_hash, normalized_pgn, metadata_json, created_at)`
- `game_owners(game_id, owner_kind, owner_id, imported_at, source_kind, source_key)`
- unique `(owner_kind, owner_id, source_kind, source_key)` where a source key exists
- immutable canonical moves after creation

One canonical game may be referenced by multiple owners, but an owner mapping never makes another
owner's review or learning data visible.

### Analyses and reviews

- `analysis_runs(id, game_id, owner_kind, owner_id, source, engine_build, engine_hash,
  profile_version, classifier_version, status, created_at, completed_at, supersedes_id)`
- `analysis_positions(run_id, ply, canonical_fen_hash, sequence, depth, nodes, time_ms,
  observation_json, validated_at)`
- `move_assessments(run_id, ply, assessment_json)`
- `reviews(run_id, review_json, created_at)`
- unique `(run_id, ply, sequence)`
- foreign keys from every analysis record to both the game and owner context

Browser observations are staging evidence, not review truth. Only C++ validation may move a run from
`validating` to `classifying` and `completed`.

### Jobs

- `analysis_jobs(id, run_id, owner_kind, owner_id, priority, status, attempt, lease_owner,
  lease_expires_at, cancel_requested_at, created_at, updated_at)`
- `job_events(job_id, sequence, stage, completed_units, total_units, created_at)`
- unique `(job_id, sequence)`
- compare-and-swap status transitions and expiring worker leases

### Variations, practice, and intelligence

- variations and nodes retain an owner and canonical root FEN;
- review attempts retain owner, game, run, ply, move, result, and timestamp;
- evidence records retain model version and source positions;
- practice scheduling and outcomes are account-private and versioned.

## Transaction rules

1. Authenticate, derive `OwnerId`, and begin a transaction.
2. Load or mutate through an owner-scoped repository method.
3. Include ownership in the query predicate; do not fetch globally and compare only in memory.
4. Use a unique idempotency record before starting expensive or externally visible work.
5. Commit canonical game plus owner mapping together.
6. Finalize an analysis run, review, and terminal job event together.
7. Use an outbox row for events that must reach a separate worker or stream after commit.

Row-level security may be added as defense in depth. Application authorization remains mandatory.

## Guest-to-account transfer

`SaveGuestReviewToAccount` accepts a verified account, a live guest proof, and an idempotency key.
One transaction:

1. locks the guest session;
2. rejects expiry or a different prior claimant;
3. creates account ownership mappings for the game and completed analyses;
4. merges exact duplicates without duplicating intelligence evidence;
5. records the claimant and transfer receipt;
6. invalidates the guest proof.

Retry returns the same receipt. It never copies another guest's records.

## Local adapter

The event-log implementation remains a supported adapter:

- local mode supplies one synthetic local owner;
- existing event types and projections remain valid;
- repository interface extraction delegates to current `Repository` behavior;
- no PostgreSQL concept leaks into chess parsing, analysis, or React.

This preserves the current import, review, retry, variation, and progress flows while the hosted
adapter is added beside it.

## Phase 1 implementation slices

1. Introduce opaque ID and owner types plus interface contract tests.
2. Wrap the existing event-log repository as the local adapter with no behavior change.
3. Add account/guest ownership to application use-case signatures.
4. Add migrations and a PostgreSQL adapter for games plus imports.
5. Add analyses and job leases.
6. Add variations, review attempts, profile evidence, practice, and settings.
7. Add export, deletion, backup, and restore tests.

## Acceptance criteria

- Local CTest and the verified analysis flow remain unchanged after interface extraction.
- Two test accounts receive indistinguishable not-found responses for each other's IDs.
- Duplicate imports and result submissions return the original resource.
- A crashed worker cannot leave an analysis permanently claimed.
- A stale analysis cannot replace a newer completed run.
- Guest claim is atomic and safe to retry.
- Account deletion covers owned mappings, private reviews, intelligence, settings, and auth subject.
- Backup restoration is rehearsed against the same migration version used in production.
