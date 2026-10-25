# Maintained source components

These authored components follow current development independently of the
older integrated runtime export. `docs/revision.json` still identifies the
reproducible full v126 export. Updating this folder does not claim that v126
contains these changes or that the entire newer runtime has been exported.

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
