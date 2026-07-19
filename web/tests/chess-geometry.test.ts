import { applyUciLineToFen, applyUciToFen, moveOverlayGeometry, parseUciMove, squareGeometry } from "../src/chess";

function equal(actual: unknown, expected: unknown, label: string): void {
  if (JSON.stringify(actual) !== JSON.stringify(expected)) {
    throw new Error(`${label}: expected ${JSON.stringify(expected)}, got ${JSON.stringify(actual)}`);
  }
}

equal(parseUciMove("e7e8q"), { from: "e7", to: "e8", promotion: "q" }, "promotion parsing");
equal(parseUciMove("e4d5"), { from: "e4", to: "d5", promotion: null }, "capture-shaped move parsing");
equal(squareGeometry("a8", "white"), { column: 0, row: 0, size: 12.5, x: 6.25, y: 6.25 }, "white orientation a8");
equal(squareGeometry("a8", "black"), { column: 7, row: 7, size: 12.5, x: 93.75, y: 93.75 }, "black orientation a8");
equal(moveOverlayGeometry("c3d4", "white"), {
  from: "c3",
  to: "d4",
  promotion: null,
  source: { column: 2, row: 5, size: 12.5, x: 31.25, y: 68.75 },
  destination: { column: 3, row: 4, size: 12.5, x: 43.75, y: 56.25 },
}, "white overlay");
equal(moveOverlayGeometry("c3d4", "black"), {
  from: "c3",
  to: "d4",
  promotion: null,
  source: { column: 5, row: 2, size: 12.5, x: 68.75, y: 31.25 },
  destination: { column: 4, row: 3, size: 12.5, x: 56.25, y: 43.75 },
}, "flipped overlay");
equal(applyUciToFen("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1", "e1g1"), "r3k2r/8/8/8/8/8/8/R4RK1 b kq - 1 1", "king-side castling animation");
equal(applyUciToFen("8/P7/8/8/8/8/7p/8 w - - 0 1", "a7a8n"), "N7/8/8/8/8/8/7p/8 b - - 0 1", "underpromotion animation");
equal(applyUciToFen("8/8/8/3pP3/8/8/8/8 w - d6 0 1", "e5d6"), "8/8/3P4/8/8/8/8/8 b - - 0 1", "en passant animation");
equal(applyUciLineToFen("8/8/8/8/8/8/4P3/8 w - - 0 1", ["e2e4"], 1), "8/8/8/8/4P3/8/8/8 b - e3 0 1", "variation cursor line");

console.log("chess geometry tests passed");
