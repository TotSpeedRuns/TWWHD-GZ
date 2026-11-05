# Setup178 USB and compatibility component

This is the authored source for the Setup178 USB/device and historical-role
component. It is a complete standalone component source build, not a complete
Setup or Game178 source build. No game, Setup executable, recipe, profile,
SDK, compiled object, captured memory or private-directory input is included.

Run from the repository root with Python3.10+ and devkitPPC:

```text
python -B tools/build_setup178_components.py --compiler /path/to/devkitPPC/bin/powerpc-eabi-gcc --output /fresh/path/setup178-objects
```

On Windows use powerpc-eabi-gcc.exe. The output parent must exist and its fresh
directory must be outside this source tree. The builder uses included C,
assembly, linker scripts and the20-record digest catalogue. The sole standard
header is stdint.h from the compiler's standard C headers; no WUT/SDK headers
are required. Compiler/linker reference hashes and all eight output hashes are
pinned. Other compiler/platform builds are accepted only if every output object
is identical; otherwise they are refused. The builder records tool identities,
cleans environment include paths, verifies the source manifest, refuses
symlinks/precompiled headers and extra component files, invokes no shell and
never modifies source or accesses a console/network service.

The eight objects reproduce merged ET_REL SHA256
1533769352d7d640d3c7687ce0a242d2d33f90f2e7e1e8f22fd23ec4aa8b8280:
5656 immutable CODE/RO bytes, zero mutable storage or SDK imports,91 typed
relocations,40 CODE-owned catalogue pointers and13 deliberately unresolved
Native borrowed interfaces. There is no per-frame work or new heap allocation.

The catalogue preserves all19 earlier artifact pairs and adds exact Game176.
Current Game178 and immediate prior177 remain separate Native profile roles.
Game174 is not invented. Recognition stays per RPX/title role, preserving the
existing mixed-known-version transaction policy. Strict current-only final
checks, current-output exclusion from prepare-source, stage/backup restrictions,
Unknown-only title extension and Cleanup165 behavior remain authoritative.

The USB selector scans the complete successful packed MCP title list and picks
the unique canonical practice registration for the requested NAND/USB choice,
independent of enumeration order. A registration only on the other device,
duplicates, malformed paths or invalid selectors refuse before mount/hash/file
changes. All five contexts, error/unwind destinations and13 Native bindings
retain their actual177 roles. bindings.json documents the exact parent and
interfaces; it supplies no image writer or physical lifetime guarantee.

Existing independent validation covers303 pure-matcher cases at three layouts
and bounded actual serialized178 storage/role execution (33 storage cases,
250 role/state/context/guard cases). Storage paths use explicit named synthetic
SDK endpoints; role paths use no IO/SDK result model. A declared NAND source
prefix with selected USB executes, but physical NAND app residency, physical
USB media, full asynchronous installation, firmware and exception unwinding
are not tested. Hardware Save/Restore and lag improvement are not established.
VALIDATION.json records exact scopes and evidence fingerprints.

A separate owned-input integrator must authenticate the exact Setup177 parent,
resolve genuine Native typed rows and recompute component placement, FileInfo,
CRC/compression, WUHB, current178/prior177 profile and recipes. Those private
materials and the full image materializer are outside this export and are not
runtime dependencies of the component builder. No console update or published
release follows from compiling these objects.
