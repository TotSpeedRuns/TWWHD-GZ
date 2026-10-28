# Maintained source components

These authored components follow current development independently of the
older integrated runtime export. `docs/revision.json` still identifies the
reproducible full v126 export. Updating this folder does not claim that v126
contains these changes or that the entire newer runtime has been exported.

## Common C callback-admission primitive

The [common-admission component](common-admission/README.md) publishes the exact
reviewed C Gate and token implementation tested with four real OS threads on
three guest CPU cores. It handles nested/shared lifetimes, nonwaiting exclusive
attempts and cross-Gate token guards. This is a scoped private Cemu result;
game-worker/lifetime closure, arena rewind and Wii U behavior remain unproved.
It does not change the reproducible integrated v126 export.

## Generic CPU observation and fresh MSS visit joins

The [generic CPU observation experiment](generic-cpu-observation/README.md)
adds reviewed authored collector/header, root and private-owner includes,
small ABI bridges and interface headers. It captures complete owned CPU
allocations for two selected classes as diagnostic evidence. All63 unresolved
closure bits remain; no replay or native capture is claimed by this export.
The full current integration and reproducible builder remain separate.

The maintained `mss_order.c`, `mss_pause_frames.c`, `mss_visit_join.c` and
`mss_visit_join.h` now use the reviewed explicit-order classifier and completed
label latch. Slow is Plus-Stick-Plus, Fast is Plus-Plus-Stick, and Insta is
Plus-B-Stick or B+Stick on the closing sample. Duration is a separate metric.
Opening a new pause preserves the last completed label; ambiguous order cannot
replace it. One private Cemu run passed twelve sequences across three dwell
values, with native pause metrics of 4 to 27 frames and unchanged save hashes.
See [MSS rules, dependencies and portable tests](MSS_INPUT_ORDER.md).
This is a partial authored component update; delivered revision remains v147,
the complete reproducible integrated export remains v126, and Wii U acceptance
is not claimed. Historical MSS notes below describe older source snapshots.

## Static Triforce graphic, full-mask image and source hints

The [graphic component](triforce-graphic/README.md) adds an authored clipped
RGBA compositor, static irregular shard page, selection outline and canonical
source hints. When the low eight ownership bits are all set, the draw policy
uses a separate static full-image descriptor; the large page draws its selected
shard outline afterward. Other ownership masks retain the eight-shard policy.
The R769 baseline passed all 16 real Cemu give/remove operations and navigation.
The additional full-mask branch has emitted-code and positive-pixel checks.
The integrated R771 image then passed 16 give/remove operations and CPU-canvas
comparisons of 921,599 large-page pixels and 4,500 Inventory-preview pixels in
one isolated Cemu session; the large comparison excludes the sole authenticated
transparent refresh pixel at (0,0). Final TV/GamePad and native-font composition,
Wii U acceptance and native shine remain unverified. Owned artwork and complete
integration dependencies are excluded. The full source export remains v126.

## Ganondorf entrance and practice-travel gear cleanup (v142)

The [gear-travel component](gear-travel/README.md) provides the authored
112-byte leaf and exact source applier for two admitted practice-travel
calls plus all three Ganondorf raw-entry aliases. It retains native Zelda
fight dialogue and clears only the proven equipment-hide event bit on
explicit practice travel. Genuine Cemu entry, dialogue, controls and
naturally raised hide-bit recovery were reviewed. Pixel visibility and
Wii U acceptance remain unobserved. The exact private R757 baseline and
its complete link inputs are required; this does not export v142 in full.

## Location warp tables and Hyrule browser

The [location-warp component](location-warps/README.md) contains the exact
authored table generator/compiler driver and clean functional destination
facts, with attribution to Teotia's reference list. It accounts for all174
source rows and preserves old warp IDs with three explicit entrance repairs.
The result includes a Hyrule text root and location-grouped chart entries.
Private entrance bytes, full generated assembly and compiled objects are
excluded. The generator still requires its exact private qualified R755
base; this is not a complete current-release or replacement v126 build.

## Quarter-heart-piece editor (private R755)

The [heart-piece component](heart_pieces/README.md) adds separate 0/4 through
3/4 progress on Inventory's Heart Containers row and corrects Refills to use
the full-heart health ceiling. The native meter consumes its queued change
while GZ and the native Plus screen remain open. One isolated Cemu session
passed 22 menu cases and two ordinary expired-bomb restore regressions, with
unchanged original ROM and native save files. This is maintained component
source, not a full newer build or Wii U acceptance.

## Bottle editor and Frame Advance air handler (v140 source proposal)

