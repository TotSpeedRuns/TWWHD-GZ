# Native bridge rewind

These authored C components implement capture and experimental rewind for the three suspension bridges in
the Dragon Roost Cavern Sickass Skip room. They are current partial components,
not a standalone game patch. The repository's complete integrated builder remains
v126. This source export adds no original game code, assets or memory captures.

The immediate defect was concrete: restoring the player's position reported
success while a naturally cut bridge remained broken. The new BRG3 component
stores the coupled CPU state needed by all three known bridge layouts, rather
than treating the bridge's action number as sufficient state.

| Native type | Planks | Saved scalar bytes | Saved geometry bytes |
| --- | ---: | ---: | ---: |
| 0 | 33 | 3,120 | 22,779 |
| 0 | 26 | 2,469 | 21,237 |
| 4 | 9 | 888 | 16,855 |

The complete three-row component uses 67,612 bytes. It includes current/back
collision vertices, planes, triangle links, blocks, groups and nodes; each plank's
physics values and model scale/matrix; supported rope centerlines and widths;
and the native background move counter. The format stores packed big-endian
values, hashes and identity keys. It does not serialize native resource pointers,
graphics descriptors or GPU buffers as values to replay.

Capture runs inside the existing genuine native read boundary and verifies the
complete cohort again before publishing it. Restore first binds every current
actor, heap, collision object, model and rope resource. It stages a complete
before-image, performs the CPU writes inside the held world transaction, verifies
the target, and has a reverse-order undo path. The final transaction must include
the bridge participant; the previous fourteen-target receipt cannot stand in for
it. Unknown layouts and missing geometry are rejected explicitly.

This first writer requires the original allocations to remain alive. It does not
recreate a deleted bridge, reload a room, or add the bridge writer to the separate
bomb-recreation transaction. Existing scalar-only archives still parse, but a
nonempty older bridge component must be recaptured to supply the geometry.

The normal native Draw path rebuilds active rope vertices from the restored CPU
centerlines, flushes the written backing and flips its existing buffer selector.
The restore preserves current graphics resources and lets that ordinary render
pass run; it does not call an entire Draw function during restore. This static
relationship is not proof of visible rendering or collision acceptance. Those
require actual engine tests, including a natural bridge break and repeated
restores. No such native acceptance result is included in this source packet.

The first private engine capture refused with code 1802 before publishing a
snapshot; no Restore was attempted. The failure was an alignment bug in this new
component: it submitted every six-byte collision-block value separately to an
allocation-binding API that requires a four-byte-aligned query address. The second
value therefore failed even though its complete native allocation was valid.

The corrected source binds the complete aligned descriptor envelope before its
byte-only value loop and validates that binding afterward. It keeps the allocation
API's alignment rule and the existing ownership checks. The same correction applies
to capture, verification and replay qualification. `descriptor-envelope-fix.patch`
and `.json` record the exact one-edit inverse; the other six exported source leaves
are unchanged.

A second private engine attempt completed both bridge capture and verification,
including the complete 67,612-byte component. It then crashed in snapshot
serialization before publishing the saved slot; no Restore was attempted. The
added length-query call had overwritten the link register in a planner that was
previously a leaf function. The adapter preserved its own return address correctly,
but the enclosing leaf had no frame preserving the outer caller's address.

`caller-LR-fix.patch` now reserves a 16-byte local frame around that call, saves
the incoming link register and live `r0`, and restores them immediately after the
adapter returns. The adapter's separate 128-byte frame is unchanged. All four new
calls were audited: the other three already execute inside original caller frames
that save and restore the outer return address. This repair adds 32 code bytes
and no initialized data or BSS. It changes no capture format or restore rule.

All three original parent objects were reproduced for the initial build. Each
later correction reproduced its actual parent object before replacing the one
input. The capture C-object/verbose-assembly parity passed; its single changed
function grew by 52 code bytes. The runtime correction received a separate emitted
review showing the exact eight-instruction addition and unchanged remaining code.
The corrected build published a real saved slot in Cemu and completed one quiet
restore with exact Link position and all fifteen target checks. Capture took
3.59 seconds including host readback. Repeating that saved state later refused:
one Fire Keese had naturally retired, leaving 418 of the 423 saved collision
objects. Recreating that owner is unfinished; the rows are not silently omitted.

A fresh capture after that retirement took 2.14 seconds. A natural bridge break
changed its action from 3 to 5. The bridge CPU forward pass and all fifteen target
checks passed, but the later transaction commit returned 40 and the real inverse
rolled the change back. That attempt exceeded the 30-second controller-observation
timeout. It is not an accepted bridge restore, a collision-traversal pass or a
visible-rendering pass. Repeated plan validation is being investigated separately.

The native attempts therefore expose useful defects and exercise capture,
physical target verification and undo, but Sickass is not yet reliably working.
No full-game, bomb, cross-room, cutscene, persistence or Wii U coverage is claimed.
This is experimental source, not a release recommendation.
The public copy was not separately recompiled. Full pins are in `source-pins.json`.

The measured object-level writable storage increase is 246,468 bytes: two banks
grow by 65,536 bytes together; capture workspace adds 17,096 BSS bytes; the bounded
replay plan adds 163,668 BSS bytes; and reports/state add 168 initialized bytes.
Read-only data adds 353 bytes. These are actual component section deltas, not a
claim about final compressed size or runtime speed. No large particle diagnostic
output is part of this bridge component. The observed restore delays are unacceptable for a practice tool; performance
and successful publication remain open work.

`integration.patch` records the unchanged eight changes to the enclosing C
owners. The exported capture source already includes the separate descriptor
envelope correction. The portable runtime map records nine assembly seams and
includes the caller-LR correction in its length-query seam; the two huge old
zero-bank strings are represented by hashes. These files
are deliberately not an automatic patcher.
`source/bridges3.s` is the additional adapter module. Its 128-byte ABI frames keep
the callee parameter area separate from saved live registers. The capture seam
reserves a separate 48-byte callee area as well. The new C leaves were compiled
with the existing soft-float flags and their emitted instructions were checked
for unexpected floating-point, paired-single and GQR use.

Integration needs the surrounding allocation, object/background lifetime,
resource-binding, archive, GPU-fence and world-transaction interfaces. The files
alone cannot supply those services. Native address aliases identify the game's
real resource-control slot and vertex-view vtable; they are not replacement
implementations. This export grants no ownership merely because a pointer or
archive hash is numerically valid.
