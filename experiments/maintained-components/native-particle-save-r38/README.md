# R38 typed particle Save and CM validation overlay

This partial authored-source overlay follows [R35](../native-particle-save-r35/README.md).
It adds nine replacement and five new files over the combined R35 source tree
and its existing JEL/hooks producer, reconstructing the 202 maintained R38/R36
source files exactly. The unchanged files remain in the earlier components.
The complete reproducible integrated builder remains **v126**; this component
does not reproduce a complete current release or installable game.

## Contents and integration

Apply R35 over `native-save-v154/source`, add R35's separate `producer` files to
their matching `jel` and `hooks` directories, then apply this `overlay`.
`export-manifest.json` identifies every exact source and its upstream component.
Keep the earlier CODE bindings and add `native-data-aliases-extra.ld` to R35's
DATA bindings, yielding 18 CODE and 34 DATA identities. These are original static
symbol addresses; no game executable or captured pointer values are included.

R36 gives the real GHS Save transaction a distinct JPG/GPU fence participant
before heap/job holds. It retains exact-owner cleanup and does not invent a
successful hold. R38 exports/rechecks the typed JEL/JEX metadata under the actual
producer lock into one fixed receiver, preserves attributed owner/history bytes
as data, and encodes strict JPC version 3. An authenticated scope and current
allocation checks remain mandatory. Historical actor keys are never followed.

The fixed work object is 80,724 bytes: a 21,992-byte typed snapshot starts at
offset 76, its JEX part is 10,600 bytes, and the three binding slots start at
offset 80,532. The ordinary metadata receiver remains a separate 64 bytes.
The work increase is 16,748 bytes. The packet bounds remain 4 MiB encoded and
64 MiB raw, with bounded page transport; no 64 MiB scratch allocation is added.
Version 1/2 particle packets are rejected by the new strict parser.

The separate `collider/cm_fast.c` has only two compiler loop-unroll directives
over the selected implementation. It retains all 8,192 row checks, both locks,
the active-count proof and full overflow fallback. Its three headers are reused
from [streaming bookkeeping](../streaming-bookkeeping-v148/README.md).
The actual PPC private-metadata cases used about 15% fewer instructions for
zero/one matches and 0.56% fewer for 8,192 matches; the CO-busy refusal adds two
instructions. Text grows 1,700 bytes, with no BSS or stack growth. These are
microcases, **not a measured game, swim or Wii U performance improvement**.

`integration-recipe.json` records the relative dependencies and compiler flags.
Matching native wrappers, real providers, imports, symbol transforms, final
linking/packing/startup rules and lawful local inputs are still required. Do not
replace a native provider with a successful-return stub to claim qualification.

## Observed Save and remaining gaps

One closed isolated Cemu session completed **one Save and zero Restores** with a
paused placed bomb at **Dragon Roost Cavern → Sickass Skip**, native `M_NewD2`,
room 2. Two full 14,718,340-byte archive readbacks agreed. Six regions included
the whole manager, all three pools, the main-heap-covered control and one actual
0x68-byte callback owner. The real GPU fence completed and released; the entire
80,724-byte work object and separate 64-byte receiver were cleared afterward.
The normal profile and six protected save files were restored after exit.

The Save took 3.422 seconds in that Cemu observation. It is not Wii U timing or
a latency guarantee. Detached source-codec tests cover 147 host and 53 actual
PPC cases per format version, plus page/boundary cases. They preserve the R37
PO-frame mismatch acceptance and verify its R38 rejection. Nonzero attributed
owners and inert history are private format fixtures: this native case had
neither, so it does not prove their live acquisition.


Known R38 session limit: the retained JEX diagnostic log holds 512 events.
Once full, the next recorded event sets sticky `JEX_OVERFLOW`; typed export
then refuses further Saves for the session. The native case above had zero JEX
events and does not qualify long sessions. This is a known unresolved source
limitation, not a diagnosis of a user console or an error guard to suppress.

Particle Restore and lifetime adoption are **not implemented**. Resource/device
and indirect-writer closure, music, text/cutscene recovery, cross-area recovery,
durable slots, Wii U operation, actual-console memory fit and full-game Save or
Restore remain unqualified. An authentic Save is not proof of replay. Missing
coverage remains explicit; unsupported guards have not simply been removed.

Only authored text source, relative manifests/recipes and static symbol bindings
are included. Private diagnostics, captures, reports, host paths, profiles,
credentials, game content, SDK files and native binaries are excluded.
