const pieces: Record<string, string> = {
  K: "♔",
  Q: "♕",
  R: "♖",
  B: "♗",
  N: "♘",
  P: "♙",
  k: "♚",
  q: "♛",
  r: "♜",
  b: "♝",
  n: "♞",
  p: "♟",
};

export interface BoardSquare {
  name: SquareName;
  piece: string;
  file: BoardFile;
  rank: BoardRank;
}

export type BoardFile = "a" | "b" | "c" | "d" | "e" | "f" | "g" | "h";
export type BoardRank = "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8";
export type SquareName = `${BoardFile}${BoardRank}`;
export type BoardOrientation = "white" | "black";
export type PromotionPiece = "q" | "r" | "b" | "n";

export interface BoardPoint {
  x: number;
  y: number;
}

export interface SquareGeometry extends BoardPoint {
  column: number;
  row: number;
  size: number;
}

export interface UciMove {
  from: SquareName;
  to: SquareName;
  promotion: PromotionPiece | null;
}

export interface MoveOverlayGeometry extends UciMove {
  source: SquareGeometry;
  destination: SquareGeometry;
}

const files = ["a", "b", "c", "d", "e", "f", "g", "h"] as const;
const ranks = ["1", "2", "3", "4", "5", "6", "7", "8"] as const;

export function squaresFromFen(fen: string, orientation: BoardOrientation = "white"): BoardSquare[] {
  const placement = fen.split(" ")[0] ?? "8/8/8/8/8/8/8/8";
  const fenRanks = placement.split("/");
  const byName = new Map<SquareName, BoardSquare>();
  fenRanks.forEach((rankData, rankIndex) => {
    let file = 0;
    for (const value of rankData) {
      if (/\d/.test(value)) {
        const count = Number(value);
        for (let offset = 0; offset < count; offset += 1) {
          const boardSquare = square(file + offset, 7 - rankIndex, "");
          byName.set(boardSquare.name, boardSquare);
        }
        file += count;
      } else {
        const boardSquare = square(file, 7 - rankIndex, pieces[value] ?? "");
        byName.set(boardSquare.name, boardSquare);
        file += 1;
      }
    }
  });
  return Array.from({ length: 64 }, (_, index) => {
    const name = squareAtBoardIndex(index, orientation);
    return byName.get(name) ?? { name, piece: "", file: name[0] as BoardFile, rank: name[1] as BoardRank };
  });
}

function square(file: number, rank: number, piece: string): BoardSquare {
  const fileName = files[file] ?? "a";
  const rankName = ranks[rank] ?? "1";
  return { name: `${fileName}${rankName}`, piece, file: fileName, rank: rankName };
}

export function uciSquares(uci: string): [string, string] | null {
  const move = parseUciMove(uci);
  return move ? [move.from, move.to] : null;
}

export function parseUciMove(uci: string): UciMove | null {
  const match = /^([a-h][1-8])([a-h][1-8])([qrbn])?$/.exec(uci.trim().toLowerCase());
  if (!match) return null;
  return {
    from: match[1] as SquareName,
    to: match[2] as SquareName,
    promotion: (match[3] as PromotionPiece | undefined) ?? null,
  };
}

export function squareAtBoardIndex(index: number, orientation: BoardOrientation = "white"): SquareName {
  if (!Number.isInteger(index) || index < 0 || index >= 64) {
    throw new RangeError("Board index must be an integer from 0 through 63");
  }
  const displayRow = Math.floor(index / 8);
  const displayColumn = index % 8;
  const fileIndex = orientation === "white" ? displayColumn : 7 - displayColumn;
  const rankIndex = orientation === "white" ? 7 - displayRow : displayRow;
  return `${files[fileIndex]}${ranks[rankIndex]}`;
}

export function squareGeometry(
  name: SquareName,
  orientation: BoardOrientation = "white",
  boardSize = 100,
): SquareGeometry {
  if (!isSquareName(name)) throw new RangeError(`Invalid chess square: ${name}`);
  if (!Number.isFinite(boardSize) || boardSize <= 0) throw new RangeError("Board size must be positive");

  const fileIndex = name.charCodeAt(0) - 97;
  const rankIndex = Number(name[1]) - 1;
  const column = orientation === "white" ? fileIndex : 7 - fileIndex;
  const row = orientation === "white" ? 7 - rankIndex : rankIndex;
  const size = boardSize / 8;
  return { column, row, size, x: (column + 0.5) * size, y: (row + 0.5) * size };
}

