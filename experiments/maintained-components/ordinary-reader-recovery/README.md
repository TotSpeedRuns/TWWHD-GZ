# Ordinary request/frame reader source recovery

The two Source files here are verbatim copies of the independently reviewed
recovery. They preserve three contiguous functions as explicit PowerPC assembly:
`current(request, frame)` is 344 bytes, `release(request)` is 260 bytes, and
`acquire(request, frame)` is 512 bytes. The header documents the scalar ABI,
return0/B72/B73/5 distinctions and a logical 44-byte fixed holder. All 279
instructions and 47 raw local branches remain represented directly. This is
maintainable assembly recovery, not an invented C decompilation.

The standalone object has 1116 bytes of CODE, 44 bytes of NOBITS, 56 typed
relocations and eleven unresolved external roles. Those roles cover the genuine
SDK imports, view preflight, worker quiet classification, thread wrapper,
exclusive Gate TRY/LEAVE, sticky halt, diagnostics, NBD report and shared JWA
owner. The holder's frame at12 differs from its NBD dispatch serial at20.
Release can drain a halted owner; no reset or rearm policy was added.

Rebuild from the repository root using Python 3 and the pinned Windows
devkitPPC r49 compiler. The compiler distribution is supplied separately.
No game dump, SDK module, WUT headers, private bindings or Native artifact
is needed for this assembly-only build:

```powershell
python tools/build_ordinary_reader.py --compiler "C:/devkitPro/devkitPPC/bin/powerpc-eabi-gcc.exe" --output "work/ordinary-reader-recovery/ordinary-reader.o"
```

The builder checks both Source files and the compiler, refuses existing output,
invokes the assembler compiler once, and requires the exact object below. It
writes an adjacent JSON build receipt. It never links, loads or executes the
object. `BUILD.json` contains the complete compiler flags and verification
identities.

| File | Bytes | SHA-256 |
| --- | ---: | --- |
| `source/ordinary_reader.S` | 23197 | `a56048f7a9b901177bd562c80f29ce247a95e6ebe258aa7631b58e805bef8320` |
| `source/ordinary_reader.h` | 4651 | `ff03c1f62a04c96099b0bf762f38719523172e6644674bddcd80673ce0c766b5` |
| Windows devkitPPC r49 compiler | 3088896 | `518fadfdd99f8882980a2564fa385597dffcfe33b589b2844c5ce3eeaab7dda5` |
| Expected `ordinary-reader.o` | 3216 | `f31b2cce32243d76a187cad114e7afaad740369e7e33fb5ef1e008da5031b07b` |

Static provenance qualifications are `cf4c1a41b179f9867273416ec28c971713f505c0c3ce92b4c8501e7f66b2f5f6`
and independent peer `0a2c0f7f8663c85b205d8ba56ce8094e439076accf0297a652e0f6369180336b`.
Their private raw-byte and preferred-layout receipts are deliberately outside
this source backup. No game/SDK binaries, disassembly, replay inputs, physical
Native bindings or runtime-composition scripts are included.

The module remains an unselected Source component. Its new logical owner and
undefined roles require a coherent integration, lifetime, startup and SDK
context contract. This ordinary request/frame interface differs from saved-image
OIS leases, Restore HELD views and fixed-JOL-token workers-zero. A successful
assembly build supplies no READ/WORLD authority or Save/Restore result.
