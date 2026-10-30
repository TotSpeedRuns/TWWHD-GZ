# Partial wall overlap trainer

This authored trainer component is staged after v151. The finite wall-circle
query uses `0.9975 * live_radius`, previously `0.995`, requiring more than 0.25%
radial overlap instead of more than 0.5%. Exact tangency remains excluded.
Native filters, finite triangle and horizontal slice checks, animation guards,
positive latch, expiry and roll timing are unchanged.

`trainer.c` is the complete unit. `radius-change.patch` isolates the threshold
and comment change against its selected predecessor. Portable source/object
pins and compiler flags are included. The predecessor object was reproduced
byte for byte before compiling the change; raw and verbose-assembly reassembly
matched. The new threshold still needs full main integration and native tests.

The preceding 0.995 component passed a controller-only Cemu observation of an
ordinary ET Roll Clip 2 ledge, corner and climb without reporting a partial clip.
That observation does not validate this newer threshold. Repeat the ordinary
ledge test and the runner's missed shallow setup before claiming sensitivity or
false-positive results. Wii U behavior remains unverified.

This component depends on the declared trainer symbols and genuine native
collision-query adapters. It is not a standalone game patch or current full
builder. The complete integrated source export remains v126; v151 contains the
earlier 0.990 threshold. No game assets, captures or personal profiles are included.
