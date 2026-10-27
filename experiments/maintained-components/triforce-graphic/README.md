# Static Triforce graphic and full-mask image component

This is authored C for a clipped RGBA8 compositor and the static shard page,
plus two small PPC bridges into the existing inventory draw caller. It is a
reviewed partial component, not a complete newer runtime or a release update.
The reproducible integrated source export remains v126.

The page uses eight irregular shard sprites, a selection outline, ownership,
and canonical source hints. If `(ownership & 255u) == 255u`, it draws the separate
static full-image descriptor in both page and Inventory preview modes. A valid
large-page selection outline is drawn last. Other masks retain the eight-shard
draw policy, including dimmed unowned pieces. Source hints describe the normal
shard location; they do not record where the current save acquired a shard.
The existing guarded inventory request dispatcher remains the only ownership
writer.

## Inputs and integration boundaries

The compositor accepts caller-owned stable readable descriptors, readable
source pixels and writable destination pixels. It checks sizes, clipping,
transformed corner overflow and aliasing before writing. It does not verify
OS memory mappings or make an untrusted game pointer safe.

`menu.c` requires the `_gz_tfg_pieces` array and `_gz_tfg_full_piece`: the eight
shard descriptors contain source/outline pointers, width, height, and doubled
native center coordinates. The separate full-image descriptor has the same
layout; its outline is not used. Both descriptors and their pixels remain
private local integration inputs. All sprite pixels, extracted layout resources,
asset assembly, compiled objects, original game code and private diagnostics are
absent. Those owned assets and the reviewed integrated caller are needed to
reproduce the private page.
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

The full-mask derivative passed 32 emitted descriptor/control-flow cases,
40 source-caption cases and two positive canvas comparisons covering 166,500
pixels. Exported `menu.c` is checked against the qualified private module object.
These are pure component checks. The unchanged core passed 316 emitted-PPC
cases and independent review of aliasing, clipping and ABI behavior. The real private R769 RPX passed strict layout
and fixup validation. One isolated Cemu session passed all 16 shard give/remove
operations, held-A behavior, navigation and source captions. Other checked
collect bytes and original/native save files were unchanged. Root, Inventory
and shard menus advanced 60, 61 and 60 counter ticks in nominal two-second
windows. Exact elapsed-time provenance was not retained consistently enough
for a performance claim; this is not a timing benchmark or Wii U measurement.

The integrated R771 image then passed 16 give/remove operations, held-A
behavior and restoration of the original shard mask in one isolated Cemu
session. Its CPU canvas matched 921,599 large-page pixels and 4,500 pixels in
the Inventory-preview region. The large comparison excludes the sole
authenticated transparent refresh pixel at (0,0). Original ROM and native save
files remained unchanged. These canvas checks exclude final TV/GamePad and
native-font composition; they do not establish Wii U visual acceptance.

This uses static native shard/full-image artwork when bound privately. Native
shine, material/noise animation and adjacent Pearl rendering are not implemented
by this snapshot. The authored source-over blend is not a claim of native NW4F
material equivalence. Full current-release source reproduction, final display
composition and Wii U visual acceptance remain separate qualifications.
