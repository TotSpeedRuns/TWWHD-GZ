# v148 streaming bookkeeping source

This is a narrow authored-source snapshot of the CM, SDK observation and HL
lookup changes shipped in the v148 test build. It does **not** reproduce the
complete v148 game image. The full reproducible integrated export remains
**v126**. There are no ROM instructions, game executables/assets, saved payloads,
raw diagnostics, private profiles or host memory readers here.

## Included implementation

`collider/cm_fast.c` retains the complete 8,192-row collider metadata preflight
and exact active-count check before retirement. It caches pointers to at most
32 matching private rows under the existing CM/CO busy protocol. Overflow
validates all remaining rows and uses a full retirement scan. It does not
truncate matches, skip lifecycle observations, dereference game objects or
change gameplay. The fixed scratch is 128 bytes on the target PPC32 ABI.

The selected old implementation already used first/last-match bounds for its
retirement work and a bitwise empty-row check. The new implementation is not
removal of an unconditional old zero-match scan. Detached actual selected-PPC
comparisons included small regressions: roughly +0.028% at zero matches,
+0.021% at one, and +0.473% at 33. The 32-match case used roughly 30% fewer
instructions. These are microcases, not measured Wii U or streaming speedups.
Public comments clarify this distinction; implementation tokens are unchanged
from the reviewed C source.

`sdk/sdk_drain.c` retains SDK observation admission, pending-operation and
conservative overlapping-birth refusal behavior. The first drain sweep always
runs. Another sweep is needed when retirement marks a heap at or before the
current scan position; later-index heaps are already visited in the same pass.
This removes empty terminal repeats while retaining the same metadata closure.
It does not remove actual free/destroy observations or supply SDK outcomes.

`hl_lookup.inc` contains the two exact changed functions, `allocation_find` and
`custom_find`. Existing-key mode does not maintain an unused first-DEAD choice.
Create mode keeps its original match-before-empty, first-DEAD selection and
full-capacity behavior. Identity remains `(heap,pointer)`, refined by descriptor
for Custom rows; identical pointers in nested heaps are not merged. The table
remains 262,144 linear-probed rows, private stride60/public projection80. The
separate unmerged triangular-probing experiment is **not included**.

## Real integration dependencies

CM requires the declared genuine CO/CM metadata arrays, control words and frame
producer. Its `members.h`, `observer.h` and `process.h` are authored interfaces,
not the complete observer/hook implementation. The existing definition is
renamed at its definition before joining the new module; the original body is
retained. Native lifetime callbacks and all admission checks remain mandatory.

SDK requires the exact old private entry and locate helpers through the two
explicit assembler-symbol bindings, the genuine 37-site table and existing
controls/rows/operations. The header does not supply those implementations.
The old entry body is retained under its legacy definition. Do not replace
these providers with successful-return stubs to claim native qualification.

HL belongs in the existing observer translation unit after its private types
and table declaration. The adjacent `native-heap-v147/lifetime.h` and
`lifetime_storage.h` supply the unchanged declarations. The caller's real lock,
header/health checks, lifetime rules, counters, generations, index links and
producer publication remain external. A found or DEAD row is not a lease or
payload-read permission. These selectors do not grant ownership by themselves.

The target is PPC32, with reviewed compiler/atomic code generation. Pointer
casts and bitfield storage are not a portable host wire ABI. CM/SDK atomics
must be audited for the target reservation-address rule; the private combined
build retained all 11 previous runtime modules and added 38 reservation
instructions with RA=0. No new hooks/imports or SDK reserve were added.

## Tests and observed limits

`lookup_cases.c` compiles the exact two HL functions and exercises 21 scalar
metadata cases, including full capacity, descriptor/heap identity and first-DEAD
reuse. Its host-named bitfields do not qualify PPC serialization. The Python
tests additionally compare CM sparse/overflow target sets and SDK ordering
closure. Those two Python models are reference arithmetic checks, **not**
execution of production CM/SDK, OS calls, synchronization, or successful API
fixtures. Separate private emitted-code tests covered those production modules.

The selected R877 Personal Cemu image completed three quiet placed-bomb natural
expiry/recreation restores with matching fuse/pose and no load. A separate
104.188-second controller-only ocean campaign reached Fire Mountain F3, Ice
Ring E6 and Outset B7 with ordinary water/waves/weather/enemies retained and no
overlay during measurement. Six ordinary/source/disposable save files stayed
unchanged. These are bounded compatibility observations; host polling is not
callback CPU time and no comparative ocean speedup was established.

The v148 Setup was delivered for a Wii U test. Wii U lag/crash improvement is
**unverified**. WORLD=0, READ_READY=0, logical MISSING15, physical MISSING31 and
session0 remain. No opaque heap capture, held-sword repair, full device/resource/
lifetime closure, unrestricted restoration or complete newer source export is
claimed. The raw-copy kernel and triangular candidate are not part of this
snapshot or delivery.
