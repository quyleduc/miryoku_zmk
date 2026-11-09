import { test } from 'node:test';
import assert from 'node:assert/strict';
import {
  BOARDS,
  DEFAULT_BOARD_ID,
  getBoard,
  listPlayableBoards,
  boardFullLabel,
  boardLabel,
} from '../js/boards/index.js';
import { PRIMARY_BOARD, KEYS, charToKey } from '../js/keyboardLayout.js';

test('default selected board is LQE2HC Fast Base 36', () => {
  assert.equal(DEFAULT_BOARD_ID, 'corne-lqe2hc');
  const board = getBoard(DEFAULT_BOARD_ID);
  assert.equal(board.KEYS.length, 36);
  assert.match(board.productName, /LQE2HC/i);
  assert.match(board.formFactor, /3×5\+3/);
});

test('board labels include product identity for sharing', () => {
  const b = getBoard('corne-lqe2hc');
  assert.match(boardLabel(b), /Corne LQE2HC/);
  assert.match(boardFullLabel(b), /ZMK Fast Base/);
  assert.match(boardFullLabel(b), /36 keys/);
});

test('registry has at least the Corne and all playable boards resolve', () => {
  assert.ok(BOARDS.length >= 1);
  assert.ok(listPlayableBoards().every((b) => b.id && b.KEYS?.length));
  assert.equal(getBoard('missing-id').id, DEFAULT_BOARD_ID);
});

test('keyboardLayout facade matches the Corne board matrix', () => {
  assert.equal(KEYS.length, PRIMARY_BOARD.KEYS.length);
  assert.deepEqual(charToKey('a'), PRIMARY_BOARD.charToKey('a'));
  assert.equal(PRIMARY_BOARD.vilPath, 'layouts/corne-v4.vil');
  assert.equal(PRIMARY_BOARD.geometry, 'corne-3x6');
});

test('Corne CHAR_MAP is unique and maps hold layers', () => {
  assert.equal(PRIMARY_BOARD.assertUniqueCharMap(), true);
  assert.equal(PRIMARY_BOARD.LAYER_HOLD[1], 'L34');
  assert.equal(PRIMARY_BOARD.LAYER_HOLD[2], 'R34');
});
