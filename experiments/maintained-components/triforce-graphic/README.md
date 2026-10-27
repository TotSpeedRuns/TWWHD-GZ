# Static Triforce graphic component (private R769)

This is authored C for a clipped RGBA8 compositor and the static shard page,
plus two small PPC bridges into the existing inventory draw caller. It is a
reviewed partial component, not a complete newer runtime or a release update.
The reproducible integrated source export remains v126.

The page uses eight irregular shard sprites, a selection outline, ownership,
and canonical source hints. Source hints describe the normal shard location;
they do not record where the current save acquired a shard. The existing
guarded inventory request dispatcher remains the only ownership writer.

## Inputs and integration boundaries

The compositor accepts caller-owned stable readable descriptors, readable
source pixels and writable destination pixels. It checks sizes, clipping,
transformed corner overflow and aliasing before writing. It does not verify
OS memory mappings or make an untrusted game pointer safe.

`menu.c` requires the `_gz_tfg_pieces` array: eight descriptors containing
source/outline pointers, width, height, and doubled native center coordinates.
All sprite pixels, extracted layout resources, compiled objects, original
game code and private diagnostics are deliberately absent. Those owned assets
and the reviewed integrated caller are needed to reproduce the private page.
The draw functions have no actor, quest, chart or save-writing interface.

The bridges require the existing qualified inventory register contract:
r29 is the practice drawing state; its +12 word is the owned RGBA canvas;
r28 is the current guarded save; r26 is the selected shard. They also use
the existing text functions and shard labels. These wrappers are not general
entry points and must not be attached to arbitrary native addresses.

Three private UI seams replace the old row drawing with the graphic/details,
add the Inventory preview call, and update page help. This folder does not
contain those complete integrated runtime dependencies or an RPX patcher.

## Build and checks

For a host compiler, the standalone compositor smoke test is:

```sh
cc -std=c99 -Wall -Wextra -Werror composite.c test_composite.c -o test_composite
./test_composite
```

For Cafe/PPC component compilation, use devkitPPC freestanding settings,
`-mcpu=750 -O2 -ffreestanding -fno-builtin -msdata=none -msoft-float`, and
`-fno-merge-constants` for the menu. The native linker must validate every
real relocation and allocated-section anchor. A single affine section anchor
does not describe pooled string offsets. Do not remove a failed fixup check.

The core passed 316 emitted-PPC cases and independent review of aliasing,
clipping and ABI behavior. The real private R769 RPX passed strict layout
and fixup validation. One isolated Cemu session passed all 16 shard give/remove
operations, held-A behavior, navigation and source captions. Other checked
collect bytes and original/native save files were unchanged. Root, Inventory
and shard menus advanced 60, 61 and 60 counter ticks in nominal two-second
windows. Exact elapsed-time provenance was not retained consistently enough
for a performance claim; this is not a timing benchmark or Wii U measurement.

This uses static native shard artwork when bound privately. Native shine,
material/noise animation, the complete-piece overlay and adjacent Pearl
rendering are not implemented by this snapshot. The authored source-over
blend is not a claim of native NW4F material equivalence. The private candidate
has not been promoted and has no Wii U visual acceptance.
