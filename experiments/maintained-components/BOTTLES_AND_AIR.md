# Bottle editor and Frame Advance air handler

This is an authored component source proposal. The full reproducible integrated
export remains v126; these files do not reproduce the full v140 runtime or its
installer. The existing R724 Testing Tools, Misc, Movement and Favorites sources
remain unchanged. Native Wii U qualification remains separate.

One actual Cemu normal-swimming test passed for the air handler: air remained
constant while paused, each of three granted frames consumed exactly one point,
and Resume restored ordinary draining. The separate draw function was unchanged.
Diving, drowning, every area and Wii U behavior remain unqualified. The earlier
gate at `0x026E6718` failed and is not part of the accepted handler fix.

## Bottle editor integration

`bottle_menu.c`, `bottle_menu.h` and `bottle_play.h` contain the guarded editor;
`bottle_bridges.s` contains its small input and dispatcher bridges. All four
original bottle slots use the ten existing content IDs. Inventory A opens page
60, Up/Down select, A queues one content request, and B returns to the selected
bottle. X gives an empty bottle into an absent slot or deletes a present bottle.
Content requests are 960 through 999; give/delete requests are 1000 through 1003.
No new bottle artwork, save packet or reserved heap is introduced.

The complete legacy runtime still needs seven explicit integration seams:

1. Include the authored C and bridges once in the existing aggregate compile.
2. Dispatch page-60 input only while the practice menu is open, preserving the
   existing D-pad repeat and configured Select/Back binds.
3. Let the inventory bridge handle only its four bottle rows and preserve the
   ordinary inventory continuation for all other rows.
4. Route page-60 drawing to `_gz_btl_draw` and bottle help to
   `_gz_btl_inventory_help`.
5. Include page 60 in the existing menu/hotkey ownership check.
6. Include page 60 in the existing D-pad repeat ownership check.
7. Route the new request interval to `_gz_btl_apply` only after the existing
   restore-operation, live-context and current-save guards have succeeded.

Calling `_gz_btl_apply` by itself does not grant native-write authority. Its
caller must retain the original game-callback path and validated current save
root. Input and drawing queue or display intent; they perform no inventory
writes. The inventory bridge's established caller provides edges in r8, held
input at r31, and preserves volatile registers around the C call. Page input
requires the existing stack/input register contract; the dispatcher receives
request r27 and the already qualified current save r28. Ordinary continuations
and the final mask/apply returns must resolve to the matching runtime labels.

The EUR v0 native code imports are exact whole-function entries:

| Symbol | Entry | Required operation |
| --- | --- | --- |
| `_gz_btl_native_history` | `0x025B5D7C` | Native acquisition flags |
| `_gz_btl_native_button` | `0x025B5538` | Existing equipped-button update, cleanup and notifications |
| `_gz_btl_native_timer` | `0x025B60E0` | Reset the one shared Forest Water timer |
| `_gz_btl_native_play` | `0x025200D8` | Whole current Play getter |

These are code imports, not native DATA aliases. The main adapter must pin the
owned EUR v0 implementation and retain all native call relocations. A different
game edition/version requires its own ABI and byte qualification. The current
GetPlay profile must match, its initialization word must be nonzero before any
call, and the whole API result/current roots must still match afterward. The
editor refuses a cold getter instead of invoking its native constructor.

Fresh Forest Water uses the native reset of 54,000 frames, then activates the
positive shared timer on the verified current Play object. The clock belongs to
the game, not an individual bottle. Other contents and deletion leave it alone.
Native pause, decrement and expiry remain the game's behavior. The Cemu content,
button/deletion, soup and bounded clock checks do not establish every bottle
use, full 30-minute expiry, every pause combination or hardware acceptance.

## Local-only GetPlay profile

The 88-byte retail instruction sequence is absent from this source tree.
Generate its local C initializer from your own exact original EUR v0 RPX:

```sh
python tools/bottle_play_profile.py --retail-rpx /path/to/owned/cking.rpx
```

The tool uses `patcher/native_rpx.py`, authenticates the original RPX and the
bounded profile against `bottle_play_profile.json`, and writes only ignored
`local-assets/bottle_play_profile.inc`. It refuses a different existing output.
Do not commit that generated file or the input game. Compile the bottle module
with `-I local-assets` in addition to the maintained-component include path.
Expanding that include restores the exact R3 comparison declaration and does not
replace any current-root or initialization predicate with a hash result.

Use the existing freestanding PPC soft-float flags, including
`-fno-merge-constants`; the payload exporter requires ordinary section-relative
relocations. The published v126 builder does not automatically wire this module.

## Frame Advance air-handler integration

`frame_advance.c` is the existing R724 control module, retained byte for byte.
`frame_advance_air.s` adds 28 bytes of authored code and no DATA or BSS. It gates
the complete countdown-state handler at `0x026E743C`, whose first instruction is
`mflr r0`. With `_gz_fad_forced != 0` the wrapper returns before native state
updates. Otherwise it replays `mflr r0` and tail-branches to the original
prologue continuation `0x026E7440` (`stwu r1,-24(r1)`). The caller's LR and r3
`this` argument are retained. Bind `_gz_fad_air_resume` to that exact continuation.

The existing completed-frame boundary owns `_gz_fad_forced`: a granted step
clears it and the next paused boundary restores it. No new thread or synthetic
native result is introduced. The separate native draw entry at `0x026E66E4`
remains untouched, as does the ineffective higher entry at `0x026E6718`.
Install only the one new whole-handler hook; do not retain the failed higher
entry hook or add a second air gate.

Either compile the assembly wrapper as a separate object or include it once
from the exact current C source with the assembler include directory. For the
include approach, use `-Wa,-I,experiments/maintained-components` and an authored
assembly include at the end of C. Do not both include and separately compile it.
All 17 existing C function bodies, their original relocation identities and
DATA/BSS must remain exact. Final hook branches, prologue, draw bytes, relocation
coverage and storage bounds still need a full integrated image check.

140 emitted component cases exercise the actual existing control boundary and
wrapper at two placements. Pause returns without stores; Off/resume/grants stop
at the real untouched continuation with the displaced LR value. They do not run
the native countdown or supply API results. The separate root Cemu test above
established the stated three-step behavior, not every state or visual path.