The authored bottle editor and its small assembly bridges are included with
the R3 current-Play checks. Retail profile bytes are extracted only into
ignored local assets. The separate R2 air countdown-handler wrapper is
included with one Cemu normal-swimming pause/three-step/resume pass. Diving,
drowning, every area and Wii U remain unqualified.
See [compatibility and build notes](BOTTLES_AND_AIR.md). Existing grouped
menus, Movement and Favorites remain the R724 source snapshot below.

## Grouped menu, turn directions and Favorites persistence (R724)

`menu.s` provides eight main rows: Warps, Routes, Tools, Inventory, Tricks,
Favorites, Testing Tools and Misc. Testing Tools contains Frame
Advance and Movement; Misc contains the Creator Code display toggle and
Control Binds. Matching Back targets in `frame_advance.c`, `movement_turn.c`
and Favorites are required. The legacy input/draw/navigation seams are not
included by this small module alone; the integrated v126 runtime is unchanged.

Movement Right/Left use the corrected quarter-turn directions for all five
current yaw fields while preserving position. Favorites derives nonempty
folders from actual route-area metadata and retains the existing stable IDs
and FAV1 format, including unavailable saved keys. The storage worker uses
the active occupied account, reads old common banks only for migration, and
commits only after close, quota flush and full readback. An unreadable bank
refuses adoption; an empty list requires all four files to be absent. Busy
jobs retain ownership through late completion and newer edits remain dirty.

One controlled Cemu create/relaunch test passed with the same build in two
fresh processes: genuine legacy migration, five Earth Temple entries plus
Outset Island, six exact reopened keys, a native checkpoint load, and empty
folder removal. Both account preference banks were 6,456 bytes; legacy common
banks and normal-save hashes were unchanged. Root/group/Back navigation,
Creator toggle and inverse turns also passed. This is not Wii U acceptance,
error/account-switch coverage, pixel validation or a new save-state test.

The UI snapshot is R724; the save-state sources below remain R703. The
separate [post-v140 restore-performance snapshot](restore-performance/README.md)
contains the reviewed authored five-view READ scope/receipt includes, integration
seams and private collider observation validator. Three expired-bomb and three
idle Cemu restores passed; it is not a complete newer build or Wii U acceptance.
The older source snapshots and reproducible v126 export remain intact.

## Pot and contact rebinding after native actor recreation (R703)

`save_state/contact_owner_mapping.c`, `contact_replay.c`,
`contact_replay_api.h` and `pot_replay.c` retain the original archived pot
packets and join their placement/constructor identity to freshly qualified
current actors. Historical actor addresses do not grant access to a new
allocation. The current process generation, queue membership, heap, model,
resource and embedded collision/status bindings still need to pass. The
current before-image remains independent of the archived target, so recovery
does not need to follow the source packet. The narrow new correspondence is
limited to the supported PF2 flags-zero packets. No snapshot or reserved heap
size was increased.

One controlled Cemu test saved Outset, drowned naturally, and restored seven
recreated pots and 266 contact rows with the exact requested player pose and
no room load. Source and native save-file hashes stayed unchanged. That
post-void restore took 21.562 seconds; it does not meet the user's under-five-
second requirement. Optimization is in progress. Partial rollback, native
Wii U acceptance, bomb failures and full-game coverage remain open. These
files are reviewed component source, not a standalone integrated build;
their complete private integration dependencies have not all been exported.

## Triforce shard inventory page (R701)

`triforce_shards.s` adds an eight-row text page to the existing inventory
controller. The entry is selection 27 on Inventory; A opens page 57. A fresh A
press toggles the selected ownership bit, Up/Down wrap through the eight rows,
and B returns to the Inventory entry. The page shows each shard's ownership
and the total. It uses the existing current-save guard and request dispatcher;
the sole new save store changes one bit in player-save byte `0xBE`. It does
not synthesize chart, chest or quest history. Native artwork is not included.

The legacy runtime seams still required are the page-57 input/draw dispatch,
open-page hotkey ownership, D-pad repeat ownership, Inventory upper bound 27,
the row-27 handler before the health clamp, request IDs 940 through 947 in the
guarded inventory dispatcher, and the added text/cursor draw calls. The older
integrated v126 export has not been regenerated to include these seams.

Validation: 16 real Cemu give/remove toggles changed exactly the expected bit,
kept the other checked collect bytes unchanged, and returned the mask to its
original value. Held A did not repeat the toggle. Navigation, Back, and the
native text queue's 0/8 and 8/8 counts passed. Normal save-file hashes stayed
unchanged. This is a private candidate; Wii U verification is pending.

