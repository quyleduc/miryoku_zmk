import { test } from 'node:test';
import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import { getBoard, DEFAULT_BOARD_ID } from '../js/boards/index.js';
import { STAGES } from '../js/lqeLessons.js';

const board = getBoard(DEFAULT_BOARD_ID);
const c = (ch) => board.charToKey(ch);

test('exact 36 physical keys in two 3x5+3 halves', () => {
  assert.equal(board.KEYS.length, 36);
  assert.equal(board.geometry, 'corne-3x5');
  for (const h of ['L','R']) {
    const keys=board.KEYS.filter(k=>k.half===h);
    assert.equal(keys.length,18);
    for (const row of [0,1,2]) assert.equal(keys.filter(k=>k.row===row).length,5);
    assert.equal(keys.filter(k=>k.row===3).length,3);
  }
  assert.equal(board.assertUniqueCharMap(), true);
});

test('all seven layers and thumb holds match ZMK bindings', () => {
  assert.deepEqual(board.LAYER_NAMES,['Base','Nav','Mouse','Media','Num','Sym','Fun']);
  assert.deepEqual(board.LAYER_HOLD,{1:'L31',2:'L32',3:'L30',4:'R31',5:'R30',6:'R32'});
  for(const id of Object.values(board.LAYER_HOLD)) assert.ok(board.KEYS.find(k=>k.id===id));
  assert.equal(board.KEYS.find(k=>k.id==='L31').legends[0],'Space/Nav');
  assert.equal(board.KEYS.find(k=>k.id==='R31').legends[0],'Bksp/Num');
  assert.equal(board.KEYS.find(k=>k.id==='R30').legends[0],'Enter/Sym');
});

test('Base letters and punctuation never require layer hold', () => {
  assert.deepEqual(c('a'), {keyId:'L10',layer:0,shift:false});
  assert.deepEqual(c('j'), {keyId:'R11',layer:0,shift:false});
  assert.deepEqual(c(' '), {keyId:'L31',layer:0,shift:false});
  assert.deepEqual(c("'"), {keyId:'R14',layer:0,shift:false});
  assert.deepEqual(c(','), {keyId:'R22',layer:0,shift:false});
});

test('Num uses right BACKSPACE, Sym uses right ENTER, Nav uses left SPACE', () => {
  assert.deepEqual(c('7'),{keyId:'L01',layer:4,shift:false});
  assert.deepEqual(c('0'),{keyId:'L31',layer:4,shift:false});
  assert.deepEqual(c('['),{keyId:'L00',layer:4,shift:false});
  assert.deepEqual(c(';'),{keyId:'L10',layer:4,shift:false});
  assert.deepEqual(c('{'),{keyId:'L00',layer:5,shift:false});
  assert.deepEqual(c('}'),{keyId:'L04',layer:5,shift:false});
  assert.deepEqual(c('&'),{keyId:'L01',layer:5,shift:false});
  assert.deepEqual(c('('),{keyId:'L03',layer:5,shift:false});
  assert.deepEqual(c('←'),{keyId:'R11',layer:1,shift:false});
  assert.deepEqual(c('→'),{keyId:'R14',layer:1,shift:false});
});

test('Uppercase / double quote uses one-shot Shift on Nav + P', () => {
  const upper = c('A');
  assert.deepEqual(upper,{keyId:'L10',layer:0,shift:true,shiftKeyId:'R04',shiftHoldId:'L31'});
  assert.equal(c('"').keyId,'R14');
  assert.equal(c('"').shift,true);
  assert.equal(board.SHIFT_HOLD_ID,'L31');
});

test('matches known-good ZMK keymap source snapshot', () => {
  const keymap=readFileSync(new URL('../../config/corne.keymap',import.meta.url),'utf8');
  for(const x of [
    'display-name = "Base";', 'display-name = "Nav";',
    'display-name = "Mouse";','display-name = "Media";',
    'display-name = "Num";','display-name = "Sym";',
    'display-name = "Fun";',
    '&u_lt 1 SPACE', '&u_lt 4 BSPC', '&u_lt 5 RET',
    '&kp N7', '&kp LBRC', '&kp LEFT',
  ]) assert.ok(keymap.includes(x),'Expected '+x);
});

test('15 actionable stages and every stage character is mapped', () => {
  assert.equal(STAGES.length,15);
  assert.equal(new Set(STAGES.map(s=>s.id)).size,STAGES.length);
  for(const stage of STAGES) {
    assert.ok(stage.pool?.length>0,stage.id);
    for(const token of stage.pool) {
      for(const ch of token) {
        assert.ok(c(ch), `Unsupported character ${JSON.stringify(ch)} in ${stage.id}: ${token}`);
      }
    }
  }
});
