import { autoplayDelay, clampPly, classificationCounts, firstReviewPly, isAcceptedTry } from "../src/review";
import type { MoveAssessment } from "../src/types";

function assert(actual: unknown, expected: unknown, label: string): void {
  if (JSON.stringify(actual) !== JSON.stringify(expected)) {
    throw new Error(`${label}: expected ${JSON.stringify(expected)}, got ${JSON.stringify(actual)}`);
  }
}

const move = (overrides: Partial<MoveAssessment>): MoveAssessment => ({
  ply: 0,
  move_number: 1,
  side: "white",
  played_uci: "e2e4",
  played_san: "e4",
  san: "e4",
  fen_before: "",
  fen_after: "",
  best_uci: "e2e4",
  best_san: "e4",
  evaluation_before: 0,
  evaluation_after: 10,
  evaluation_after_best: 10,
  loss: 0,
  expected_points_before: 0.5,
  expected_points_after: 0.51,
  expected_points_loss: 0,
  quality: "Best",
  classification: "Best",
  classification_state: "final",
  classification_reasons: [],
  tactical_tags: [],
  principal_variation: [],
  acceptable_alternatives: [],
  phase: "opening",
  best_response: "",
  book_source: "",
  book_version: "",
  depth: 16,
  nodes: 1,
  time_ms: 1,
  multipv: 1,
  engine_version: "test",
  classification_model_version: "tutor-classification-1",
  ...overrides,
});

assert(firstReviewPly(18), 0, "new game starts after White move one");
assert(firstReviewPly(0), -1, "empty game has no selected ply");
assert(clampPly(99, 18), 17, "cursor clamps at end");
assert(autoplayDelay(move({ classification: "Book" })), 380, "routine move speed");
assert(autoplayDelay(move({ classification: "Great" })), 1500, "positive pause");
assert(autoplayDelay(move({ classification: "Inaccuracy" })), 850, "inaccuracy pause");
assert(autoplayDelay(move({ classification: "Blunder" })), null, "blunder blocks autoplay");
assert(isAcceptedTry(move({ best_uci: "e2e4", acceptable_alternatives: ["d2d4"] }), "d2d4"), true, "equivalent candidate accepted");
assert(classificationCounts([
  move({ side: "white", classification: "Best" }),
  move({ side: "black", classification: "Mistake" }),
  move({ side: "white", classification: "Best" }),
], "white"), { Best: 2 }, "classification totals are side-specific");

console.log("review state tests passed");
