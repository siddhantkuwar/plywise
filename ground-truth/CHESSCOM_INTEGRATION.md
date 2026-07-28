# Chess.com Integration

## Status

The existing completed-game URL and public archive flow remains the reference implementation.

The public website may support:

- a user-pasted completed-game URL;
- a user-pasted PGN;
- permitted public profile/archive synchronization.

The proposed browser extension remains blocked until Chess.com provides written clarification or
authorization for the intended completed-game workflow.

## Rules

- Analyze completed games only.
- Never provide assistance for a live or ongoing game.
- Never collect a Chess.com password, session cookie, authentication token, chat, or private data.
- Prefer documented public APIs.
- Do not scrape page content when an approved API or user-provided PGN is available.
- Do not imply affiliation, sponsorship, or compatibility approval.
- Do not describe Plywise as bypassing a paywall.
- Do not copy Chess.com's board, pieces, sounds, icons, classification glyphs, product copy, or
  layout.

## Public API behavior

Use the read-only PubAPI conservatively:

1. Store a normalized public username after explicit user action.
2. Request archive metadata serially or with a documented small bound.
3. Use ETag and Last-Modified when supplied.
4. Respect cache freshness and 429 responses.
5. Send a descriptive User-Agent with a project contact.
6. Import unseen completed games only.
7. Show synchronization freshness.
8. Analyze only after the user asks.

Public API freshness does not guarantee instant post-game availability. The product must not promise
that profile refresh alone imports a game immediately.

## Browser companion proposal

The minimum-permission extension concept is:

1. The user invokes Plywise on a completed game page.
2. The extension reads only the current public game URL.
3. It confirms a completed state through an approved signal.
4. It displays a clear user-facing action.
5. On click, it opens Plywise with a short-lived import handoff.
6. Plywise resolves the completed game through the approved integration path.

An injected button underneath the board is a later option, not the starting implementation.

Before extension work, send Chess.com:

- product purpose and open-source repository;
- exact data accessed;
- completed-game and fair-play safeguards;
- proposed UI location;
- API request behavior;
- retention and deletion behavior;
- privacy policy and contact;
- explicit request for written authorization.

## Data

Normalize and validate:

- stable game identity and URL;
- PGN;
- players and ratings;
- result and termination;
- time control and end time;
- variant;
- available opening metadata;
- upstream freshness and provenance.

## Errors

Unknown username, unavailable or stale game, incomplete game, unsupported variant, duplicate import,
rate limiting, upstream failure, malformed response, and policy-disabled extension behavior are
distinct structured states.

## Privacy

Users can disconnect a public username, export linked records, and delete account data. Public data
retrieval is disclosed plainly. No game is made publicly discoverable by Plywise merely because it
was public upstream.