## Pause-order and D-pad ownership corrections (R692)

`mss_order.c` classifies Slow as Pause, Stick, Resume and Fast as Pause,
Resume, Stick. Duration and swimming speed do not select the type. A delayed
trigger for a continuously held button is not a second press; a coalesced
release/repress is handled separately. The fourth wrapper argument supplies
the existing raw release bits. Plus and B can both resume. Pause residency
still reports its own frame count; B alone does not imply an Insta pause.
The runtime displays the completed classification rather than a provisional
Fast result while the sequence is incomplete. A same-sample ordering ambiguity
is reported as unclear. The caller does not yet provide every historical native
input sample, so omitted sub-sample stick motion cannot be reconstructed.

`frame_advance.c` retains D-pad ownership only after accepting a pause, step
or resume command. Merely enabling Frame Advance leaves an ordinary Up press
and release available to the Wind Waker. The owned resume press stays consumed
through its release; a subsequent fresh Up returns to the native item.

Validation: 143 emitted PPC metadata cases and eight real Cemu pause-order
cases passed, including Plus/B resumes and short/long delays. Four real Cemu
D-pad cases passed: native Wind Waker use, Down stepping, owned resume/release,
and a subsequent native Wind Waker press. The separate native pause-menu
update gate is still a private candidate and is not included here. These
results do not establish Wii U acceptance or full save-state coverage.

## Earlier menu and capture baseline (R677)

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

The earlier Favorites worker moved preference I/O off the game callback.
R724 supersedes its common-bank writes and permanent stalled-request fallback
with the account, preservation and late-completion behavior described above.
The callback remains the sole live-list writer and never performs filesystem
operations. The normal game save is never opened by Favorites.

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
Input queues preference changes; the ordinary game callback owns live-list
mutation. One private worker owns all SAVE/FS calls and file writes. There is
no filesystem work in VPAD, drawing, or the ordinary game callback.

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


## Collider invalidation scan (R684 private candidate)

`collider_scan/members.c` preserves full validation of the 8,192-row private
collider table before changing any row. During validation it records the first
and last overlapping active row; the retirement pass then visits only those
bounds, or no rows when no collider overlaps. Native addresses remain scalar
keys in this routine. Tracking, locks, flags and counter behavior stay intact.
There are no new hooks or permanent data buffers. Invalidation stack use rises
from 40 to 56 bytes, and its machine code grows by 128 bytes.

Validation compares original and candidate emitted PPC across 73 private
metadata cases, plus seven converted-fragment cases, with every store restricted
to mapped private metadata or the fixture stack. Static instruction, relocation
and branch-target comparison also covers the unchanged helper/query functions.
These checks establish invalidation behavior, not a model of native ownership.

The integrated private Cemu image passed two placed-bomb expiry/restores, with
exact saved fuse and position. A three-second no-overlay swimming sample crossed
four sea sectors at about 30 FPS; observer flags stayed clear through 394
invalidations with zero collider overlaps. Wii U performance remains untested.
The modeled empty/singleton path uses about 25% fewer instructions; dense or
wide-bound tables can execute more instructions. This is not a universal game
speedup claim or a claim that remaining superswim lag is resolved.


## Post-v140 Forest Water and Inventory UI corrections

`bottle_play.h` retains the whole-entry signature guard and warm/current Play
checks. It reconstructs the exact expected HA/LO DATA relocations using the
existing heap-slot alias. The generated `bottle_play_profile.inc` stays local;
use the existing extraction command with an owned original EUR v0 RPX.

`inventory_stick.c`, `.h` and `inventory_stick_bridge.s` add cursor movement on Inventory page 8.
The component reads canonical IEEE-754 stick bits without floating-point ABI
changes, uses menu-local 0.5 press / 0.35 release thresholds and a 12/4-frame
repeat, and gives digital navigation priority. It does not synthesize actions,
change gameplay deadzones or edit resource values. Its ABI bridge replaces
one existing input load, preserves the documented registers and replays that
load before returning. Bottle and shard submenus keep their existing input.

`menu_stats_gate.s` defers Debug/Input text while the practice menu owns the
existing text queue, preserving their enabled flags. The existing draw call
targets this small gate instead; both overlays return when the menu closes.
Other tool overlays and native hook sites are unchanged.

Focused Cemu checks passed four-direction cursor navigation, repeat/release,
resource-row behavior, all 12 Tools labels and values with both overlays on,
and repeated close/reopen cycles. Forest Water passed all four bottle slots,
countdown, editing/deletion of other contents, native Plus capture/restoration
and refill. These are bounded UI checks, not full-game or Wii U acceptance.


