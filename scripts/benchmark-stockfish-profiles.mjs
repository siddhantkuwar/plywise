#!/usr/bin/env node

import { createHash } from "node:crypto";
import { spawn } from "node:child_process";
import { createRequire } from "node:module";
import { readFile, stat, writeFile } from "node:fs/promises";
import { basename, join, resolve } from "node:path";
import process from "node:process";
import readline from "node:readline";

const require = createRequire(import.meta.url);

const fixtures = [
  {
    id: "initial",
    kind: "opening",
    fen: "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
  },
  {
    id: "kiwipete",
    kind: "castling",
    fen: "r3k2r/p1ppqpb1/bn2pnp1/2pP4/1p2P3/2N2N2/PPQBBPPP/R3K2R w KQkq - 0 1",
  },
  {
    id: "quiet-middlegame",
    kind: "positional",
    fen: "r1bq1rk1/pp1n1ppp/2pbpn2/8/2BPP3/2N1BN2/PPQ2PPP/R4RK1 w - - 4 10",
  },
  {
    id: "open-middlegame",
    kind: "tactical",
    fen: "r1bq1rk1/pp3ppp/2n1pn2/2bp4/8/2P1PN2/PPBN1PPP/R1BQ1RK1 w - - 2 9",
  },
  {
    id: "en-passant",
    kind: "special-move",
    fen: "rnbqkbnr/pppp1ppp/8/4pP2/8/8/PPPPP1PP/RNBQKBNR w KQkq e6 0 2",
  },
  {
    id: "promotion-race",
    kind: "endgame",
    fen: "8/P6k/8/8/8/8/6pK/8 w - - 0 1",
  },
  {
    id: "rook-endgame",
    kind: "endgame",
    fen: "8/5pk1/5np1/7p/4P3/1P3P1P/5KP1/2R5 w - - 0 36",
  },
  {
    id: "mate-score",
    kind: "mate",
    fen: "6k1/5ppp/8/8/8/8/5PPP/6RK w - - 0 1",
  },
  {
    id: "repetition-shape",
    kind: "repetition",
    fen: "r1bq1rk1/ppp2ppp/2np1n2/8/2B1P3/2N2N2/PPP2PPP/R1BQ1RK1 w - - 4 8",
  },
  {
    id: "long-endgame-shape",
    kind: "endgame",
    fen: "8/2p2pk1/3p2p1/1p1P3p/1P2P2P/2P3P1/5PK1/8 w - - 0 40",
  },
];

function parseArgs(argv) {
  const result = {
    engineDir: process.env.PCT_STOCKFISH_JS_DIR ?? "",
    native: process.env.PCT_STOCKFISH ?? "/usr/local/bin/stockfish",
    output: "",
  };
  for (let index = 0; index < argv.length; index += 1) {
    const argument = argv[index];
    const value = argv[index + 1];
    if (argument === "--engine-dir") {
      result.engineDir = value;
      index += 1;
    } else if (argument === "--native") {
      result.native = value;
      index += 1;
    } else if (argument === "--output") {
      result.output = value;
      index += 1;
    } else {
      throw new Error(`unknown or incomplete argument: ${argument}`);
    }
  }
  if (!result.engineDir) {
    throw new Error(
      "Stockfish.js directory required: pass --engine-dir or set PCT_STOCKFISH_JS_DIR",
    );
  }
  return result;
}

function nowMilliseconds() {
  return Number(process.hrtime.bigint()) / 1_000_000;
}

function parseInfo(line) {
  if (!line.startsWith("info ")) return null;
  const tokens = line.trim().split(/\s+/);
  const valueAfter = (key) => {
    const index = tokens.indexOf(key);
    return index >= 0 && index + 1 < tokens.length ? tokens[index + 1] : null;
  };
  const scoreIndex = tokens.indexOf("score");
  const pvIndex = tokens.indexOf("pv");
  return {
    depth: Number(valueAfter("depth") ?? 0),
    seldepth: Number(valueAfter("seldepth") ?? 0),
    nodes: Number(valueAfter("nodes") ?? 0),
    engine_time_ms: Number(valueAfter("time") ?? 0),
    score:
      scoreIndex >= 0
        ? { type: tokens[scoreIndex + 1], value: Number(tokens[scoreIndex + 2]) }
        : null,
    pv: pvIndex >= 0 ? tokens.slice(pvIndex + 1) : [],
  };
}

class UciSession {
  constructor(send, close) {
    this.sendRaw = send;
    this.closeRaw = close;
    this.messages = [];
    this.waiters = [];
    this.infoListener = null;
  }

  receive(message) {
    const line = String(message).trim();
    if (!line) return;
    if (this.infoListener) this.infoListener(line);
    const waiterIndex = this.waiters.findIndex((waiter) => waiter.predicate(line));
    if (waiterIndex >= 0) {
      const [waiter] = this.waiters.splice(waiterIndex, 1);
      clearTimeout(waiter.timer);
      waiter.resolve(line);
    } else {
      this.messages.push(line);
    }
  }

