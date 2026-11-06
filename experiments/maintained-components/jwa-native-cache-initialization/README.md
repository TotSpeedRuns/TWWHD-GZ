# JWA Native-cache initialization — unselected Source component

This is the complete 23-file authored provider source for Source714. It adds
cache preparation before the cold after-frame initializer, then repeats the
existing GFO/thread/frame/COLD checks before the unchanged fixed constructor.
The new zero-argument JWT adapter accepts either successful cold construction or
a checked same-thread OPEN: state and report[3] must be2, while the stored private
initialization thread and report[59] agree with the nonnull current thread,
including final samples after a barrier. The adapter validates ordinary IRQ
context and current thread before and after the potentially yielding prepare
call. It returns accepted initialization1, rather than fresh construction or
Read/world readiness. Observed INITIALIZING and POISONED are not reset.

`_gz_jwa_native_sdk_prepare` remains a genuine undefined zero-argument FUNC role.
Its required retained-Native contract is return1 only for cache READY2 or a
successful all-five export commit; busy1, FAILED3 or failed lookup return0.
The source supplies no absolute address, Native alias, cache reset, retry or
direct cache write. Direct Source SDK imports do not prepare that Native cache.

The qualified static build preserves all 43 other provider functions, including
the fixed352 constructor, byte-for-byte with exact typed operands. The updated
cold helper is348 CODE/32 temporary stack; the JWT adapter is344 CODE/16 stack.
Total CODE growth is348 bytes, with zero static mutable or ownership growth.
This backup includes no compiler, WUT/SDK files, original game code, game image,
private profile, binary object or target/runtime fixture.

From the repository root, supply your own devkitPPC compiler and genuine WUT
headers, and choose a new output directory:

```text
python tools/build_jwa_native_cache_init.py --compiler /path/to/powerpc-eabi-gcc --wut-include /path/to/wut/include --output /path/to/new-output
```

The tool performs one static provider compile with explicit frozen flags and
does not link, emulate, contact a console or execute target/SDK code. Qualified
Windows devkitPPC R49 compiler SHA256
`518fadfdd99f8882980a2564fa385597dffcfe33b589b2844c5ce3eeaab7dda5`
reproduces the30044-byte object SHA256
`358863917014a4acc916f44f3afafb93a2fb03c3d76cce0f8b936a86f96b7d19`.
Different code generation is reported as a mismatch, without compatibility claims.

This component is unselected and cannot be installed by itself. Physical
integration must bind the same state/thread/Gate872, directory44 and report256,
prove cold-before-use and retained-consumer lifetimes, and authenticate the
prepare role. An unaliased private Source Gate does not initialize Native Gate.
SDK lock/reentrancy, module/export/cache lifetime and normal-return premises
remain open. The genuine Native JWT FDCC caller must eventually select this new
adapter. Remaining old Native260 constructor routes must select the fixed COLD
constructor or be proved unreachable, because Native260 retains plain terminal
OPEN stores. Do not globally change the COLD constructor's already-OPEN return0
semantics. READY2 remains a caller prerequisite for shared physical owners.

No retained Native caller, JWT/scene gate, optional probe, request gate, current
game or setup is changed here. The final samples are not a return-time atomic
guarantee: another writer can race the COLD CAS or poison after an OPEN sample.
No Gate-door state, Read/world authority, lease, SDK-object lifetime, runtime
readiness, B43 correction or lag improvement follows from this source backup.
