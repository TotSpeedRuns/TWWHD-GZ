# v147 native heap groundwork and SOURCE validation

This folder publishes reviewed authored C, headers and includes from the v147
development branch. It is a partial component snapshot, not a standalone
library or a complete current runtime export. The full reproducible integrated
export remains **v126**. No ROM, copied native instruction blocks, binary asset,
runtime assembly aggregate, saved state, private profile or raw log is supplied.
Fixed scalar callsite addresses in the C code are integration contracts, not
native instruction data or evidence that a caller owns memory.

## Admission and the fixed metadata consumer

`worker_family.inc`, `worker_family.h` and the separately compiled
`worker_atomic.c/.h` track three whole native worker entries and the nine older
callback kinds during bootstrap. The new entries are archive read, archive
preload and layout execution. Native bodies remain concurrent. Counts begin
before the decisive initialization-state read. A cold-to-OPEN race proceeds
through genuine shared admission on the existing Gate. A cold record's immutable
stack identity conflicts with a later OPEN record before the latter rewrites LR.
Nonlocal unwind remains unsupported; stale or failed records are retained and
poison later checks, rather than being declared complete.

`directory_metadata.c/.h` provides the exact private identity helpers. The
worker include belongs in the existing nine-wrapper adapter translation unit,
with its one static `jwa_gate`, initialization state, report, directory mutex,
counter helper and barriers. It does not create another Gate. The exact
[common-admission R2](../common-admission/README.md) sources are already public;
pass that directory on the include path. The complete old adapter, native entry
and return wrappers, hook installer and relocated continuation bindings remain
external integration dependencies.

The omitted wrappers must preserve integer, floating/paired, FPSCR, GQR, CR,
XER, CTR and original LR state. Their 800-byte CPU-save frame is transient:
original SP is restored before the native body. That does not prove transitive
SDK/kernel stack headroom on every native thread. The 48 new GQR accesses already
use user SPR896..903; they need no supervisor-alias rewrite. The frozen 80 older
supervisor-to-user corrections are a separate unchanged integration operation.
Audit actual reservation instructions for RA=0 and keep atomic helpers in
separate translation units without LTO.

`lifetime_bridge.inc` accesses the same static Gate. `jol_context.c/.h` issues
only a dedicated current AFTER_RETURN allocator diagnostic context after a fresh
complete native completion cohort. It never changes the existing frame
observer's WORLD/READ functions. `coupled_hold.c/.h` takes genuine HG first,
then performs nonwaiting exclusive job TRY. BUSY releases HG without draining
writers. Actual LEAVE may contend on the existing private metadata mutex.
Unexpected errors retain genuine tokens and halt diagnostic activity; supervisors
must stop rather than clear tokens or manufacture cleanup.

JOL ABI4 calls the real family-zero check before its fixed GHD metadata consumer.
Normal family BUSY skips the consumer, cleans both holds and does not increment
the clean-cycle counter. JWF's 64 words are independently atomic diagnostics,
**not a coherent snapshot or admission token**. Never equate its sampled active
counts or last-zero result with a historical JOL/GHD packet. Before controller
input, the private tests refused sticky flags, retained-token publication or
unexpected actual shared/LEAVE statuses.

## Current private allocation metadata

`object-binding-try.inc` makes one real metadata-lock attempt with a shared
65,536-visit budget. `object-directory.inc` measures or copies a validated
private directory in two linear passes under one lock, reserving the complete
output before stores. `heap_domain_cut.inc/.h` performs a bounded three-root
selected-domain census with current generations, parent/backing extents and
allocation containment. It preserves global observer health/capacity checks,
charges traversal work, and refuses instead of guessing when proof is missing.
The reviewed resized-backing policy retains provenance requirements; it does
not make an observed parentless heap SDK-certified.

These includes require the exact existing lifetime translation unit's private
tables, indexes, metadata lock and helper definitions. `gate.h`, `lifetime.h`,
`lifetime_storage.h` and the object headers supply declarations and layouts;
their presence does not supply the complete native observer implementation or
all birth/free/destroy hooks. The caller must own its aligned high-DATA output
and exclude other writers. Metadata matches remain MISSING31/session0 and do
not grant native payload reads, no-free/reuse ownership or replay.

`actor_heap_cut.inc` is the fixed once-per-300-frame consumer. It reads three
resident root labels outside HG/job holds, calls the real metadata provider
under both genuine holds, then verifies labels only after successful cleanup.
The caller/source masks and both actual cleanup results govern usability.
No manager body, vector, actor payload, allocator callback or caller-selected
function pointer is added. Overlapping root capacity sums are not unique bytes.

## Genuine process-arena birth provenance

`process_arena.c/.h` follows the original root SDK allocation through its actual
factory/HL producer publication, with pre-destructor retirement and participating
SDK free/destroy entry/return intervals. The resolver runs at the fixed native
SDK entry before HG/SK/private metadata locks. It uses genuine existing loader
imports `OSDynLoad_Acquire` and `OSDynLoad_FindExport` for coreinit's
`MEMFindContainHeap`, caching only a successful real export. There is no guessed
address, direct invented import, API result fixture or fallback owner.

The root initializer can already hold its recursive native manager mutex; the
factory seam precedes the factory's own additional lock, not every caller-held
lock. The bounded private lock order is HL then GPA, never GPA then HL. The
provenance matcher requires a real retained HG token and fresh healthy current
HL root metadata. It is not an independent release lease or READ token. Its
native wrapper ticket provenance, all SDK release intervals, loader fixups and
HL birth/ending seams remain required external integration work. Never restore
the Gate, OS synchronization objects, tickets, private provenance state or SDK
resource metadata as opaque game bytes.

