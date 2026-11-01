# R39 bounded causal-window Save overlay

This partial authored-source overlay follows [R38](../native-particle-save-r38/README.md).
Eight replacements and four additions reconstruct the 206 maintained source files
exactly; unchanged sources remain in the earlier components. The complete
reproducible integrated builder remains **v126**. This overlay does not build an
installable current release on its own.

## Event window and integration

R39 replaces R38's append-only 512-event limit with a bounded recyclable causal
window. Event serials remain monotonic. An outstanding native removal keeps its
exact entry pinned until that serial returns; ordinary completed entries may
recycle. Return completion is recorded before appending its completion event,
so even self-slot reuse clears the old returned bit correctly. Genuine capacity
exhaustion, serial/cookie wrap and corrupt receipts still refuse. Fault bits are
not cleared to manufacture success. This is bounded history, not a full timeline.

The new semantics are explicit: **JPC wire 4, JEX capture 2 and JEX report 2**.
The strict reader rejects older particle packet versions. Report counters and
returned bits describe physical window slots; omitted event payloads are not
reconstructed from archived values. Attributed owners and generation-zero inert
history remain data, and historical actor keys are never followed as pointers.

Apply this `overlay` over the R38 reconstructed source tree. The manifest records
every source's relative component/path and exact hash. CODE/DATA bindings and the
R38 compiler-only collider change are unchanged. The report remains 96 bytes,
the typed snapshot 21,992 bytes, the work object 80,724 bytes and its separate
receiver 64 bytes. Existing 512 event slots are reused; no permanent BSS or
snapshot growth is added. The full source-window validator uses a bounded
temporary uniqueness set; its pure PPC frame measured 4,144 bytes, which does
not establish hardware stack fit for the integrated call chain.

The `bomb` directory contains **two header overlays only**. The matching current
whole bomb-recreation translation unit, include tree and its two matching
auxiliary members are not supplied by this component. A rebuild must retain that
exact composition and every other current fix. Do not substitute an older
public monolith or invent a missing relative dependency. The recipe makes this
incomplete integration boundary explicit.

## What was checked

Detached tests exercised 56 host and 56 actual PPC window scenarios, plus 51
host and 27 PPC wire cases. They cover normal sequences beyond 512 events,
long-pinned/out-of-order receipts, all-slots-pending refusal, self-recycling,
bitmap reuse, duplicate/stale cookies, corruption and genuine 32-bit exhaustion.
These use private authored data; they are not native gameplay events. Actual PPC
store traces checked 3,107 commits for serial-before-slot ordering and observed
no writes clearing the fault word. Both real producer and quiet-consumer
components compiled; the selected quiet baseline reproduced byte-for-byte,
its original imports/ABI remained, and its writable storage did not grow.

One isolated Cemu session completed **one Save and zero Restores**, with a paused
placed bomb at **Dragon Roost Cavern → Sickass Skip**, native `M_NewD2`, room 2.
Two complete 14,721,444-byte readbacks agreed. The six captured regions included
one actual 0x68-byte callback owner. The genuine GPU fence completed and released,
the work and receiver cleared, and the normal profile and six protected saves
were restored after exit. The Save took 3.734 seconds in that Cemu observation;
it is not Wii U timing or a latency guarantee.

That native case had **zero JEX events and zero attributed owners**. It does
not verify native event churn beyond 512. In the selected integration, the
bomb-explosion observer (BXP) opens attribution only for an explicit active
diagnostic request with its original guards; request zero is inert. It does
not attribute every ordinary gameplay effect. The R38 512-event defect is a
source-derived limitation addressed by this algorithm, **not a diagnosis of a
player's unsupported-state error**. R38's frozen notes describe that older source.

Particle Restore and lifetime adoption remain **unimplemented**. Resource/device
and indirect-writer closure, music/text/cutscene and cross-area recovery,
durable slots, native Wii U operation, console memory/stack fit and full-game
Save/Restore remain unqualified. No new swim or lag improvement is claimed.
Only authored text and relative manifests/recipes are included: no game assets,
SDK files, binaries, memory captures, private diagnostics, host paths or codes.
