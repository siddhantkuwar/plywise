# Open-Source Compliance

## Current conclusion

The GitHub repository is public, but Plywise is not yet legally open source because it has no
top-level license covering the original code.

Do not add a license on the owner's behalf. The recommended decision is `GPL-3.0-or-later`: it
matches the project's public-good intent, keeps derivative distributions open, and is the clearest
project-wide posture if Stockfish.js is distributed with the website. The owner must explicitly
approve that choice.

This inventory is engineering guidance, not legal advice.

## Distribution inventory

| Component | How used | License/status | Required action |
| --- | --- | --- | --- |
| Plywise C++, React, docs, and original assets | Project source and binaries | No project license | Owner chooses and adds top-level `LICENSE` |
| Stockfish native executable | User-installed local runtime today | GPL-3.0 | Do not bundle without GPL notices and corresponding source offer |
| `stockfish@18.0.8` / Stockfish.js | Proposed browser WASM worker; not committed | GPL-3.0 | If shipped, include license, copyright/attribution, exact source or durable corresponding-source offer, build/version record, and modification notice |
| Lasker SVG pieces | Vendored runtime art | CC BY 4.0 | Keep `web/public/pieces/lasker/ATTRIBUTION.md` and `LICENSE.md` in source and deployed notices |
| Curated Lichess puzzle sample | Vendored optional data | CC0 | Keep source URL, version, and CC0 declaration from `resources/tactical-corpus.json` |
| React, React DOM, frontend runtime helpers | Bundled JavaScript | MIT/ISC/BSD dependencies | Generate deployable third-party notice from the lockfile on release |
| TypeScript, Vite, Rollup, esbuild | Build tooling | Apache-2.0/MIT and transitive permissive licenses | Preserve lockfile; include notices if their licensed code is redistributed |
| libcurl | System-linked C++ dependency | curl license | Record build linkage and include notice when distributing a binary/container |
| Threads/CMake/Ninja/compiler | Platform/build tools | Toolchain-specific | Record build provenance; notices only for redistributed components |
| North-star reference bundle | Design-time reference, not runtime | Provenance not established in canonical docs | Do not ship its screenshots/assets until provenance is recorded |

The installed frontend tree currently resolves to MIT, ISC, BSD-3-Clause, and Apache-2.0 packages.
The lockfile, not a developer's `node_modules`, is the release input.

## Stockfish.js distribution gate

Before copying Stockfish.js into `web/public/` or a CDN artifact:

1. pin an exact release and verify JavaScript/WASM hashes;
2. retain `Copying.txt` and upstream attribution;
3. publish the exact corresponding source and build instructions, or a durable written source offer
   that satisfies GPL requirements;
4. identify local modifications or state that the upstream artifact is unmodified;
5. keep the worker separate and versioned in the application contract;
6. add its source and license link to the in-product notices page;
7. make CDN caching preserve the matching version rather than a floating filename;
8. have counsel or a qualified open-source reviewer confirm the combined-distribution posture.

The benchmark in `artifacts/benchmarks/2026-07-27-stockfish-profile-spike.md` used a temporary npm
download only. No Stockfish.js binary is added by that work.

## Recommended repository changes after owner approval

- top-level `LICENSE` with the chosen original-code license;
- `THIRD_PARTY_NOTICES.md` generated from locked production inputs and curated asset notices;
- `COPYING.stockfish.txt` plus corresponding-source location if Stockfish is distributed;
- `SECURITY.md` with private reporting instructions;
- `CONTRIBUTING.md` with contributor licensing terms;
- CI license-policy check for forbidden, missing, and unexpected licenses;
- release manifest containing dependency versions, engine hashes, container digest, and source links.

## Decision record

Owner action required:

- [ ] approve `GPL-3.0-or-later`;
- [ ] choose a different OSI license and obtain compatibility review;
- [ ] decide whether the hosted release distributes Stockfish.js or retrieves an independently
      installed engine.

Until one option is approved, issue #3 remains visibly blocked and the project must not claim that
the original code is open-source licensed.
