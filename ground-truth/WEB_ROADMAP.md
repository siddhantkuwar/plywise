# Web Roadmap

## Goal

Move Plywise from a Mac-first local application to a web-first hybrid product without discarding
the verified C++ analysis system.

Completed single-game analysis remains free. Browser compute carries the default engine cost.
Accounts add continuity and personal intelligence. Optional paid features, if they ever exist, must
fund material cost or provide genuinely distinct value.

## Phase 0 — Web product definition

### Status

Engineering definition is complete. The original-code license and Chess.com authorization remain
explicit owner/external blockers; neither is misrepresented as complete.

### Deliverables

- Canonical web product vision and architecture.
- Simple public README.
- Six-phase GitHub backlog with issues, labels, and milestones.
- Browser/server engine benchmark plan.
- Persistence and account migration seams.
- Chess.com integration authorization draft.
- Open-source license decision and third-party inventory.
- Initial privacy, fair-play, and threat boundaries.

### Questions to answer

- Which browser Stockfish build gives a useful first review on ordinary hardware?
- What data can remain entirely on device in guest mode?
- Which C++ repository responsibilities must become interfaces?
- What is the minimum hosted C++ service?
- How are browser engine observations validated?
- What Chess.com interaction is explicitly permitted?
- Which license covers Plywise's original code?

### Exit criteria

- Canonical documents consistently describe the web-first product.
- Representative analysis benchmarks are recorded.
- Phase 1 interfaces and acceptance criteria are concrete.
- Licensing and Chess.com authorization are resolved or visibly blocked.
- No Mac packaging work is presented as the primary release path.

## Phase 1 — Web foundation

### Deliverables

- Independently deployable React frontend.
- Environment-based API origin and local-development compatibility.
- Containerized C++ service with health and readiness checks.
- Repository interfaces for games, analyses, jobs, profiles, drills, and settings.
- PostgreSQL implementation and migrations.
- Authentication integration.
- Account ownership on durable records.
- Server-side authorization and cross-account denial tests.
- Online vertical slice for one saved review.

### Vertical slice

```text
create account
→ sign in
→ import completed PGN
→ request analysis
→ observe real progress
→ open completed review
→ retry and create a legal variation
→ sign out/in
→ reopen the same review
```

### Exit criteria

- Two test users cannot access each other's records.
- The hosted flow preserves the verified local behavior.
- Imports and jobs are idempotent.
- Database backup and migration procedures are documented.
- Local development still works.

## Phase 2 — Free private web alpha

### Deliverables

- Guest completed-game import without account creation.
- Stockfish WebAssembly running in a Web Worker.
- Quick and Balanced engine profiles.
- Worker cancellation, restart, compatibility, and resource limits.
- Typed submission of versioned engine observations.
- C++ validation and browser/server parity fixtures.
- Guest-to-account save flow.
- Saved cross-device history.
- Account export and deletion.
- Rate limiting, queue bounds, and basic operational monitoring.
- Private-alpha browser and accessibility matrix.

### Exit criteria

- A guest can complete a useful free review.
- Account creation is optional until saving.
- Browser observations create the same Plywise review as equivalent server observations.
- Unsupported devices receive an honest fallback.
- Private-alpha users can return to saved reviews from another device.
- No critical privacy, isolation, or data-loss issue remains.

## Phase 3 — Personal intelligence

### Deliverables

- Versioned weakness taxonomy and stronger pattern evidence.
- Cross-game recurrence with supporting positions.
- Confidence, recency, and sample-size rules.
- Personal Home focus.
- Practice queue from real mistakes.
- Spaced repetition and drill outcomes.
- Evidence of whether practice transfers into later games.
- Opening and game-phase profiles.
- Clear insufficient-data states.

### Exit criteria

- A returning user receives at least one useful, traceable insight that cannot come from a single
  isolated review.
- Every insight links to games and positions.
- The system does not claim mastery or decline from weak evidence.
- Practice selection explains why each position matters.

## Phase 4 — Authorized browser companion

### Prerequisite

Written clarification or authorization for the intended Chess.com completed-game integration.

### Deliverables

- Manifest V3 extension with one narrow purpose.
- Minimal permissions and user-initiated access.
- Reliable completed-game guard.
- Short-lived website handoff.
- No credentials, cookies, chat, or unrelated browsing collection.
- Privacy policy, fair-play disclosure, store materials, and support path.
- Toolbar or side-panel experience first.
- Injected under-board action only if explicitly permitted and robust.

### Exit criteria

- The extension cannot analyze a game in progress.
- Data collection matches the store disclosure.
- Page changes fail closed.
- Store and upstream policy review are complete.

## Phase 5 — Sustainable optional features

This phase is optional. Plywise does not require monetization to complete the free product.

### Candidate features

- Deep server analysis credits.
- Large batch queues.
- Advanced longitudinal intelligence.
- Expensive coaching or model infrastructure.
- Team or coach workflows with explicit sharing.

### Rules

- Do not put basic completed-game review behind payment.
- Measure compute and support cost before setting limits.
- Centralize entitlements in typed server contracts.
- Avoid advertising based on chess, browsing, or learning data.
- Keep account export and deletion independent of subscription state.

### Exit criteria

- Any paid boundary corresponds to real cost or distinct value.
- Billing failures cannot lose user data.
- Usage limits and costs are understandable.

## Phase 6 — Public open-source release

### Deliverables

- Complete open-source license and third-party notices.
- Public contributor setup and architecture guide.
- Stable hosted release and status communication.
- Checked-in critical-flow browser tests.
- Load, abuse, sanitizer, race, migration, and restore qualification.
- Accessibility review.
- Privacy policy, terms, security reporting, and support documentation.
- Release notes, changelog, versioning, and rollback plan.

### Exit criteria

- A new contributor can run the stack from documented steps.
- A new user can analyze a completed game without help.
- Backups have been restored in a rehearsal.
- Public traffic has explicit cost ceilings.
- No critical security, privacy, accessibility, or fair-play issue remains.

## Deferred until justified

- Native app-store distribution.
- Online chess play.
- Social feeds.
- Cloud LLM chess authority.
- Multiple engines without a clear user benefit.
- Broad browser permissions.
- Mobile-native applications.
