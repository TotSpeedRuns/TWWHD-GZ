# Native job Gate scheduling probe

This authored experiment attempts an empty exclusive admission cut on the same
private Gate used by nine native callback wrappers. It checks scheduling and
token mechanics. It does not read, capture, restore or grant ownership of game,
actor, resource or device memory. WORLD_READY=0, READ_READY=0 and MISSING=15 stay
explicit in the report.

`exclusive_probe.inc` is the exact reviewed R3 source. `exclusive_probe.h`
describes its public function and 128-byte JXP1/v1 diagnostic report. The include
belongs inside the owning adapter's C translation unit; it is not a standalone
module or a replacement for the complete current runtime.

## Real admission and terminal behavior

The exact [common-admission](../common-admission/README.md) R2 implementation
provides the Gate and token API. Its exclusive TRY uses genuine OSTryLockMutex
and returns BUSY when metadata is contended or shared work is active. Ordinary
BUSY is a skipped attempt: there is no forced drain, cancellation or poisoning.
On actual OK, the next API call is genuine exclusive LEAVE with the same Gate
and token. LEAVE takes the private metadata OSMutex and may block. The successful
cut contains no game read, private report update or callback.

The producer checks real current-thread identity and interrupt state, as well as
the returned-frame serial and one-time initialization state. Initialization
OPEN means initialization succeeded; it is not the current admission door.
The existing writer report must use ABI3: word25 unwind restriction1, word26
OPEN-only0, word27 WORLD0, word28 MISSING15 and word63 logical-probe mode1.
Counter and immutable first-row offsets stay unchanged.

An unexpected TRY/LEAVE error or preexisting active token halts the probe and
preserves its first raw error and genuine token. Admission may remain closed.
Later calls do not change a halted report. A supervising disposable test must
stop on this unsafe terminal state; it must not clear the token or fake cleanup.
The sequence reserves FC -> oddFD -> terminal evenFE for exhaustion publication
and refuses odd, terminal-even and halted states before report writes. Raw
UINT32_MAX values mean no call occurred. They never mean successful API calls.

## Integration and compilation dependencies

The owning translation unit must already provide:

- `<stdint.h>`, `<stddef.h>`, official WUT interrupt/thread declarations, and
  the exact R2 `common_admission.h` types and constants.
- The single initialized `GzJcaGate jwa_gate` used by all nine wrappers, the
  actual initialization thread `jwa_init_thread`, and `jwa_initialization` with
  the adapter's successful `OPEN` value. Never initialize a second Gate here.
- The real ABI3 `_gz_jwa_report[64]` and the existing `jwa_barrier()` memory
  barrier. Fault word4 must describe the actual writer adapter.
- The genuine returned-frame caller. It must preserve the established
  thread/serial/context checks and skip the probe on initialization failure.

Compile the complete owner using the reviewed adapter flags: devkitPPC,
`-mcpu=750 -meabi -mhard-float -msdata=none -O2 -std=c11`, freestanding C,
`-Wall -Wextra -Werror`, no builtin, common symbols, stack protector, constant
merging, loop-pattern runtime substitution or unwind tables. Use official WUT
headers and `-fstack-usage`. The separate existing counter helper remains a
separate C object; do not enable LTO. See the
[counter workaround](../counter-atomic-workaround/README.md).

Link the exact common-admission implementation and genuine OS current-thread,
interrupt, mutex and condition bindings. The private integration resolves the
required exports; this export supplies no OS shim or positive return fixture.
The complete nine-wrapper adapter, native hook definitions and full integration
are separate dependencies and are not included in this narrow source export.

The new initialized report occupies `.data.gz_jxp` (128 bytes), and the token,
busy flag, last serial and halt flag occupy `.bss.gz_jxp` (28 bytes). Preserve
these allocated writable nonmergeable sections under the normal DATA/BSS policy.
The final image must retain the initialized report, include the whole new BSS
span in genuine cold startup zeroing, and resolve every reference correctly.
Never snapshot or replay initialized Gate, OS mutex/condition or token storage.
The executable/data boundary fence also remains an integration requirement.

Audit actual selected code and final fixups: all lwarx/stwcx effective addresses
must have RA=0. The reviewed R3 separation keeps the old wrapper/common/frame
functions' normalized instructions and semantic relocations unchanged, apart
from the new tick call. Local compiler stack usage was 24 bytes for the probe
and 16 for its tick caller; this does not prove transitive SDK/caller headroom.

## Observed scope

One genuine Cemu run on 2026-10-27 observed a window of 121 completed frames and
121 new same-Gate TRY/acquire/LEAVE/release cycles. Eight callback families were
observed; family 7 was unobserved. BUSY was also unobserved and was not forced.
The run used real native API results. It did not trigger HALTED/exhaustion or
failed-LEAVE paths natively; those retain separate source/emitted checks.

The source-visible private lock graph contains no GateMetadata-to-Directory or
GateMetadata-to-game-callback edge under the actual OS mutex/condition contract.
Unknown outer caller locks, OS internals, priority inversion, fairness and LEAVE
wall time remain unbounded by that audit. Returned-frame placement is not proof
that every game worker, allocator, constructor, disposer or resource has closed.

This is incomplete save-state groundwork. A logical cut and an even private
report do not establish a readable world snapshot, complete writer coverage,
physical/lifetime/resource/device closure, replay, all-game behavior or Wii U
acceptance. The inherited frame-report sequence's eventual wrap remains a
separate limitation. The reproducible integrated public export remains v126;
this small maintained component does not reproduce a complete v146 build.
