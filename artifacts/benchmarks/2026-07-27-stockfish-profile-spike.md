# Stockfish profile spike — 2026-07-27

## Decision

Keep Stockfish.js 18.0.8 lite single-threaded as the leading browser candidate.

- Quick depth 10 is fast enough for triage, but its 60% best-move agreement is not strong enough to
  name it the completed-review default.
- Balanced lite depth 14 is the best Phase 2 candidate. It matched 7 of 10 native-reference best
  moves with a 7.0 MiB payload.
- Do not ship the full single-threaded build in the first alpha. It did not improve best-move
  agreement over lite depth 14 in this fixture set, while its payload is 107.8 MiB.
- No profile is a product default yet. Full-game classification parity, real Web Worker behavior,
  browser compatibility, and lower-powered devices remain unmeasured.

## What ran

Three clean process runs on an Apple M2 Pro MacBook Pro with 16 GB RAM. Node.js 20.20.2 hosted the
WASM modules as a repeatable proxy for browser-worker compute. Native Stockfish 18 at depth 18 was
the reference.

The ten public regression FENs cover opening, quiet and tactical middlegames, castling, en passant,
promotion, repetition shape, mate-score handling, and endgames. No user game data was used.

The benchmark package was downloaded to a temporary directory and was not added to Git.

## Median of three runs

| Profile | Payload | Startup | 10-position wall time | Stop latency | Best-move agreement | Scores within 75 cp |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| Native depth 18 reference | installed binary | 645 ms | 6,902 ms | 0.51 ms | reference | reference |
| Quick lite single, depth 10 | 7.0 MiB | 239 ms | 211 ms | 47 ms | 6/10 | 5/10 |
| Balanced lite single, depth 14 | 7.0 MiB | 141 ms | 565 ms | 46 ms | 7/10 | 7/10 |
| Balanced full single, depth 14 | 107.8 MiB | 786 ms | 1,156 ms | 47 ms | 7/10 | 9/10 |

Process-wide RSS deltas were volatile but directionally important: the median was about 170 MiB for
the first lite load, 88 MiB for the second lite load, and 503 MiB for the full build. These are
Node-host measurements, not browser-worker peaks.

The lite worker also reinitialized successfully in all three runs, with a median 131 ms restart.

## Exact candidate

- npm package: `stockfish@18.0.8`
- package license: GPL-3.0
- engine identity: Stockfish 18 Lite WASM
- lite JavaScript SHA-256:
  `5243fd9b276cab7dfe3ad1d43ab9ead73568fac76468c614242977a210c4a391`
- lite WASM SHA-256:
  `a8fbc05ec6920b56d7485826dcb02c5ffd2826bcbf751cf973046f237a9096f1`
- full JavaScript SHA-256:
  `ce07b916870473a837b598b9b558c125e24568624e47dabc3381474558ee201d`
- full WASM SHA-256:
  `f611ac05ddb248fe975a4f180ac9fec7f7fb650f8f17f5fe4230fcc0fe6419c7`

## Reproduce

Download and extract the package outside the repository, then run:

```sh
node scripts/benchmark-stockfish-profiles.mjs \
  --engine-dir /path/to/extracted/package \
  --native /usr/local/bin/stockfish \
  --output /tmp/plywise-stockfish-result.json
```

The script prints the full per-position JSON. The engine directory must contain the npm package's
`index.js`, `package.json`, and `bin/` files.

## Limits and next evidence

- Run the same engine in a real Web Worker on current Chrome, Firefox, and Safari.
- Record network transfer, streaming compilation, Cache Storage behavior, long-task count, worker
  peak memory, and tab responsiveness.
- Add ordinary 35–45 move and long-endgame PGN fixtures, then compare the complete Plywise
  classification output rather than isolated engine scores.
- Test cancellation during repeated whole-game analysis, worker crashes, background-tab throttling,
  iOS Safari, Android Chrome, and a lower-powered laptop.
- Benchmark multithreaded lite only after the hosting headers and compatibility cost are explicit.

This spike selects an implementation candidate. It does not satisfy the Phase 2 browser matrix or
authorize distributing the GPL engine without the compliance work in
`ground-truth/OPEN_SOURCE_COMPLIANCE.md`.
