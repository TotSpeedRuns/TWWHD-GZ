# Maintained source components

These authored components follow current development independently of the
older integrated runtime export. `docs/revision.json` still identifies the
reproducible full v126 export. Updating this folder does not claim that v126
contains these changes or that the entire newer runtime has been exported.

## Current menu and capture changes (R677)

`creator_visibility.c` adds the last main-menu row, Show Creator Code.
Personal defaults visible; Public defaults hidden. Both can toggle the caption
and number during a session. The Creator beacon and transport are unchanged.
The runtime places the caption/number below the main options to avoid overlap.

`movement_turn.c` queues left/right quarter-turn intent from the Movement page.
Only the native game callback resolves the current unique Link process, checks
its live identity/extent and writes the five named current/old/shape yaw scalars.
Position, velocity, speed, actor pointers and save bytes are retained. Six
ordinary native Cemu turns, inverses and four-turn wrap passed with unchanged
position/velocity. These are not typed keyboard or speed-angle editors.

`favorites_storage_r562.c` retains the existing two checksummed preference
files but moves filesystem operations to one private worker. The game callback
is the sole live-list writer; the worker consumes and returns copied packets.
A sleeping message queue avoids idle polling; the main callback never waits
for file I/O. One job is queued at a time, with session-only fallback after a
stalled request. Buffers and worker stack have SDK alignment. Missing-file
errors are explicitly returned; the normal game save is never opened.

Actual Cemu add/remove/held-X, list loading and fresh-process persistence pass.
The initial lower-priority worker was starved; the accepted worker receives a
brief service opportunity above its creating thread and blocks when idle.
The reported native Wii U v138 freeze still requires console verification.

`save_state/ui_capture_gate.c` now distinguishes legacy capture from selected
slot capture for retry/cancellation. The runtime opens the chooser before world
capture. Only confirming a slot captures and publishes it. Opening/canceling
with placed and genuinely held bombs, including explosions while the chooser
remains open, made no capture calls and left the existing slot unchanged in
Cemu. Two placed-bomb expiry/recreation cycles passed with exact fuse/pose and
no scene loading. This does not prove complete held-item/effect capture or fix
all native Wii U bomb failures. No new retail hook or reserved heap was added.

## Current logical collision-background projection (R650)

`rito_physics_state.c` records a versioned wall-query tuple in the existing
28 padding bytes of its832-byte packet. This distinguishes an explicitly
empty constructor cache from missing historical metadata. Legacy RPH1 keeps
its original strict path; missing data never gains rebirth admission.

`background_owner_relation.c` finds immutable placement candidates without
following historical pointers. The native service separately rediscovers the
current actor queue, process generation/class and BG registration. It requires
a unique current owner and the exact collision resource signature; wall
queries additionally retain exact resource, vtable and triangle count checks.
Relocated or changed geometry remains unsupported by this exact contract.

`rito_background_target.c` projects only a private target copy. Original source
PID/generation/frame fields and the archived snapshot remain unchanged. The
typed writer covers four polygon query triples and the working wall-cache
triple, plus existing named numeric members. Vtables, arbitrary pointers and
self links are not written. The independent current before-image supports
source-free undo. Packet, private-region and reserved-heap sizes do not grow.

Validation:32 emitted pure placement-relation cases and26 packet-format cases
pass without native API doubles. Independent live observations found unique
recreated island and raft owners with unchanged resources. In the real native
drowning test, the Rito's four controllers, behavior, model, texture, Tetra
dependency and collision plan all prepared successfully. Full restoration
still refuses1182 at the later contact-cohort binding, before forward mutation.
Reborn collision forward/undo is consequently not claimed tested end-to-end.

The exact R650 image passed two ordinary placed-bomb expiry/recreation cycles,
with new native actor IDs, exact saved fuse57 and position, no room/route load
and unchanged native save hashes.811 integrated image/layout checks pass.
The public edition changes only the four-byte Creator drawing entry. Wii U
verification and full-game coverage remain incomplete.

The user separately approved the previously blocked foliage late-failure
diagnostic. Genuine final-service failure reached native rollback; internal
full-manager/Link/camera before-image checks passed, the owner released, and
a normal retry succeeded without a room load. A separate unpaused room-order
read taken outside the held native frame differed, so independent external
order acceptance remains unresolved. No diagnostic admission is enabled in
the production R650 image.

