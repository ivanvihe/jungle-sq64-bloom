#!/usr/bin/env node
import {readFileSync} from 'node:fs';
import {pathToFileURL} from 'node:url';
import {resolve} from 'node:path';

const webRoot = resolve(process.argv[2]);
const imagePath = resolve(process.argv[3]);
const load = path => import(pathToFileURL(resolve(webRoot, path)));
const {default: createUnicorn} = await load('vendor/unicorn-arm.js');
const {Machine} = await load('emulator/core.js');
const {loadImage} = await load('emulator/image.js');

const engine = await createUnicorn();
const midi = [];
const machine = new Machine(engine, await loadImage(new Uint8Array(readFileSync(imagePath))),
  {onMidi: message => midi.push(Array.from(message))});
const run = instructions => {
  while (instructions > 0) {
    const report = machine.run(Math.min(instructions, 200000));
    if (report.error) throw Error(report.error);
    instructions -= 200000;
  }
};
const press = control => {
  machine.event({type: 'button', control, down: true}); run(400000);
  machine.event({type: 'button', control, down: false}); run(400000);
};
const chord = (modifier, control) => {
  machine.event({type: 'button', control: modifier, down: true}); run(400000);
  press(control);
  machine.event({type: 'button', control: modifier, down: false}); run(400000);
};
const channelC = from => midi.slice(from)
  .filter(m => m.length >= 3 && (m[0] & 0xf0) === 0x90 && (m[0] & 15) === 2 && m[2])
  .map(m => m[1]);
const scalePcs = new Set([0, 2, 3, 5, 7, 9, 10]); // C Dorian

run(1500000);
press('pattern');
press('play');

const names = ['random-walk', 'weighted', 'markov', 'arpeggiator', 'harmonic'];
for (const name of names) {
  chord('shift', 'trackC');
  const mark = midi.length;
  run(8000000);
  const notes = channelC(mark);
  if (!notes.length) throw Error(`${name}: no channel-C notes`);
  if (!notes.every(note => scalePcs.has(note % 12)))
    throw Error(`${name}: emitted note outside C Dorian: ${notes.join(',')}`);
  console.log(`ok   ${name}: ${notes.length} notes, ${new Set(notes).size} pitches`);
}

// One more cycle returns to Euclidean, then random-walk is selected again.
chord('shift', 'trackC');
chord('shift', 'trackC');
let mark = midi.length;
run(8000000);
const evolving = channelC(mark);
if (new Set(evolving).size < 2) throw Error('random walk did not evolve before freeze');
chord('shift', 'stop');
mark = midi.length;
run(8000000);
const frozen = channelC(mark);
if (!frozen.length || new Set(frozen).size !== 1) throw Error('track C did not freeze to one pitch');
if (machine.report().critical.terminated) throw Error('critical hazard terminated the run');
console.log(`ok   freeze: ${frozen.length} repeated notes at ${frozen[0]}`);
console.log('browser generative-engine test passed');
machine.close();