## Saved SOURCE work within a genuine pin

`source_active_write_guard.inc` excludes writes overlapping the complete active
SO allocation capacity. It preserves ordinary unleased serialization/decode.
The qualified OIS integration retains its original metadata-span authentication;
a wholly bounded low interval is disjoint from SO high-DATA spans, while all
other intervals require the full OSA-apart check. GCPU/image write admission
must call the same active-source guard. Those exact owner/serializer integration
seams remain external; adding this include alone does not close write aliases.

`source_pin_digest*.inc/.h` reuse only the actual sealed Row.hash under a fresh
genuine SO/OSA tuple and ordinary WCT invocation. Packed status plus digest keeps
hash zero distinct from refusal. Real acquire validation remains full, and both
healthy WCT precommit and installer publication force a fresh full SOURCE hash.
Every CURRENT equality, key, request, deadline, generation, registry revision,
owner and cleanup check remains fresh. Unsafe recovery retains its source-free
before-image path. No installed=0 bypass or cached current validation is added.

`source_pin_format.inc/.h` memoizes only four complete saved-format checks after
the fixed dispatcher itself executes the real helper and rechecks actual digest
and pin. The four `format_*.inc` files expose the exact extracted saved-only helpers
and CURRENT wrappers; `archive_layout.h` supplies their directory-shape check.
Their real deep validators, module types/constants, source/working buffers and
bank/birth/layers/typed-state integrations remain external dependencies. They
are not standalone complete validators and supply no successful API substitute.
BOM/BSR/capture remain FULL. A genuinely idle, healthy pre-issued decode takes
the old FULL path; an active lost/inconsistent/fatal pin refuses. The first memo
candidate's actual pre-issued-phase refusal remains part of development history,
not a passing native test. Historical memo pin/mask is not a hit counter or
capability. The eight-byte memo must be excluded from restored game payload.

`source_work.c/.h` and `source_work_atomic.c` provide a bounded private diagnostic
report, separate from admission. It observes eight declared SOURCE FNV
expressions in six categories plus five format-invocation categories. Lost or
overflow flags make counts incomplete without changing original results. Extents
are function arguments, not actual format bytes scanned; invocation counts are
not saved-scan counts, total process hashes or native CPU measurements.

## Observed scope and remaining gaps

One isolated Cemu session of the combined v147 private image completed two real
usable metadata censuses at least 300 frames apart. Both had source mask7 and
cleanup mask3, seven ancestor nodes, 118 selected heaps and parentless ancestry
flag2 still set. The stable historical GPA boot report recorded real resolver
state2/LIVE5 and joined the current parentless root metadata. The GPA matcher
was not called by that campaign. JWF sticky/retained-token diagnostics stayed
clear. Independent atomic JWF samples are not a globally coherent drain proof.

The same image then captured once and completed three idle same-area typed WCT
restores, with 14 participants, six groups, matching pose/cleanup and no scene
load. Host-observed times were about 1.012, 0.953 and 0.955 seconds, including
polling and added safety-report reads. Nine covered whole-SOURCE evaluations
were recorded per restore; four format categories still had 349 invocations each.
All three controller spans stayed within the compiled input interface and six
ordinary/source/disposable save files remained unchanged. The owned engine was
closed. These are bounded Cemu observations, not Wii U timing or reliability.

WORLD=0, READ_READY=0, logical MISSING15, physical MISSING31 and session0 remain.
This does not deliver an opaque heap payload snapshot, full writer/resource/
device/lifetime closure, boss/cutscene replay or universal save-state coverage.
The separate proposed raw-copy kernel remains default0 and is **not included**
here or adopted by this release. The supplied sources do not reproduce the
complete integrated v147 build; the reproducible integrated export stays v126.


## Generic object-binding fixed private-range checks

`object-binding-fixed-private.patch` is a later one-function development patch
for the ordinary selected `ob_private` helper. It evaluates the heap-independent
private-range predicates before and after the original heap loop. Every live,
mutating or ending heap still receives both original allocation-span and heap-
header disjointness checks under the same metadata lock. Source-age, ownership,
health, native access and cleanup checks remain required. This caches no lookup
or lifetime result across calls and adds no state, import or source format.

The patch was compiled over the exact selected `2a525ddf` native-lifetime parent
to `348e2fef`. It does not replace `object-binding-try.inc`, which is the separate
bounded, unissued try-only design described above. The complete observer and
native integration remain external; this partial patch is not a standalone or
complete current runtime export. The source fingerprints and bounded evidence
are recorded in `object-binding-fixed-private.json`.

One private Cemu test with the fixed-private patch completed a fresh capture and
three ordinary same-allocation bridge restores after a natural break, native
Draw, and near-ground traversal. Capture took 3.031 seconds; the restores took
12.906, 12.843 and 12.641 seconds including ordinary input and first observation.
Each reported native success, all 15 targets and a fresh commit, with the same
three bridge identities, rewound actions and exact Link state. All 120 checked
renderer buffers matched their expected centers within 0.00000763 units. Six
save files were unchanged, and the owned engine was closed.

The earlier separate baseline case took 21.390–21.625 seconds. The later SOURCE
was 12,800 bytes larger and the fresh cohort/context differed, so these are
individual observations, not an exact percentage or a general speed guarantee.
The observer still counted 118,587,315 node visits and 516,420 heap walks per
restore. The separate later owner-list single-traversal proposal is not included
in this patch or qualified by these timings. This does not qualify Wii U,
full-game or smoke restoration, retired-owner recreation, dirty recovery,
universal renderer/collision acceptance, or a release.