## Historical Rito placement and Tetra preparation (R638)

`save_state/rito_service_prepare.c` independently rediscovers a matching Rito
from the current native actor queue under the existing real NSV heap/source
hold. Exact surviving processes retain a separately rechecked strict path.
A different process lifetime needs a unique immutable placement match, the
original authenticated source/birth records and the prepared current player
context. Source packet PID/generation/frame fields remain unchanged.

`rito_source_identity.c` stores a small sealed receipt in existing private
row metadata. Each skeletal, behavior, model, texture and physics leaf still
checks the current process, allocation, controller and resource contracts.
The receipt is not lifetime, heap, writer or publication authority. Original
source packet joins and copied private target hashes remain intact.

The Rito's Tetra PID reference is independently matched by Tetra placement,
constructor readiness, current queue membership and process extent. Its new
PID is retained as a current reference; no old PID or pointer is assigned.
`rito_child_relation.c` is the pure candidate check. Existing row padding
holds the dependency receipt, so no archive bank or reserved heap grows.

Validation:33 emitted-code receipt checks and25 emitted Tetra placement cases
pass using real capture data. The intermediate native R631 test progressed
from refusal1433 to the child-reference check1391. R635 then passed all four
controllers, behavior, model and texture preparation after real drowning,
with Tetra502 matched to825 and Rito505 to828. The original source stayed
unchanged and no restore-triggered room loading occurred. Full restoration
still refuses1481 at Rito collision/background-reference qualification.

A subsequent bomb regression exposed a new1430 context refusal: the bomb
service prepares NPCs before the separate player scene-role plan exists.
R638 corrects this only for an exact surviving process, with fresh queue,
PoEntry and native header checks. Two original-save native bomb explosion/
recreation cycles then passed with new actor IDs532/537, exact fuse60 and
position, no room loading and unchanged native save hashes. The earlier
1430 refusal is preserved. No full-game or Wii U acceptance is implied by
this source export.805 image/layout checks pass.

These are component files from the assembled native runtime, not a complete
new integrated build export. The full reproducible export remains v126.
No new hook, reserved heap bank or route checkpoint is added by this change.

## Foliage native preparation and order replay (R621), Rito candidate (R628)

`save_state/foliage_journal.c` now independently allocation-binds the current
manager roots after a same-area native scene rebirth. The source/world context
must agree with the already prepared current player, exact owner token,
source pin, stage, room and layer. This does not adopt a saved pointer as a
current allocation. Surviving managers retain the original birth-before-source
check; reborn managers use fresh observed current allocation generations.
Separate plant actors retain their original strict lifetime checks.

The topology masks distinguish registered numeric units from opaque unused
pool payloads. A typed bounded-room writer generates each active member from
the current pool base and a checked logical slot. Linked heads/next pointers
must already represent the same logical graph. No source pointer, vtable,
manager header or unused member storage is installed. The separately captured
current order is retained for source-free undo.

Validation:22 captured-data topology cases,2,377 integer pointer-recipe cases
and798 integrated layout/relocation checks pass. One actual native Outset
restore replayed two changed room orders covering68 members and passed its
final typed checks. After natural drowning, all four fresh foliage managers
and1,178 registered entries passed preparation. Whole restoration still
refused1433 at the recreated Rito process join, before forward mutation.
Two matched original-save bomb expiry/recreation cycles then passed with the
saved fuse59, exact pose and no room loading. Native save hashes stayed intact.

Independent external undo acceptance for the new order writer remains
pending. The subsequently user-approved native late-failure test is described
above. Ordinary production code keeps diagnostic fault admission disabled.

The broader tests retain two additional failures: Northern Fairy Island has
an unimplemented warship profile179 during capture; an Outset travel/bomb
scenario refused1481 at the Rito physics-reference join. These are not waived
or counted as passing coverage.

`save_state/rito_role_relation.c` adds a pure captured-data candidate check.
Two genuine snapshots show the same immutable Rito placement at a reused
address, with new PID/generation and scene epoch. It passed28 emitted-code
cases. A candidate grants no native lifetime, replay or publication authority.
Its four controllers, fresh resources, models, texture, physics and dependent
actor references still need integration. Canonical source packet identities
remain unchanged.

