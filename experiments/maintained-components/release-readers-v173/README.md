# Indexed release readers for the qualified v173 parent

This is authored C, PowerPC assembly and an ABI header under the included MIT
license. The standalone builder reproduces one PPC ELF32 big-endian relocatable
object: **684 bytes of CODE, zero mutable storage and zero new SDK imports**.
It contains no game/SDK bytes, compiled objects, game dump, memory capture or
binary recipe. The header describes existing module metadata; its functions
are dependencies already present in the qualified parent, not bundled here.

From the repository root, with Python 3.10 or newer:

```powershell
python -B tools/build_release_reader_component.py --compiler C:/path/to/devkitPPC/bin/powerpc-eabi-gcc.exe --output C:/path/to/new-empty-output
```

The output directory must not exist, its parent must exist, and it must be
outside this source tree. `build-spec.json` fixes compiler/linker SHA256, flags,
compile order and whole-object SHA256. The script accepts no flags override.
It checks all declared source files, rejects unexpected component files and
precompiled headers, strips compiler environment overrides, then verifies the
entire object and unresolved interface list. No SDK, private helper, game input
or network is used. Compiler binaries are not distributed. Their reference
hashes identify the standalone devkitPPC r49 toolchain used for qualification.

The exact object hash is
`fea3e775f9126876afb6687e5b4c01ec344b3c160091ad87dd97d5ec02e4e4b2`.
The local header include is the sole source spelling change from the frozen
qualified source; the exported source rebuild must match the whole object.

## Binding limit

`bindings.json` applies only to the **current v173 parent** with RPX SHA256
`6f6b32298de696e52ff7bed16c38d0f40463f0b675a0a15d8fc06beb11b52bfc`.
This source package is not an RPX builder and does not activate hooks. Its
preferred CODE and DATA addresses require genuine parent loader expressions,
provider-body authentication, branch-range checks and original caller proof.
The existing static `ready()` entry is not an exported SDK ABI.

**The newer full-kit/AD layout shifts provider CODE by +4. These bindings are
not portable to that layout. Separate proof against the actual parent and its
typed rows is required; adding4 to remembered addresses is insufficient.**

Two original compiler-inlined release-observer readers are replaced. Begin
uses the existing selector and preserves first pointer+heap match in circular
probe order, even if its state is rejected afterward. Return filters pointer,
heap, ENDING state and token before the first EMPTY, choosing the smallest
original circular distance rather than bucket insertion order. Begin retains
match-before-EMPTY behavior for pointer0; Return retains EMPTY-first and the
unchanged outer token0 early return. The original lock, admission, counters,
token generation, field rechecks, metadata writes and unlock remain in place.

A constant-space Brent preflight checks every existing next-link before the
real yielding cursor, including chains with no matching pointer. Bounds and
the262144-row cap precede every link read. Failure invalidates cache readiness
and atomically ORs the existing bad/exhausted flag; it never clears flags or
publishes a partial winner. No pointed-to heap or allocation payload is read.

Each context owns a48B EABI frame, retaining only proven continuation live-ins
and LR. C ABI retains nonvolatile GPR/CR/FP state. Actual integer providers use
no FP instructions and seeded full-FP preservation was separately tested; the
generic ABI alone does not promise preservation of volatile FP registers.
The helper's qualified compiler frame is80B. Original caller frames remain.
Metadata access relies on the existing common-lock/caller contract; this code
does not establish a new physical lifetime lease or concurrent-writer proof.

## Validation and cost

`VALIDATION.json` records the qualified source and independent review hashes
without redistributing private fixtures or copyrighted bytes. The source build
checks reproduction, not whole-game behavior. The underlying tests executed
real serialized Native paths/typed rows with explicit original interrupt SDK
endpoint models; they are not a physical-console latency measurement.

| Fixture | Original instructions | Indexed instructions |
|---|---:|---:|
|262144 historical DEAD keys, Begin miss|2621556|352|
|Same, Return miss|2621538|438|
|Sparse complete Begin|485|750|
|Sparse complete Return|644|1118|

The dense cases reduce primary metadata reads from262144 to1 and leave final
metadata identical. Sparse cases have extra validation overhead. Cost still
depends on bucket length. Existing one-time cold rebuild remains under the
original lock: the cold Return fixture measured6719838 instructions and524288
primary reads. These two readers do not repair every historical reader,
capacity/overflow issue, Save/Restore authority or physical streaming stall.
No hardware latency improvement or numbered release is claimed here.
