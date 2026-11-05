# Authored renderer normalization source for the v171 local test

This partial component preserves the authored v171 renderer adapter and its
diagnostics. It contains C, headers, one small assembler alias, a pure local
trace decoder and source build metadata. It contains no game instructions,
native executable, assets, captured memory, console profile or successful
provider stubs. Existing native owners and interfaces remain external.

The main component is 29,184 CODE bytes; the separate private predicate is108
bytes. Together they retain exactly91,200 private mutable bytes. Relative to
the authored v170 diagnostic component, only12 CODE bytes are added and no
mutable bytes are added. The original16 MiB startup reservation is external
and unchanged. Main79 unresolved interfaces plus the private extension leave
80 distinct genuine external interfaces after their shared provider is joined.

## Behavior and limits

The existing job gate closes before GPU completion and allocator hold. GPU
completion is never requested inside that hold. Authentic graphics ownership,
main-core and display-list checks remain required; an observed lock or numeric
pointer range is not a physical lease. Inside the real phase2/writer4 capture,
the bounded graph guard precedes genuine native queue reset and lifetime export.
Saved seals remain coupled to final slot identity/generation after compaction.

v171 changes only the two allocation-table lookup loops to scan at most262,144
entries, using RCG_ALLOC_PROBES. The independent metadata-read budget remains
131,072. The other64-probe/node/field/log limits, overflow checks, rechecks and
error behavior stay intact. No first-match acceptance, tombstone reclaim or
owner validation is removed. This is not an unlimited heap or memory scan.

The diagnostic helper reuses existing pending words12..15 (16 bytes), with
request/frame validation and a final barrier; it allocates no new owner and
does not write SlotSeal reserved words. diagnostics/decode_trace.py is a pure
local decoder of16 supplied unsigned words. Traces are sequential, non-atomic
and advisory; matching words do not prove freshness, admission or ownership.
An early failure before a matching pending record cannot necessarily be traced.

## Strict source-only rebuild

Use the exact reviewed devkitPPC r49/GCC15.2.0 toolchain and WUT1.3 headers:

    python -B tools/build_renderer_normalization171_source.py --compiler ../toolchain/devkitppc-r49/opt/devkitpro/devkitPPC/bin/powerpc-eabi-gcc.exe --sdk-include ../toolchain/wut-1.3.0/include --output ../renderer171-source-build

Output must be fresh, outside this repository, with an existing parent. The
builder validates the exact source manifest, unit/flag plan, gcc/as/ld/cc1,
public ELF inspector and all declared headers before creating output. It uses
both -MM and -M: eight compile units consume exactly44 authored C/header/asm
files,10 WUT headers and16 compiler/newlib headers. render_lifetime.c includes
render_graph.c, which is therefore a real dependency even though not a separate
compile unit. Duplicate spelling/Windows slash variants are normalized as sets.
The two historical unused include directories remain contained optional search
paths; no missing header is supplied by a private recovery directory.

Precompiled-header files/directories are refused in declared include trees,
because normal compilation may otherwise consume them while dependency scans
read text. Ambient compiler/include overrides are not inherited. All inputs
are checked again before output and after the build. Exact complete raw ELF
objects must match the reviewed identities, including symbols and typed rows;
a separate canonical allocated-byte/ABI/relocation gate also runs. All80 real
external native/SDK interfaces remain unresolved.

The boundary is the pinned authored files, standard Python library, reviewed
public ELF inspector and explicit compiler/WUT tool/header inputs. This does
not claim closure of the Windows compiler runtime or a full standalone game.
The builder never loads a game image, private object, recovery binary or live
profile, and does not execute SDK/game code or resolve native addresses.

## Evidence scope

provenance.json records the frozen source and independent review hashes.
The allocation-specific packed tests and normal paired fixtures retain their
explicit SDK/allocator/capture/whole-world models. This exact authored rebuild
adds no hardware boot, Save/Restore, complete world/device replay, elapsed-time
or causal live-fix claim. The v169 component/tool, current Source export,
native package, registry and unrelated work are not promoted or modified.