No new retail hook, heap reservation or snapshot bank size is introduced.
Wii U acceptance and full post-void restoration are still unproven.

## MSS native pause duration (R617)

`mss_pause_frames.c` measures completed frames inside the actual settled
native Plus-menu state. The existing completed-frame callback calls
`_gz_mpd_tick`; `_gz_mss_draw` calls `_gz_mpd_text` below its coaching line
at pixel position16,584 using the existing small native font path.

The live label uses `|| Pause: Nf`; the retained result uses
`> Last pause: Nf`. Two frames are labeled `Insta`; three frames are marked
`3f target`, following the runner's requested practice target. The separate
Slow/Fast event-order classifier and gameplay buffer counter are unchanged.
Opening `ToMenu` and closing `ToPlay` animation frames are excluded. The
counter preserves an observed gameplay baseline through the opening, then
publishes the measured visit at closing. GZ menus, scene changes, unknown
states and missing callbacks cancel incomplete visits; enabling the trainer
inside a pause does not invent its beginning.

No native menu, timer, player or save byte is written. The feature adds no
retail hook or heap reservation. Counting is integer-only and performs no
allocation. Nineteen emitted-code counting checks and796 integration checks pass.
Seven actual native menu visits passed on land and while swimming, including
exact two-frame Insta and three-frame target results, a longer eight-frame
visit, retained results and GZ-menu exclusion. Native save hashes stayed
unchanged. Wii U verification is pending.

## Scene-rebirth preparation (R604) and foliage topology plan (R612)

The player and camera participants keep immutable source identity separate
from current native roots, complete observed process extents, fresh heaps
and resources. A newly created player/camera is not treated as the previous
allocation merely because its address is reused. Source archives are not
rewritten to match the current process IDs.

`save_state/camera_process_identity.c` adds a versioned96-byte CPI1 process
audit in unused padding at1760 of the existing2048-byte camera bank. Its
numeric validator grants no native lifetime or replay authority. Legacy
records without this evidence retain the same-camera requirement. The
current camera is independently discovered from the native camera table;
its source Link reference is rebound to the current native player role.
The typed inverse still targets the current camera's sealed before-image.

The private integration calls the capture audit from our authored
`_gz_camera_read`, after its existing UI validation and before its valid
marker. This is an internal callback, not an additional retail-game hook:

```asm
mr 3,31
bl _gz_cct_capture_identity
cmpwi 3,0
bne _gz_camera_read_bad
```

`save_state/light_replay.c` can bind a genuinely empty light bank after a
scene rebirth using the already prepared player and exact current token/pin.
It still enumerates the current actor queue; an added light cannot pass as
an empty bank. Nonempty light lifetimes need separate mapping.

Validation:25 emitted-code camera format/relation checks and790 integrated
layout/relocation checks pass. One actual Cemu case passed ordinary capture
and restore, then native drowning return recreated both player and camera.
Camera, player, face and empty-light preparation all passed from the original
unchanged source. Whole post-void restoration still refused at foliage root
identity; no successful whole restore is claimed. Two subsequent native
placed-bomb explosion/recreation cycles passed with new process IDs, the
saved fuse and pose, and no scene loading. Wii U testing is pending.

`save_state/foliage_manager_topology.c` is a separate pure serialized-data
planner. It does not read native objects or grant ownership. It maps the
registered slot masks and checks placement, internal linked-list indices,
bounded room membership and duplicates. Unregistered slot payloads remain
opaque and must not be replayed. Nonzero unregistered state is refused.
Source and current buffers are never modified.

Matching room membership with changed order returns1607, requiring an
explicit owned pointer-order writer and inverse before publication. It is
not an ordinary READY result. In paired actual pre/post-void captures, the
planner identifies1178 registered entries across four relocated managers;
one59-member tree-room list requires order replay. It passed22 emitted-code
metadata cases, including invalid links, cycles, duplicate members, stale
pointer bases, opaque unused positions and unregistered-state rejection.
At R612 it was not yet integrated into native foliage restoration; R621 above
adds the participant path. Separate plant
actors and renderer/resource/lifetime closure remain required.

