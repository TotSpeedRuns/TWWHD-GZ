# Quarter-heart-piece editor

This authored component comes from private R755, tested in Cemu R756. It is
separate from the full reproducible v126 export and does not reproduce the
complete current runtime or installer. No game binary, retail instruction
profile, captured state, artwork or diagnostic dump is included.

On Inventory's Heart Containers entry (row 25), X cycles piece progress through
0/4, 1/4, 2/4 and 3/4. Returning to 0/4 preserves the number of whole hearts;
Left, Right and A retain the existing whole-container controls. At 20 hearts,
X does nothing. The native text queue displays `Pieces: n/4 (X)` below the
container value. The current-health editor and Y shortcut to Refills remain.

## Native behavior and integration

`heart_pieces.c` posts one signed delta to the current native Play object's
pending maximum-life field. The ordinary native meter then applies the saved
maximum, usable health ceiling, refill/clamp, HUD update and pending retirement.
It does not call item acquisition callbacks or alter their lifetime statistics,
chest or stage reward flags. Positive piece changes refill through that native
path. This is an inventory edit, not an emulated item reward event.

The handler retains the original restore-owner, live-context, heap, current
save and UI checks. Its inherited whole-entry GetPlay validation requires the
exact initialized native implementation and relocated DATA roots. It refuses
pending life/max pickups, minigame type 6 and the native battle-reset mode;
it does not discard or merge a pending pickup.

Use `integration-seams.json` and its readable diff to review the four runtime
changes and appended labels against the exact parent hash. They add row-25 X
request 948, dispatch it after the existing write guards, draw piece progress,
and make Refill Health request 703 use the full-heart ceiling. Maximum values
13, 14 and 15 therefore refill to 12 quarter-hearts; 16 refills to 16.
The input seam stores the newly selected row before queueing, including when
navigation and X arrive together.

Compile the module once and combine it into the existing runtime object. Do
not also add it as a separate main component. The selected private integration
retained all 73 other object roles, including current bottles, inventory stick
navigation and MSS. No hook, import, BSS or heap reservation was added; the
integrated game image grew by 2,048 bytes. The complete legacy runtime and all
of its private integration dependencies are not exported by this folder.

The header's retail profile is a local-only include, generated from the exact
owned EUR v0 input with the existing repository tool:

```sh
python tools/bottle_play_profile.py --retail-rpx /path/to/owned/cking.rpx
```

Compile with `-I local-assets` and this directory's include path. Expanding
`bottle_play_profile.inc` restores the exact validated private header. Keep the
existing freestanding PPC 750 soft-float flags, `-fno-merge-constants` and symbol
integration. The existing native GetPlay import is the whole entry at
`0x025200D8`; a cold getter remains refused. The unused bottle timer helper in
the inherited header is not part of this feature.

## Validation and limits

One isolated controller-only Cemu session passed 22 cases: complete normal and
native-Plus-paused piece cycles while GZ stayed open, pending retirement,
positive changes from underfull health, refill limits, simultaneous navigation
and X, existing container controls and the 20-heart no-op. The checked native
acquisition statistic and stage reward fields stayed unchanged. The native
text queue contained the expected labels; pixel rendering was not checked.

Two subsequent placed-bomb expiry/restoration regressions passed with exact
saved fuse and poses, new native actor IDs and no room load. Their wall times
were about 2.39 and 2.30 seconds; this menu change is not a restore-speed fix.
Original ROM, source and disposable native save-file hashes stayed unchanged,
and the single owned emulator was closed afterward.

The component also passed 1,882 emitted-code cases and exact object/relocation
reviews. Those offline cases do not establish native pickup-pending, minigame
or battle-reset refusal behavior. Wii U acceptance, every health/UI mode,
source-free partial recovery, complete actor/global state, cross-area/death/
cutscene restoration and durable ten-slot coverage remain unproved.
