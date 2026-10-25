# Maintained source components

These authored components follow current development independently of the
older integrated runtime export. `docs/revision.json` still identifies the
reproducible full v126 export. Updating this folder does not claim that v126
contains these changes or that the entire newer runtime has been exported.

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
It is not yet integrated into native foliage restoration. Separate plant
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