These are reviewed components, not a replacement integrated-runtime export
or a claim of complete cross-area/death/void restoration. The reproducible
full build export remains v126. No new native hook, archive-size growth or
extra heap reservation is introduced by the R604 camera metadata change.

## Frame advance (R595)

`frame_advance.c` adds an opt-in Frame Advance page. D-pad Down pauses or
advances one simulation frame after release; D-pad Up resumes. The default
is Off. Requests are queued from controller input and applied by the existing
completed-frame callback. The small top-right HUD uses native-font `||`
and `>` pause/play markers. A step keeps its play indication for ten display
frames, then returns to paused. No image asset or extra heap reservation is
introduced.

The native pause hook is at the verified complete function entry. When not
forcing pause, its assembly wrapper replays the exact displaced first
instruction and resumes the original function. Native UI queries keep Plus
inventory navigation independent. Menu, save/restore, shoulder shortcuts,
Zombie Hover Auto start and active Upcharge bindings retain precedence.

Paused controller polling keeps the native game's held-button history at
the last simulated input. Fresh action edges are retained for the granted
frame; action buttons and floating-point stick bytes are handled separately
from the reserved Up/Down commands. VPAD and Pro transports have different
button encodings. The transport routines preserve unrelated sample fields,
validate the read count and do not accept archive-supplied native pointers.

Validation: both integrated editions passed 788 layout/relocation checks;
132 emitted-code checks cover control state, UI ownership, exact transport
fields and indicator timing. One actual Outset Cemu case passed live bomb
fuse stepping, held Down, stick movement, stepped sword-draw animation,
native world-clock pause, native Plus routing, capture/restore while paused,
resume and disabling the tool. Its native font queue contained all three
pause/play/step labels at the intended coordinates. This is not a screenshot
or Wii U visual acceptance claim. Pro hardware and all boss/scene/tool
combinations remain untested. Cross-area/void restoration and durable state
slots remain separate unfinished work.

`frame_advance.s` is the small input/hook adapter. The private integration
also wires the root/page draw, controller batch paths and frame callback;
these component files are not a replacement full-runtime build recipe.

## Swimming air rewind (R574)

`save_state/` contains the current authored clock/air packet and its restore
participant. Version 2 adds the exact current air meter, maximum and byte
status in previously reserved words of the same 128-byte bank. Version 1
remains valid and restores only its five recorded clock fields; absent air
data is never invented. Capturing a new state is required to record air.

The fixed root kernel reads and applies the air scalars under the existing
frame/thread/root/heap ownership checks. The before-image and inverse cover
the same fields. No native pointer is accepted from the archive, no new hook
is installed, and no extra heap reservation is introduced. These files are
components of the newer private integration, not a standalone build recipe.

Validation: both final images passed 776 layout/relocation checks; 26
emitted-PPC format checks cover the two packet versions and invalid inputs.
Actual Cemu swimming capture followed by natural air drain and three restores
returned the meter exactly to 882/900 at the native restore boundary, then
normal draining resumed. No room load occurred. This does not establish full
swimming effect replay, drowning, cross-area recovery or Wii U acceptance.

## Compact raw stick display (R546)

`input_float.c` formats the original normalized f32 LX/LY/RX/RY sample bits.
It rounds to at most nine fractional digits, trims trailing zeros, retains
one decimal digit and preserves negative zero. It has four separate 24-byte
buffers, no libc dependency, no allocation and no gameplay stores.

`input_float_draw.s` samples all four components before formatting. The
existing small native font places four rows beside the controller display.
The private integration calls `_gz_ivf_draw` near the end of
`_gz_gamepad_draw`; it keeps the existing input snapshot and trainer ABI.
The compiler/linker must retain the component's ordinary data relocations.

Validation: 3,752 emitted-PPC checks across both editions and two relocation
placements, followed by actual Cemu controller samples, buffer checks and
native font queue coordinates. Closed-menu updates passed. This is not a
Wii U visual or full-game save-state acceptance claim.

## Speed grouping

`speed_grouping.s` wraps the existing `_gz_float_text` formatter. The private
integration uses it only for Debug speed and the MSS speed/speed-change
rows. It preserves signs, fractions and the existing unavailable/scientific
fallback. It does not change speed, facing, position or input.