  waitFor(predicate, timeoutMs = 30_000) {
    const existing = this.messages.findIndex(predicate);
    if (existing >= 0) {
      const [message] = this.messages.splice(existing, 1);
      return Promise.resolve(message);
    }
    return new Promise((resolvePromise, rejectPromise) => {
      const waiter = {
        predicate,
        resolve: resolvePromise,
        timer: setTimeout(() => {
          const index = this.waiters.indexOf(waiter);
          if (index >= 0) this.waiters.splice(index, 1);
          rejectPromise(new Error(`timed out after ${timeoutMs} ms waiting for UCI output`));
        }, timeoutMs),
      };
      this.waiters.push(waiter);
    });
  }

  send(command) {
    this.sendRaw(command);
  }

  async ready() {
    this.send("uci");
    const identity = await this.waitFor((line) => line.startsWith("id name "));
    await this.waitFor((line) => line === "uciok");
    this.send("setoption name Hash value 16");
    this.send("setoption name Threads value 1");
    this.send("isready");
    await this.waitFor((line) => line === "readyok");
    return identity.slice("id name ".length);
  }

  async analyze(fen, depth) {
    const info = [];
    this.infoListener = (line) => {
      const parsed = parseInfo(line);
      if (parsed) info.push(parsed);
    };
    const start = nowMilliseconds();
    this.send("setoption name MultiPV value 1");
    this.send(`position fen ${fen}`);
    this.send(`go depth ${depth}`);
    const bestMoveLine = await this.waitFor((line) => line.startsWith("bestmove "), 60_000);
    const wallTimeMs = nowMilliseconds() - start;
    this.infoListener = null;
    const latest = info.reduce(
      (best, candidate) => (candidate.depth >= (best?.depth ?? -1) ? candidate : best),
      null,
    );
    const bestMoveTokens = bestMoveLine.split(/\s+/);
    return {
      best_move: bestMoveTokens[1],
      ponder_move: bestMoveTokens[2] === "ponder" ? bestMoveTokens[3] : null,
      wall_time_ms: Number(wallTimeMs.toFixed(2)),
      ...latest,
    };
  }

  async cancellationLatency() {
    this.send(`position fen ${fixtures[2].fen}`);
    this.send("go infinite");
    await new Promise((resolvePromise) => setTimeout(resolvePromise, 100));
    const start = nowMilliseconds();
    this.send("stop");
    await this.waitFor((line) => line.startsWith("bestmove "), 5_000);
    return Number((nowMilliseconds() - start).toFixed(2));
  }

  close() {
    this.closeRaw();
  }
}

async function createNativeSession(executable) {
  const child = spawn(executable, [], { stdio: ["pipe", "pipe", "pipe"] });
  const session = new UciSession(
    (command) => child.stdin.write(`${command}\n`),
    () => {
      child.stdin.write("quit\n");
      child.stdin.end();
    },
  );
  readline.createInterface({ input: child.stdout }).on("line", (line) => session.receive(line));
  let stderr = "";
  child.stderr.on("data", (chunk) => {
    stderr += chunk.toString();
  });
  child.on("error", (error) => {
    throw error;
  });
  child.on("exit", (code) => {
    if (code && stderr) process.stderr.write(stderr);
  });
  return session;
}

async function createWasmSession(engineDir, flavor) {
  const suffix = flavor === "lite-single" ? "lite-single" : "single";
  const engineModule = join(engineDir, "bin", `stockfish-18-${suffix}.js`);
  delete require.cache[require.resolve(engineModule)];
  const initEngine = require(join(engineDir, "index.js"));
  const engine = await initEngine(flavor);
  const session = new UciSession(
    (command) => engine.sendCommand(command),
    () => {},
  );
  engine.listener = (line) => session.receive(line);
  return session;
}

async function sha256(path) {
  const bytes = await readFile(path);
  return createHash("sha256").update(bytes).digest("hex");
}

async function artifactMetadata(engineDir, flavor) {
  const suffix = flavor === "lite-single" ? "lite-single" : "single";
  const jsPath = join(engineDir, "bin", `stockfish-18-${suffix}.js`);
  const wasmPath = join(engineDir, "bin", `stockfish-18-${suffix}.wasm`);
  const [jsStat, wasmStat, jsHash, wasmHash] = await Promise.all([
    stat(jsPath),
    stat(wasmPath),
    sha256(jsPath),
    sha256(wasmPath),
  ]);
  return {
    javascript: {
      file: basename(jsPath),
      bytes: jsStat.size,
      sha256: jsHash,
    },
    wasm: {
      file: basename(wasmPath),
      bytes: wasmStat.size,
      sha256: wasmHash,
    },
    total_bytes: jsStat.size + wasmStat.size,
  };
}

