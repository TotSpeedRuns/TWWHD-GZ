# Ganondorf entrance and practice-travel gear cleanup

This is the authored component source for the scoped v142 change, based on
the exact private R757 runtime source. It is a partial export. The complete
reproducible source build in this repository remains v126; this folder does
not reproduce the complete v142 runtime or installer.

The Ganondorf raw destination uses the authored normal entrance
`GTower / room 0 / spawn 1 / layer 0` through all three existing aliases:
map record 116, Ganon text record 331 and Hyrule text record 383. The previous
spawn 0 selected the opening event. Selecting point 1 preserves the native
Zelda fight conversation; no dialogue code, story script or route payload is
removed. The existing Ganondorf checkpoint remains unchanged.

`gear_travel.s` clears only event `3F40`'s `0x40` bit when the user requests
an admitted practice raw warp or memfile load. Native sword/shield draw guards
read that bit from the current relocated game-info root. The leaf checks the
root's range/alignment and preserves every other event bit, all GPRs, full CR,
LR, CTR, XER and FP/GQR state. It uses one 32-byte stack frame, calls no native
API and creates no persistent storage or heap reservation.

The two call sites keep their original ROH, live, index, busy and route
admission checks. The helper runs before the existing travel request is
published. It is absent from capture, save-state restore, position restore,
native story travel, drawing and frame ticks. Legitimate captured cutscene
state must retain its own semantics. No model flag or general event bank is
cleared, and no model is manually rebuilt.

## Apply to the matching private source

```sh
python apply.py --edition personal --source /path/to/exact-r757-personal.s --output /path/to/new-personal.s
python apply.py --edition public --source /path/to/exact-r757-public.s --output /path/to/new-public.s
python test_apply.py
```

The applier requires the exact edition-specific input hash, exact leaf and
seam hashes, unique call sites and three original table lines. It proves a
complete reversal and checks the expected composed source hash before writing
a new file. It refuses an existing output and never writes its input. It does
not invoke a compiler, patch a game executable, access a process/console or
include a Cemu driver.

The qualified build also requires the private R757 link inputs, including the
matching bottle, Inventory-stick and quarter-heart components, native aliases,
builder and local assets. Those complete dependencies are not exported here.
`source-pins.json` records source, component and final-image hashes without
shipping the large generated runtime, objects or owned game bytes. The small
leaf and three-word edits do not replace the full runtime with an older copy.

## Evidence and limits

`evidence-summary.json` separates portable source-text checks, private emitted
PPC checks and genuine Cemu observations. Both complete private baseline
runtime objects reproduced byte for byte. The final change adds 128 bytes of
TEXT: a 112-byte leaf, two calls and alignment padding. All 803 route payloads,
298 native hooks, 80 GQR patches, other 73 linked roles and existing heap
reservations remain unchanged.

The three menu aliases reached the normal native point-1 pose. Native
`PZ_TALK` requested message 13666, identifying Zelda's bow-cover line; ordinary
A completed the conversation, and movement, sword attack and shield controls
worked. A disposable point-2 ending probe naturally raised the hide bit. Raw
travel and memfile travel returned it clear while retaining sword/shield IDs;
repeated raw travel left it clear. The probe changes only an entrance DATA word
and is not selected for the v142 images.

These observations do not establish rendered-pixel visibility, Wii U hardware
acceptance, every ending/progress path or arbitrary script reassertion timing.
The memfile payload also clears the byte, so its arrival test does not isolate
the helper's causal timing. Stale inactive event-name pointers are not event
identity. The first overly broad event assertion remains a recorded harness
failure, and recorded halfword message statuses are not byte enums. This adds
no save-state coverage, persistent slots or cross-area instant-restore claim.

No owned executable, retail code/signature, asset, save, live dump, private
Creator profile, credential or machine-local path is distributed in this folder.
