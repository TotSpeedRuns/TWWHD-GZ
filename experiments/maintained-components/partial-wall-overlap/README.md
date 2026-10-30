# Partial wall overlap trainer

This is the authored trainer unit used in the next private candidate after v151.
The finite wall-circle query now uses `0.995 * live_radius`, previously `0.990`.
It therefore needs more than 0.5% radial overlap instead of more than 1%.
Exact tangency remains excluded. Wall-normal, native filter, finite triangle,
horizontal slice and animation guards are unchanged; roll timing is separate.
The detected partial remains latched through the same hang-to-climb attempt.

`trainer.c` is the complete current unit; `radius-change.patch` isolates the two
changed lines from its selected predecessor. The portable pins include the exact
source, original object and newly compiled object hashes and compiler flags.
The original object was reproduced before compiling the change. Raw object and
verbose-assembly reassembly matched, then the actual linked and packed bodies
were checked. The surrounding native build retains all 803 checkpoint payloads.

One controller-only Cemu observation loaded ET Roll Clip 2 through the real route
menu, enabled the trainer, grabbed a ledge, moved around its corner and climbed.
It reached native procedures 44 through 48 without reporting a partial clip.
That checks this ordinary ledge path. The runner's missed shallow clip was not
reproduced; positive sensitivity and Wii U behavior still need playtesting.
The observation used no host actor, pose, animation or loader writes.

This source depends on the trainer symbols and genuine native collision-query
adapters declared at its top. It is a partial maintained component, not a
standalone game patch, a full current builder or a new released binary. The
complete integrated source export remains v126; v151 ships the earlier threshold.
No game assets, memory captures, personal profiles or console codes are included.
