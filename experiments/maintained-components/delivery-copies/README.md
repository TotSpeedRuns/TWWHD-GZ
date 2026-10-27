# Delivery Bag copies

This authored partial component adds one actual Delivery copy with Right and
removes one with Left. A retains its add/remove toggle. The displayed quantity
counts the eight physical bag slots, rather than showing one availability bit.
A full bag refuses another copy without changing its contents. Removing a copy
compacts the first matching slot and keeps its availability bit while another
copy remains.

`delivery.c` plans detached byte changes without globals, native calls or game
pointers. `delivery.h` defines its 24-byte input and 48-byte plan.
`delivery_bridge.s` copies current values only when called by the existing
guarded inventory dispatcher: r28 is its current owned save, r31 the selected
item and r9 the decoded edit action. It returns to the existing apply/grant
paths. It is not a standalone native entry or an ownership check. The original
user/live/current-save guards must remain before this branch.

The planner also describes selected-slot compaction/remapping for five saved
bindings and their runtime usable-item bytes. That behavior has detached PPC
tests; actual equipped Delivery remapping has **not** been tested in Cemu or on
Wii U. The native run used five unassigned saved bindings and checked that they
and the five existing cache bytes stayed unchanged.

## Source integration

`integration-seams.json` records four exact authored source edits: directional
input, guarded apply dispatch, actual-copy count and the help string. It records
original private source hashes separately from canonical LF publication hashes.
Only hashes and authored snippets are included; generated full runtime text,
retail instructions, artwork, captures, game files and private diagnostics are
not distributed here.

`apply_source.py` requires the exact separately owned reviewed R777 Personal
assembly, checks every source pin/seam, appends the small bridge and verifies
the complete resulting assembly hash before creating a new output. It refuses
wrong parents, ambiguous seams and existing outputs. It does not compile or
provide the other current build inputs. The complete reproducible integrated
export in this repository remains **v126**.

```sh
python apply_source.py --input /path/to/matching/runtime-personal.s --output /path/to/new/runtime-delivery.s
python -m unittest discover -s . -p test_source.py
```

The portable tests exercise source metadata and refusal behavior only. The
private compiled component passed 3,170 detached PPC checks; those are separate
from native acceptance and are not runtime ownership models.

## Bounded native evidence

One private Cemu process completed 23 controller-only cases: 21 Delivery edits
and two existing Spoils/Bait counter regressions. These covered repeated Moblin
letters, one distinct Father letter, eight-slot/full refusal, first-copy
compaction, last-copy availability cleanup and normal A toggles. All five
unrelated saved bindings/cache bytes stayed unchanged. Both neighbor counter
arrays returned to their observed beforeimages. All six original/disposable
normal files were unchanged, and the exact owned process was closed.

The first driver remains failed: after its 21 completed Delivery cases it tried
Spoils tile 6 instead of tile 9. A separately reviewed remainder driver completed
only the outstanding Spoils tile 9 and Bait tile 10 checks in the same attested
process. This is a combined completed cohort, not a relabeled passing first
report.

The Delivery contents/availability bits and tested neighbor counters were
cleaned through normal controls. Container/acquisition grants remained in the
disposable process RAM; this is not full RAM rollback or a durability test.
Native letter use, every supported item row, held-input repetition, glyphs,
equipped-letter remapping and Wii U behavior remain unverified. No save-state,
hardware-performance, bridge, boss, cutscene or music restoration claim follows
from these inventory tests.