## Trainer corrections (R555)

`mss_order.c` classifies pause type from raw controller event order. Slow is
pause, stick movement, pause; fast is pause, pause, stick movement. It reads
no player speed or speed gain. Repeated held/trigger samples are not extra
pause taps, new attempts clear the old label, and an order that cannot be
distinguished within one input sample remains explicitly unclear.
`mss_order_wrapper.s` preserves the legacy caller's state, snapshot and input
counter registers. The component passed 92 emitted-code checks. A real Cemu
swimming session also classified pause-stick-pause as Slow and
pause-pause-stick as Fast. Wii U acceptance remains separate.

`roll_clip_detector.c` is the exact earlier root-volume detector restored at
the user's request. The subsequently added animated-body fallback has been
removed. Native accepted-roll timing, latching and notification behavior stay
with that earlier implementation. This rollback does not claim every wall or
ledge has been newly validated.

## Particle report fast path (R559)

`particle_guard.c` and `particle_guard.h` retain the existing nine-method
coordination guard. Ordinary particle entry/return now refreshes the changed
diagnostic lane instead of copying every method's unchanged counters twice.
Active capture/restore ownership still publishes the complete report. Native
calls, atomic admission, identity checks, waits and GPU checks are retained.
No effect is disabled, and speed, physics and heap reservations are unchanged.

Validation: 224 emitted-PPC checks compare report bytes and unchanged authority
state. The isolated report routine executes 28 instructions instead of 175;
this is not a whole-game speedup percentage. The integrated Cemu candidate
passed three placed-bomb expiry/restores and both native pause-order cases.
Wii U superswim performance is not yet established.

`lite_hud.s` extends the existing timer text fast path to closed-menu Debug/MSS
text. Native font refusal falls back to the ordinary renderer. Menus, toasts,
the graphical Input Viewer and other graphical overlays keep the normal path.
It passed 236 emitted-code checks and actual Cemu native font queue checks.
It does not explain or claim to fix lag with every overlay off.

## Favorites (R570)

The Favorites source adds a main-menu entry and an X toggle on route memfiles.
The list loads through the existing checkpoint loader, shows the selected
route and area, and supports removing entries with X. X keeps its existing
meaning in the warp map and gameplay. An empty list cannot load a checkpoint.

`favorites_r562.c` uses two-word identities derived from route, area, name,
stage and entrance instead of saved numeric list indices. Unknown identities
are retained and shown as unavailable rather than redirected to another file.
Input queues preference changes; the ordinary game callback owns mutation and
file writes. There is no filesystem work in VPAD or the draw callback.

`favorites_storage_r562.c` keeps two checksummed 6,456-byte preference files
in the practice title's common save directory. It uses `SAVEInitSaveDir(255)`
before the first read/write, alternates banks, closes and flushes a complete
write before advancing its sequence. It never opens `cking.sav`. Storage
failure leaves session favorites usable and is shown in the menu.

Compile these components with the existing freestanding soft-float flags and
`-fno-merge-constants`: the current payload relocation exporter requires
ordinary section-relative string offsets, without linker string merging.
`favorites_input.s` preserves the existing input registers before dispatch.
The private integration also adds the root row and custom-page drawing, and
calls `_gz_fav_io_tick` from the ordinary game callback. No native hook or
extra heap reservation is introduced.

Validation includes 80 emitted-PPC checks across both editions and two
relocation placements. Native menu and filesystem acceptance is tracked
separately from these pure component tests. Actual Cemu checks passed X
add/remove/hold behavior, native file writes, a fresh-process reload of the
exact favorite identities, loading an original checkpoint and removing a
favorite afterward. Ordinary game progress files stayed unchanged. Wii U
acceptance remains pending.

## Source synchronization

`tools/sync_components.py --manifest <local-manifest> --publish` validates
explicit file hashes and destination paths, copies the selected authored
source, runs the repository audit/tests and pushes without force. The local
manifest contains machine paths and must stay outside the published tree.
Call this step after a completed, reviewed component build. It is not a
background watcher and does not create a scheduled job.

Do not put game assets, user capture arrays or raw state dumps in this folder.
The newer full multi-object runtime and installer need a separate reproducible
export; the legacy full-release exporter must not silently label them current.
