import type { MoveAssessment } from "./types";

export type ReviewMode = "manual" | "playing" | "try" | "reveal" | "variation";

export const routineClassifications = new Set(["Book", "Best", "Excellent", "Good"]);
export const positiveClassifications = new Set(["Brilliant", "Great"]);
export const blockingClassifications = new Set(["Mistake", "Miss", "Blunder"]);

export function autoplayDelay(move: MoveAssessment | undefined): number | null {
  if (!move) return null;
  if (blockingClassifications.has(move.classification)) return null;
  if (positiveClassifications.has(move.classification)) return 1500;
  if (move.classification === "Inaccuracy") return 850;
  return 380;
}

export function firstReviewPly(moveCount: number): number {
  return moveCount > 0 ? 0 : -1;
}

export function clampPly(ply: number, moveCount: number): number {
  if (moveCount <= 0) return -1;
  return Math.max(0, Math.min(moveCount - 1, Math.trunc(ply)));
}

export function classificationCounts(moves: MoveAssessment[], side: "white" | "black"): Record<string, number> {
  const counts: Record<string, number> = {};
  for (const move of moves) {
    if (move.side !== side) continue;
    counts[move.classification] = (counts[move.classification] ?? 0) + 1;
  }
  return counts;
}

export function acceptableMoves(move: MoveAssessment | undefined): string[] {
  if (!move) return [];
  return Array.from(new Set([move.best_uci, ...move.acceptable_alternatives].filter(Boolean)));
}

export function isAcceptedTry(move: MoveAssessment | undefined, uci: string): boolean {
  return acceptableMoves(move).includes(uci.trim().toLowerCase());
}

export function isKeyMove(move: MoveAssessment | undefined): boolean {
  return Boolean(move && (blockingClassifications.has(move.classification) || positiveClassifications.has(move.classification)));
}