## Common menu navigation and small clip adjustment

`menu_navigation.c`, `.h`, and `navigation_bridges.s` supersede the older
Inventory-only analog caller. They move cursor metadata, never synthesize
action buttons, use menu-local 0.50/0.35 thresholds and a 12/4-frame repeat,
and give real D-pad/action edges priority. Gameplay deadzones stay separate.
Vertical lists use Up/Down; horizontal stick navigation belongs to the item
grid, map and shards. The existing map D-pad/configured directions retain
their original repeat path; only its former analog fallback is retired.
`menu_navigation_seams.json` lists the required authored UI edits, including
retiring the old map analog fallback. Keeping both map paths causes duplicate
movement. The matching Favorites UI adds a read-only current-row-count getter;
its storage format and worker are unchanged.

Plain Left from Inventory's right panel returns to the nearest item row.
L+Left retains decrement, with existing Right/A/X/Y edits unchanged. Shard
selection accepts Left/Right as well as Up/Down; A remains the only toggle.
The clip detector's sole numeric adjustment is radius*.975 to radius*.98:
latest private tuning changes the radius multiplier from .9800 to .9850,
requiring more than 1.5% overlap, with its ledge/climb latch unchanged.
The delivered v146 uses .9800; this newer private candidate has 140 emitted
geometry/latch checks, without a new live wall-clip or hardware claim.

These are independently reviewed partial source components for a private
candidate. Native controller acceptance and Wii U delivery are separate;
the reproducible integrated export remains v126. The earlier Inventory-only
files remain as historical source, not an additional caller to install.


The R777 private candidate completed 64 controller-only menu cases in Cemu:
cursor navigation, action priority, right-panel return, shard selection and
Favorites navigation/temporary-entry cleanup. The resource state was restored
and ordinary/source/disposable saves were unchanged. Initial harness failures
were retained and corrected before this completed run. This supports those UI
behaviors; it does not qualify Wii U behavior, game-font pixels, native shine,
music, bridge restoration or hardware frame rate. The .975-to-.98 clip change
has 88 pure geometry/latch cases; no live shallow-wall result is claimed.


## Map area folders and the counter helper

[Map area hierarchy](map-area-hierarchy/README.md) adds related-area folders
while preserving all 218 existing destinations. Its private Cemu navigation
test covers all 49 sectors, 14 folders and parent-cursor restoration.
Forsaken Fortress separates FF1, FF2 and FF3, with the shared later exterior
labeled explicitly. The
module, eight exact source seams and integration contract are provided;
Wii U presentation and leaf-warp execution remain separate validations.

[The single-address counter helper](counter-atomic-workaround/README.md) is
a small separately compiled C function with a C/C++ header. It avoids the
diagnosed Cemu 2.6 indexed atomic-store register clobber. It supplies no game
snapshot or actor-lifetime authority.

These are reviewed authored partial components. The complete reproducible
integrated export remains v126; the newer modules do not constitute a full
v146 integrated source export.

## Native job Gate scheduling probe

The [native-job Gate experiment](native-job-gate/README.md) adds the reviewed
authored same-Gate TRY/immediate metadata-LEAVE producer and its small report
interface. One genuine Cemu window observed 121 completed frames and 121 new
acquire/release cycles; callback family 7 and BUSY remained unobserved. The real
LEAVE may block, and WORLD/READ readiness remains 0 with MISSING=15. Game-worker,
physical, resource and lifetime closure are unfinished. This narrow include
requires the existing common-admission primitive and full adapter integration;
it does not update the reproducible integrated v126 export to v146.


## Current owner allocation metadata

The [job-owner label observer](job-owner-labels/README.md) has two genuine Cemu
observations of both manager allocation identities. It reads fixed resident
labels and private allocation metadata; it does not read manager payloads or
grant lifetime, capture or replay permission. The later coupled exclusion
mechanism and complete integrated source export remain separate.

## v147 native heap groundwork and SOURCE validation

[The v147 authored-source snapshot](native-heap-v147/README.md) adds same-Gate
worker admission, HG-first metadata cuts, process-arena birth provenance and
SOURCE digest/format reuse under a genuine pin. One combined Cemu session
completed two usable metadata censuses and three idle typed WCT restores.
Parentless ancestry flag2 and WORLD/READ=false remain explicit; the raw-copy
proposal is not included or adopted. These are partial modules with external
native integration dependencies, not a complete v147 build export. The full
reproducible integrated export remains v126.
