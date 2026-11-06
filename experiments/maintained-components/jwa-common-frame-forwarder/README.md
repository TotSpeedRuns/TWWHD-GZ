# Common after-frame forwarder

Unselected source-only entry for the real ordinary after-frame seam while the retained invocation is still in phase 2. It is placed before the original adapter, which clears that phase. This is not universal startup.

`_gz_jwa_common_after_native_frame(parent, thread, frame, optional_probe)` calls the existing `_gz_jwa_init_after_native_frame(parent, thread, frame)` from [jwa-after-frame-initialization](../jwa-after-frame-initialization), restores all four original scalars and the incoming SP/LR, then tail-forwards the original adapter once. Its return value is ignored. The optional-probe flag and existing request/probe gates are preserved.

The component contains 64 bytes of CODE, uses 64 bytes of temporary stack and adds zero static mutable storage, owners or SDK imports. The callee linkage/argument area at SP+0..39 remains available; scalar saves use SP+40/44/48/52 and saved LR uses the caller linkage slot at SP+68. The typed helper call is at CODE+28 and the original-adapter tail at CODE+60.

`_gz_gfo_native_after_return_adapter` is a new undefined Source interface, not a physical Native binding. A later integration must authenticate the real Native symbol plus signed addend, entry/ABI and live owner seam. No patch, physical alias, placement, installation or activation is included here.

Forwarding requires a normal ABI-conforming helper return. Prior paired-single/FP availability and canonical-FPSCR0 qualification limits remain; this integer entry supplies no arbitrary-FPSCR or whole-SDK preservation proof. The helper uses genuine direct coreinit imports and does not initialize the old Native SDK dispatch cache. Cache readiness, Gate/owner correspondence, cold-before-use, lifetime and consumer closure remain separate integration requirements. This is not a demonstrated fix for save errors, crashes or loading lag.

From this directory, assemble with devkitPPC r49:

```sh
powerpc-eabi-gcc -m32 -meabi -mcpu=750 -mhard-float -fno-pic -x assembler-with-cpp -c common_frame_forwarder.S -o common-frame-forwarder.o
```

Qualified compiler SHA256: `518fadfdd99f8882980a2564fa385597dffcfe33b589b2844c5ce3eeaab7dda5`.

Expected object: 896 bytes, SHA256 `b54dd3ea431cfb9f2cbd1c2543691460d20ad0dcdabd8815ad0366b4cb11179e`. This assembly build needs no SDK, game image, private profile or parent binary. Neither the object nor any game/SDK code belongs in this source export.
