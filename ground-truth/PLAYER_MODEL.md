# Player Model and Personal Intelligence

## Purpose

The personal intelligence layer is Plywise's main long-term differentiation.

It is a traceable model of the user's chess—not a generic dashboard, personality score, or pile of
decorative charts.

## Sources

- Permitted public profile and rating data.
- Imported canonical PGNs.
- Versioned Plywise reviews.
- Move classifications, openings, and patterns.
- Retry and practice outcomes.
- Review behavior intentionally saved by the user.

Guest reviews do not silently join an account profile. The user deliberately saves or imports them.

## Profile areas

### Rating

Current and historical rating by time control, peak, recent change, game count, and freshness.

### Move quality

Accuracy, errors per game, average evaluation loss, first serious error, and performance while
winning, equal, or losing.

### Game phase

Opening, middlegame, and endgame evidence with sample size and trend.

### Openings

Color, results, evaluation at key ranges, personal departures, repeated errors, and supporting
positions.

### Patterns

Seen, missed, successfully used, impact, recurrence, practice history, and later-game transfer.

### Review behavior

Reviews started and completed, retries attempted, hints used, variations explored, and positions
revisited. These are learning signals, not engagement tricks.

## Weakness taxonomy

Examples include hanging pieces, ignored threats, unsafe king, premature pawn moves, poor trades,
missed tactics, passed-pawn handling, back-rank safety, delayed development, and time pressure.

A weakness aggregates repeated evidence and always links to supporting positions.

## Deterministic insights

Examples:

> In your last 20 analyzed rapid games, 11 of 17 major errors occurred before move 12.

> You solved three piece-safety retries, but the same pattern appeared again in two later games.

Each claim stores:

- account and analysis scope;
- supporting game and ply identifiers;
- numerator and denominator;
- time-control and date window;
- detector/classifier versions;
- confidence and freshness.

## Confidence

Confidence accounts for sample size, recency, time control, detector strength, engine compatibility,
and version changes. Insufficient evidence is an explicit state.

Do not declare mastery or decline from a tiny or incompatible sample.

## Practice selection

Practice prioritizes:

1. recurring high-impact weaknesses;
2. recently missed motifs;
3. positions not yet successfully retried;
4. spaced repetition due dates;
5. evidence of failure transferring into later games.

The system explains why each position was selected.

## Privacy and ownership

- Account data is private by default.
- No telemetry or model training use is implied by account creation.
- Export and deletion are first-class product actions.
- Cross-account aggregation uses no identifiable game content without explicit consent.
- Connected public profile data can be disconnected and removed.
- Retention and backup behavior are documented.
