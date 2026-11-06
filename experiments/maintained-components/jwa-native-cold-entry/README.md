# JWA cold Native-entry guard — unselected Source component

This small package extends the already published 23-file
`../jwa-native-cache-initialization/provider-source` baseline. The builder
validates every baseline filename, size and SHA256 from `baseline-files.json`,
copies it into a new output directory, then overlays only the exact authored
`source/frame_init.h` and `source/frame_init.inc`. The baseline is the qualified
30,044-byte provider (`358863…`).

The new zero-argument `_gz_jwa_init_cold_for_native_entry` is 252 CODE bytes with
24 bytes of temporary stack and no new static storage or imports. It returns0
on an initial non-COLD observation before querying SDK roles. For COLD it
validates ordinary IRQ context and a nonnull current thread, requires the
existing Native-prepare role to return1, then repeats ordinary context, same
thread and COLD checks after a barrier before tail-calling unchanged fixed
constructor352. Cache busy/failed states are not reset or retried.

This is a COLD-only entry. It does not perform the JWT adapter's 344-byte checked
acceptance of an already-OPEN same-thread owner. Invalid context or failed cache
preparation returns0 before Source initialization writes; that differs from
Native260's existing post-CAS poison path. No full semantic equivalence with
Native260 is claimed. All 45 prior provider functions, 41 undefined interfaces
and 16,480 mutable bytes remain unchanged.

From the repository root, supply an explicit devkitPPC compiler and genuine WUT
headers, plus a new output directory:

```text
python tools/build_jwa_native_cold_entry.py --compiler /path/to/powerpc-eabi-gcc --wut-include /path/to/wut/include --output /path/to/new-output
```

The tool performs one static compile, without linking, target/SDK execution,
emulation or console access. The qualified Windows devkitPPC R49 compiler SHA256
`518fadfdd99f8882980a2564fa385597dffcfe33b589b2844c5ce3eeaab7dda5`
reproduces the 30,632-byte object SHA256
`56c1624e2a091a4b237f8708190ba4ffdbb590e63d0a1572e88f93d4734fdb59`.
A different object is reported as a mismatch, without compatibility claims.

The Source-only guard remains unselected. Future integration must authenticate
any Native260 entry retarget and its actual callers, bind the existing shared
state/Gate/directory/report owners and genuine cache-prepare role, and establish
cold-before-use, ordinary context, SDK lock/reentrancy and object/module/export
lifetime. No Native entry, Source activation hook, physical owner or game image
is patched here. A race after the final observation remains possible and the
fixed constructor still arbitrates with its CAS/context checks. No Read/world,
lease, readiness, B43 correction or lag-improvement claim follows.

Only authored text is included. The package exports no compiler, WUT/SDK files,
game code/image, private profile, binary object or runtime fixture.
