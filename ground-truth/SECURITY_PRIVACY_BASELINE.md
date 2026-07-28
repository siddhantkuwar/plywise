# Security, Privacy, and Fair-Play Baseline

## Status

This is the minimum release boundary for the hosted product. The current server remains a loopback
single-user development runtime and must not be exposed directly to the internet.

## Data inventory

| Data | Purpose | Sensitivity | Default |
| --- | --- | --- | --- |
| Authentication subject and email | Account access and notices | Private identity data | Managed provider; store stable subject and only required email |
| Imported PGN and game metadata | Review and history | Private account data even when public upstream | Private by default |
| Engine observations and reviews | Analysis and reproducibility | Private learning data | Versioned and owner-scoped |
| Variations, attempts, practice, profile evidence | Personal intelligence | Private behavioral/learning data | Account-only |
| Public Chess.com username | User-requested import/sync | Public upstream but linked identity | Store only after explicit action |
| Guest session proof | Temporary ownership | Security credential | Hash server-side; short lived |
| Operational logs | Reliability and abuse response | Metadata that may become identifying | Minimize and expire |

Plywise does not need passwords, Chess.com cookies, payment data, chat, contacts, or unrelated
browsing history for Phases 1–3.

## Provisional retention

These are engineering defaults until a published privacy policy makes them user-facing commitments:

- guest server records: expire within 24 hours;
- guest proof: expire with the guest record and invalidate immediately after account claim;
- account games, reviews, and learning data: retain until the user deletes them;
- request/security logs: 14 days, with no raw PGN, access token, principal variation, or full FEN;
- encrypted database backups: 30 days;
- deletion from the active database: immediate job, completed or visibly failed within 24 hours;
- deletion aging out of backups: within 30 days.

Changing these values requires a documented product and privacy review.

## User controls

- Account creation is optional for one completed-game review.
- Saving makes retention and cross-device behavior explicit.
- Users can unlink a public Chess.com username without deleting unrelated account records.
- Export includes imported games, reviews, variations, practice history, settings, and intelligence
  evidence in a portable documented format.
- Delete account revokes sessions, removes active private records, schedules backup expiry, and
  returns a trackable receipt.
- Plywise never turns a public upstream game into a publicly discoverable Plywise profile by default.

## Hosted security gates

- A managed provider authenticates; C++ verifies token issuer, audience, signature, expiry, and
  required claims.
- Every read and mutation authorizes account or guest ownership in the service and query.
- IDs are opaque; knowing an ID grants no access.
- Browser Stockfish observations are bounded, versioned, sequenced, and validated against canonical
  C++ FEN and legal moves.
- Imports, jobs, result submissions, guest claims, and deletion are idempotent.
- Request, PGN, observation, queue, worker, account, guest, IP, and global limits are explicit.
- TLS terminates at an approved proxy; internal services and database are not publicly reachable.
- Production responses set a strict CSP, HSTS, `nosniff`, frame denial, referrer policy, and the
  minimum permissions policy.
- Cross-origin isolation is enabled only if a selected engine profile needs it.
- Logs redact tokens and do not include raw game bodies or private learning content.
- Secrets come from deployment secret storage and never enter the frontend bundle or Git.
- CI pins or reviews third-party actions and scans dependencies, secrets, containers, and licenses.

## Browser-engine boundary

The worker is isolated from React state by a typed message contract. It accepts canonical positions
and bounded profiles, and returns observations only. React cannot classify a move or persist an
observation as truth.

The C++ service verifies:

- run and owner;
- expected FEN hash and ply;
- allowed engine build/hash and profile;
- depth, node, time, MultiPV, line-count, and payload bounds;
- legal best move and every principal-variation move;
- sequence completeness, duplicates, stale runs, and finalization state.

Forged client results may affect only the attacker's transient run until validation succeeds. They
must never alter shared caches, public statistics, or another user's intelligence.

## Fair play

- Analyze only completed games.
- Do not offer live board evaluation, move suggestions, overlays, or background monitoring.
- A pasted Chess.com URL must resolve to a completed supported game before analysis.
- Profile sync imports completed archive records and never starts analysis without user intent.
- The proposed extension remains blocked without written Chess.com authorization.
- Any future extension fails closed when completion is ambiguous and requests only the current
  completed-game URL.
- Product copy does not promise affiliation or describe analysis as bypassing a paywall.

## Incident minimum

Before a public alpha, define an owner and a contact path for:

- suspected cross-account access;
- leaked signing or database credentials;
- malicious engine payloads or parser crashes;
- abusive compute traffic;
- upstream policy complaints;
- deletion/export failures;
- dependency or build compromise.

The response runbook must support token/key rotation, session revocation, job admission shutdown,
deployment rollback, user notice assessment, and evidence-preserving logs.

## Release blockers

- exposing the current unauthenticated loopback API as the hosted API;
- any cross-account read or write;
- accepting browser analysis without C++ legality and run validation;
- storing raw credentials or Chess.com session data;
- silently retaining expired guest data;
- starting extension implementation before authorization;
- distributing Stockfish.js without the required GPL source/license offer;
- publishing without an original-code license and third-party notices.