async function runProfile({ id, kind, depth, create }) {
  const memoryBefore = process.memoryUsage().rss;
  const startupStart = nowMilliseconds();
  const session = await create();
  const engineVersion = await session.ready();
  const startupMs = nowMilliseconds() - startupStart;
  const positions = [];
  for (const fixture of fixtures) {
    positions.push({
      fixture_id: fixture.id,
      fixture_kind: fixture.kind,
      ...(await session.analyze(fixture.fen, depth)),
    });
  }
  const cancellationLatencyMs = await session.cancellationLatency();
  session.close();
  const memoryAfter = process.memoryUsage().rss;
  return {
    id,
    engine_kind: kind,
    engine_version: engineVersion,
    depth,
    multipv: 1,
    hash_mb: 16,
    threads: 1,
    startup_ms: Number(startupMs.toFixed(2)),
    total_analysis_wall_time_ms: Number(
      positions.reduce((total, position) => total + position.wall_time_ms, 0).toFixed(2),
    ),
    cancellation_latency_ms: cancellationLatencyMs,
    process_rss_delta_bytes: memoryAfter - memoryBefore,
    positions,
  };
}

function agreement(profile, reference) {
  const referenceByFixture = new Map(
    reference.positions.map((position) => [position.fixture_id, position]),
  );
  let bestMoveMatches = 0;
  let comparableScores = 0;
  let scoresWithin75Cp = 0;
  for (const position of profile.positions) {
    const expected = referenceByFixture.get(position.fixture_id);
    if (!expected) continue;
    if (position.best_move === expected.best_move) bestMoveMatches += 1;
    if (position.score?.type === "cp" && expected.score?.type === "cp") {
      comparableScores += 1;
      if (Math.abs(position.score.value - expected.score.value) <= 75) scoresWithin75Cp += 1;
    }
  }
  return {
    reference_profile: reference.id,
    positions: profile.positions.length,
    best_move_matches: bestMoveMatches,
    best_move_agreement:
      profile.positions.length > 0 ? bestMoveMatches / profile.positions.length : 0,
    comparable_centipawn_scores: comparableScores,
    scores_within_75cp: scoresWithin75Cp,
    scores_within_75cp_rate: comparableScores > 0 ? scoresWithin75Cp / comparableScores : null,
  };
}

async function main() {
  const args = parseArgs(process.argv.slice(2));
  const engineDir = resolve(args.engineDir);
  const packageMetadata = JSON.parse(
    await readFile(join(engineDir, "package.json"), "utf8"),
  );
  const profiles = [];

  profiles.push(
    await runProfile({
      id: "native-deep-reference",
      kind: "native",
      depth: 18,
      create: () => createNativeSession(args.native),
    }),
  );
  profiles.push(
    await runProfile({
      id: "browser-quick-candidate",
      kind: "stockfish-js-lite-single",
      depth: 10,
      create: () => createWasmSession(engineDir, "lite-single"),
    }),
  );
  profiles.push(
    await runProfile({
      id: "browser-balanced-lite-candidate",
      kind: "stockfish-js-lite-single",
      depth: 14,
      create: () => createWasmSession(engineDir, "lite-single"),
    }),
  );
  profiles.push(
    await runProfile({
      id: "browser-balanced-full-candidate",
      kind: "stockfish-js-full-single",
      depth: 14,
      create: () => createWasmSession(engineDir, "single"),
    }),
  );

  const restartStart = nowMilliseconds();
  const restarted = await createWasmSession(engineDir, "lite-single");
  const restartVersion = await restarted.ready();
  const restartMs = nowMilliseconds() - restartStart;
  restarted.close();

  const reference = profiles[0];
  const output = {
    schema: "plywise-stockfish-profile-benchmark-1",
    recorded_at: new Date().toISOString(),
    host: {
      node_architecture: process.arch,
      platform: process.platform,
      node_release: process.release,
      node: process.version,
      hardware_note: "Apple M2 Pro, 12 cores, 16 GB RAM",
    },
    scope: {
      execution_host:
        "Node.js-hosted WebAssembly proxy for browser-worker compute; browser UI responsiveness and mobile compatibility are not measured",
      fixtures:
        "Public chess regression FENs covering opening, middlegame, castling, en passant, promotion, mate, repetition shape, and endgames",
      fixture_count: fixtures.length,
      caveats: [
        "Node.js startup excludes network transfer and HTTP cache behavior.",
        "Process RSS includes the Node.js host and is not a browser-worker peak-memory measurement.",
        "Classification agreement requires the future typed C++ observation-ingest path and is not claimed here.",
        "One machine is evidence for candidate selection, not the complete browser and device matrix.",
      ],
    },
    stockfish_js: {
      package: "stockfish",
      version: packageMetadata.version,
      license: packageMetadata.license,
      lite_single: await artifactMetadata(engineDir, "lite-single"),
      full_single: await artifactMetadata(engineDir, "single"),
    },
    native_executable: resolve(args.native),
    profiles,
    agreement: profiles.slice(1).map((profile) => ({
      profile: profile.id,
      ...agreement(profile, reference),
    })),
    restart: {
      profile: "stockfish-js-lite-single",
      engine_version: restartVersion,
      startup_ms: Number(restartMs.toFixed(2)),
      completed: true,
    },
  };

  const serialized = `${JSON.stringify(output, null, 2)}\n`;
  if (args.output) await writeFile(resolve(args.output), serialized);
  process.stdout.write(serialized);
}

main().catch((error) => {
  process.stderr.write(`${error.stack ?? error.message}\n`);
  process.exitCode = 1;
});
