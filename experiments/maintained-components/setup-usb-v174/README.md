This is the authored source snapshot for the selected experimental Setup174 USB correction. Setup174 retains the experimental Game173 payload. It does not claim a hardware Save/Restore or USB installation result.

The component builds from the included C, assembly, linker scripts and hash catalog. It adds 5242 immutable CODE/RO bytes, zero mutable bytes, no imports and no per-frame work. The export contains no game, original Setup executable, recipes, SDK, compiled binary, captured memory or private directory dependency.

The device selector examines the complete successful packed MCP title list. It selects the unique canonical practice registration on the requested device independent of enumeration order. A registration only on the other device, duplicates on either device, a malformed practice path or invalid selection refuses before mount/hash/file changes. The four adapted callers use their genuine Native choice/volume/CR4 context. The other three global/source callers retain their original behavior. Native MCP allocation, copying, free/close and error handling remain in place.

The historical catalog recognizes exact RPX hashes only in RPX predicates and exact title hashes only in the title classifier. It contains 17 authenticated artifact records: Public154,155–164,167–172. No distinct Game165/166 identity is invented. Existing Native transaction rules permit some mixed known versions; this component preserves that policy and does not impose a same-version pair requirement. Original current-only final checks, icon checks, stage/backup restrictions and error paths remain authoritative.

Run the standalone component builder from the repository root:

```text
python -B tools/build_setup174_components.py --compiler /path/to/devkitPPC/bin/powerpc-eabi-gcc --output /fresh/path/setup174-objects
```

On Windows, use `powerpc-eabi-gcc.exe`. The output's parent must exist and the output directory must be fresh and outside this source tree. Python 3.10+ and a PowerPC EABI compiler/linker are required. The reviewed toolchain is devkitPPC r49. The builder accepts another toolchain only when every resulting object still matches the exact selected hashes; it records whether the tool binaries match the reference. It requires no WUT/SDK headers. It validates source hashes, refuses precompiled source headers/symlinks and never invokes a shell or changes an input.

The builder produces eight authored ET_REL objects and a build report. It checks the exact selected merged hash `e8386e5f40584ff9b3ee8f6f8610fed70cce68f945c5ef835614c88559dfc2d8`, immutable extent, all 13 unresolved genuine Native interfaces, all relocation kinds/counts and the 34 CODE-owned catalog pointers. Other compiler/platform combinations may differ and need their own explicit qualification; differing output is refused.

This is a complete component-source build, not a complete Setup image build. `bindings.json` specifies the five exact Native173 hooks, CODE/DATA bindings, obsolete HA-row removal and retained LO row, revision metadata and packing requirements. It is restricted to the exact parent Setup173 hash. A separate integrator must validate the owned parent, resolve typed loader rows, regenerate RPX FileInfo/CRC/compression, preserve Native code/exception metadata and rebuild WUHB/profile resources. Those image/recipe/resource inputs and that integrator are deliberately outside this export. Numeric addresses do not supply a caller lifetime or authorize an arbitrary-version patch.

The selected final Setup174 root hash is `9ec93695abe1848d7fb5ebeda0ac88d34744616f7a130499ff30f408b609280e`; its WUHB hash is `543c1d1e7642a793f7ddb819c1404f10f9d731f418a4abe251c5a6c73ac95754`. The 25 printable revision bytes are `v174 USB Test / Game v173`, followed by the preserved Native terminator. The component builder does not produce either final image or modify a console.

Independent CPU checks covered the actual serialized final image with real typed RPL rows and no instruction overlays: 1500 role-predicate pairs,530 Native state pairs,60 helper guards and360 register-context pairs at five CODE/DATA biases. The separate packed lookup checks covered all seven callers, both storage choices, negative paths and relocation sign boundaries. These checks use explicit post-hash/caller fixtures and bounded SDK endpoints where stated; they do not execute physical USB media, a full install, C++ unwinding or Wii U hardware.
