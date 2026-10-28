# Fixed clock and persistent-root borrowing

This partial authored snapshot repairs nested allocation-admission refusal in a
private post-v148 save-state branch. It is not a standalone library or a new
release export. The complete reproducible integrated export remains **v126**.
No original game method bodies, game instructions/assets, binary images,
captures, save files, user/session data or host-memory reader is included.

## Real ownership and the fixed borrower

The whole-transaction provider in `wct_heap_lease.inc` acquires the real HG
allocation exclusion first and immediately tries the same existing job Gate.
It retains both genuine tokens until checked final target/inverse/source
validation, then releases the job token before HG. The `wct_job_bridge.inc`
fragment is included in the same adapter translation unit as its actual static
`jwa_gate`; it does not initialize or copy a second Gate. Worker counters are
independent atomics, and their sticky/current checks remain required.

`wct_native_context.inc` authenticates the real NBD-to-HBP native callback,
current frame/thread and compiled WCT request. It cannot create GFO/JOL, WORLD
or READ authority. Leaving that callback expires the context; retained failure
state does not authorize recovery from a later frame.

The fixed `wct_root_borrow.inc` layer lets CLOCK and PERSIST reuse that actual
interval. Its sealed 32-byte private record binds kind, mode, entered-kernel
serial, frame, thread, monotonic acquisition identity and actual HG token.
Each operation freshly checks real WHL/job ownership and the actual CKR/PRK
kernel producer. WGP modes 1 through 4 remain separate from root tags 5 and 6.
There is no caller-supplied pointer, callback or successful result.

## Getter and cleanup rules

The exact authored `save_origin` fragments retain the existing SrhBinding
hash, both root/backing generations and current tuples, real HL try-lock,
fixed clock/persistent tables, expected/alternate values, and pre/post kernel
scope checks. All three old HG seals remain real. A borrowed seal first checks
the exact live root borrower; the ordinary seal calls its original API.

Outside a pending WHL transaction, the original independent HG begin/seal/end
path remains. A pending but refused borrow never falls back to that path.
Successful unborrow clears only private borrower metadata. A failed or orphaned
borrow remains pending and blocks actual release, rebegin, WGP leave/abort,
unborrow and private finish. Tokens are never manually cleared to manufacture
cleanup success. SSP's finite native stow/inverse protocol is unchanged.

The old report sizes remain unchanged. SCK word 15 and SPK word 16 are path
markers: 1 borrowed, 0 ordinary. Their word 5 is the actual private-unborrow
status on a borrowed path, or actual HG-end status on the ordinary path. WHL
words 13 and 24 remain the actual HG-end/job-LEAVE results. None is a lease or
a coherent cross-report cut.

## Integration dependencies

These includes require the matching external WCT/ROH/WSP/WGP journals and
checks, actual NBD/HBP producer seams, native HG enrollment, current job-worker
coverage, fixed CKR/PRK kernels, and original save-root lifetime/projection
providers. `integration-seams.json` records the exact seventeen authored
changes against hashed private parent sources; `integration-seams.txt` renders
them for review. The complete parent translation units and current builder are
not exported. The getter files are exact function fragments, not independent
complete root implementations.

The job bridge uses the public `common-admission/common_admission.h` types and
the existing `native-heap-v147/worker_atomic.h` interface. The actual adapter
still provides `Jwa` state and ordinary-context predicates. The HG provider
uses public WUT interrupt declarations. PPC32 pointer/layout and `sync`/atomic
semantics apply; a host compile alone cannot qualify this integration.

Named sections give the private state distinct layout/zeroing anchors. The
new root layer adds 32 NOBITS bytes and no DATA, native import name or hook.
The existing WHL/native context/job-token state is retained. Main startup must
clear every actual allocated BSS span and preserve the genuine same-Gate,
native DATA/TEXT relocation and enrolled-lifecycle contracts.

## Observed scope

On one private reviewed R894 Cemu image, three holstered-save/drawn-current
restores completed with native redraw and real cleanup: 0.917, 0.853 and
0.887 seconds. Three separate quiet placed-bomb natural-expiry restores also
completed through the unchanged standalone BOH path: 1.310, 1.213 and
1.198 seconds. Saved poses/fuse and the six original save-file hashes passed
their scoped checks, with no room load. Successful live SOURCE equality is
proved by the frozen driver's executed equality assertion; separate live
post-success source packets were not retained.

These end-to-end intervals include host polling and are not a controlled CPU
benchmark, Wii U measurement, universal held-item proof or release claim. The
clock/persistent repair does not solve current-only actor retirement, broad
heap replay, asynchronous resources, all native lifetimes or full-game save
states. The later SAND/contact memo and other unmerged proposals are excluded.
WORLD and READ readiness remain false.

The portable repository tests verify source pins, exact authored seams,
borrow/cleanup guard ordering and absence of fallback. They do not execute
native APIs or provide synthetic successful Gate/owner results. Actual PPC
component reproduction, semantic/relocation review and finite controller tests
were separate private checks.