export function moveOverlayGeometry(
  uci: string,
  orientation: BoardOrientation = "white",
  boardSize = 100,
): MoveOverlayGeometry | null {
  const move = parseUciMove(uci);
  if (!move) return null;
  return {
    ...move,
    source: squareGeometry(move.from, orientation, boardSize),
    destination: squareGeometry(move.to, orientation, boardSize),
  };
}

export function isSquareName(value: string): value is SquareName {
  return /^[a-h][1-8]$/.test(value);
}

/** Applies a legal UCI move to a FEN for local review animation. The server remains the rules authority. */
export function applyUciToFen(fen: string, uci: string): string | null {
  const move = parseUciMove(uci);
  if (!move) return null;
  const fields = fen.trim().split(/\s+/);
  const board = new Map<SquareName, string>();
  for (const square of squaresFromFen(fen)) if (square.piece) board.set(square.name, unicodeToFen(square.piece));
  const piece = board.get(move.from);
  if (!piece) return null;
  const targetWasEmpty = !board.has(move.to);
  board.delete(move.from);
  if (piece.toLowerCase() === "p" && move.from[0] !== move.to[0] && targetWasEmpty && fields[3] === move.to) {
    const capturedRank = String(Number(move.to[1]) + (piece === "P" ? -1 : 1)) as BoardRank;
    board.delete(`${move.to[0]}${capturedRank}` as SquareName);
  }
  if (piece.toLowerCase() === "k" && Math.abs(move.from.charCodeAt(0) - move.to.charCodeAt(0)) === 2) {
    const kingSide = move.to[0] === "g";
    const rank = move.from[1] as BoardRank;
    const rookFrom = `${kingSide ? "h" : "a"}${rank}` as SquareName;
    const rookTo = `${kingSide ? "f" : "d"}${rank}` as SquareName;
    const rook = board.get(rookFrom);
    if (rook) { board.delete(rookFrom); board.set(rookTo, rook); }
  }
  const placed = move.promotion ? (piece === piece.toUpperCase() ? move.promotion.toUpperCase() : move.promotion) : piece;
  board.set(move.to, placed);
  let castling = fields[2] ?? "-";
  if (piece === "K") castling = castling.replace(/[KQ]/g, "");
  if (piece === "k") castling = castling.replace(/[kq]/g, "");
  const rookRights: Record<string, string> = { a1: "Q", h1: "K", a8: "q", h8: "k" };
  for (const square of [move.from, move.to]) castling = castling.replace(rookRights[square] ?? "", "");
  const fromRank = Number(move.from[1]);
  const toRank = Number(move.to[1]);
  const enPassant = piece.toLowerCase() === "p" && Math.abs(fromRank - toRank) === 2
    ? `${move.from[0]}${(fromRank + toRank) / 2}`
    : "-";
  const side = fields[1] === "b" ? "w" : "b";
  const halfmove = piece.toLowerCase() === "p" || !targetWasEmpty ? 0 : Number(fields[4] ?? 0) + 1;
  const fullmove = Number(fields[5] ?? 1) + (fields[1] === "b" ? 1 : 0);
  return `${fenPlacement(board)} ${side} ${castling || "-"} ${enPassant} ${halfmove} ${fullmove}`;
}

export function applyUciLineToFen(fen: string, moves: string[], count = moves.length): string {
  let current = fen;
  for (const move of moves.slice(0, Math.max(0, count))) current = applyUciToFen(current, move) ?? current;
  return current;
}

function unicodeToFen(piece: string): string {
  const entry = Object.entries(pieces).find(([, symbol]) => symbol === piece);
  return entry?.[0] ?? "";
}

function fenPlacement(board: Map<SquareName, string>): string {
  const rows: string[] = [];
  for (let rank = 8; rank >= 1; rank -= 1) {
    let row = "";
    let empty = 0;
    for (const file of files) {
      const piece = board.get(`${file}${rank}` as SquareName);
      if (!piece) { empty += 1; continue; }
      if (empty) { row += String(empty); empty = 0; }
      row += piece;
    }
    if (empty) row += String(empty);
    rows.push(row);
  }
  return rows.join("/");
}
