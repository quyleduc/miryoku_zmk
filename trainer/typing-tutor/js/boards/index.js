// Board registry. Add future models here (Iris, Lily58, custom, …) and they
// appear in the menu selector automatically.

import { board as corneLqe } from './corne-lqe2hc.js';
// Upstream Corne V4 remains in the source tree for attribution, but is not a
// selectable profile: its QMK/Vial layers do not match the user's ZMK.


/**
 * Playable boards only. When you add a new model:
 *  1. Drop its Vial export in layouts/<id>.vil
 *  2. Create js/boards/<id>.js from the corne-v4 template
 *  3. Import and push it onto BOARDS below
 *  4. Extend scripts/check-layout.mjs if the matrix shape differs
 */
export const BOARDS = [corneLqe];

export const DEFAULT_BOARD_ID = 'corne-lqe2hc';

export function listBoards() {
  return BOARDS;
}

export function listPlayableBoards() {
  return BOARDS.filter((b) => !b.comingSoon);
}

export function getBoard(id) {
  const playable = listPlayableBoards();
  return playable.find((b) => b.id === id)
    ?? playable.find((b) => b.id === DEFAULT_BOARD_ID)
    ?? playable[0];
}

export function boardLabel(board) {
  if (!board) return '';
  return `${board.name} · ${board.formFactor}`;
}

export function boardFullLabel(board) {
  if (!board) return '';
  return `${board.productName}, ${board.formFactor}`;
}
