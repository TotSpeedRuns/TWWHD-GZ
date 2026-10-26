# Source and build status

The full reproducible export remains v126. The newer maintained-components
snapshot is R684. The latest packaged release is v139, but its complete
multi-object build has not yet been exported here. Do not treat the component
snapshot as a reproducible v139 release or use its checks as full-game evidence.

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
