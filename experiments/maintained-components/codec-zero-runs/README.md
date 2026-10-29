# Lossless zero-run decoder optimization

This authored-source snapshot changes only the hash-only zero-token decoder
branch. FNV-1a over `n` zero bytes is exactly `h * prime^n` modulo 2^32;
exponentiation by squaring reduces that branch from O(n) to O(log n).
Literal and repeated-word handling, decoded output, bounds checks, checksums,
validity publication and the public API remain unchanged.

`codec-original.c` contains the prior codec with LF line endings, and `codec.patch` is the source
delta. `compat_harness.c` compares caller-owned buffers using original/candidate
functions renamed to `old_sc_*` and `new_sc_*` during compilation. It contains
no game code, memory access, native API outcomes or private captured states.

Qualification completed 4,610 portable old/new C checks, then independently
compared encoding, hashes and full decoded bytes for two newly captured private
bomb states. The emitted PowerPC object changes only decoder `walk` (+20 bytes),
with seven other functions preserved and no added imports or persistent state.
The private integrated build preserves all 803 checkpoint payloads. It passed
three ordinary unpaused Outset bomb-expiry/restoration cycles in Cemu, including
fresh completion, exact bomb position and frame-consistent fuse checks.

This does not implement missing smoke-particle reconstruction. A state captured
with placement smoke still has an explicitly unsupported restore path. Full-game
save-state support and Wii U performance are unproven. The integrated public
builder remains v126; these files are a partial component, not a complete current
game build export. No Wii U update accompanies this source snapshot.
