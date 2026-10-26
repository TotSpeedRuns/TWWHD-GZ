# Source and build status

The full reproducible export remains v126. The maintained snapshot includes
UI components through R724 and save-state components through R703. The latest
packaged release is v140; its complete multi-object build has not been exported
here. The snapshot does not reproduce v140 or establish full-game coverage.
R724 UI testing does not promote the older save-state sources. A separate
post-v140 restore-performance folder now supplies the reviewed authored READ
scope/receipt includes and private collider metadata validator. It does not
export a complete newer restore build. Three expired-bomb and three idle Cemu
restores passed; native Wii U, post-void and sword-model gaps remain unresolved.
See the maintained restore-performance compatibility notes for exact limits.

The v140 source proposal additionally stages the reviewed bottle editor and
Frame Advance air-handler component. It does not replace the v126 full
builder or export the full newer runtime. One Cemu normal-swimming air test
passed pause stability, three one-point steps and ordinary Resume draining;
diving, drowning, every area and Wii U remain unqualified. The 88-byte
retail profile is absent from source and must be extracted locally from
the exact owned EUR v0 input. See the maintained-component compatibility
notes. This source snapshot does not reproduce the full release or claim native
Wii U acceptance.

The post-v140 UI update publishes the relocation-aware Forest Water guard,
Inventory stick cursor component and the menu ownership gate for Debug/Input
text. The Forest Water guard still checks all 22 native entry words exactly;
it recomputes only the seven documented DATA relocation halfwords from the
existing native heap-slot alias. The 88-byte retail signature remains a
generated local-only input. No raw game code or live diagnostics are exported.
Focused Cemu checks passed all four Forest Water slots, its shared countdown,
native Plus save/restoration, stick navigation and complete Tools toggle text.
Wii U acceptance of these post-v140 changes remains pending. These files do
not claim to fix the remaining MSS typing or save-state coverage failures.

The large legacy assembly template mixes authored patch code, compiler-generated
code and generated tables. It is not a good canonical review surface for the
whole project. The newer maintained C modules do not remove that limitation.

## Architecture cleanup requirements

- Keep authored C/C++ sources separate from generated assembly and data.
- Keep assembly bridges limited to documented patch/ABI boundaries. Record the
  original instruction, required registers, entry/return contract and test for
  each retained bridge. Existing hooks are not all justified merely by being
  present in the current build.
- Use readable modules, common integer/layout types, named fields and explicit
  size/offset assertions. Document alignment and section attributes individually;
  do not retain unexplained attributes as decoration.
- Compile ordinary logic through a dependency-aware build. Python remains useful
  for RPX transformation, relocation checks and asset extraction; a Python driver
  is not itself a defect. Version-specific source rewrites and machine-local
  build copies need replacement with explicit build inputs.
- Use verified typed interfaces for existing game objects. Choosing C++ does not
  by itself establish a native class layout, ABI or resource lifetime.
- Export enough source, toolchain information and lawful local-asset extraction
  instructions to reproduce the current release. Preserve native regression
  evidence when changing the build or representation.

These are open requirements, not claims that a language migration or a current
full-release export has already been completed.

## Dependency provenance

The build uses the public devkitPro toolchain and WUT interfaces. For example,
`GX2DrawDone` is declared in WUT's public `include/gx2/event.h`; knowledge of that
API alone does not establish use of a leaked SDK. WUT publishes a zlib license.
See its [source](https://github.com/devkitPro/wut) and
[license](https://github.com/devkitPro/wut/blob/master/LICENSE.txt).

This identifies a public dependency; it is not a blanket provenance certification
for every project file or for model training data. A complete dependency/header
provenance inventory remains to be finished. Existing third-party source notices
are listed in [NOTICE.md](../NOTICE.md). Proprietary SDK headers and game assets
must not be added to the published source tree.
