This is an unselected, source-only backup of the authored JWA/JXP provider with the reviewed terminal-publication fix. `_gz_jwa_init_open` publishes OPEN with conditional atomic updates to both its initialization state and diagnostic state, preserving an already published POISONED state. A successful return records those publications; it does not guarantee continuing readiness if another caller poisons the provider later.

Build the owned provider TU with Python 3.9 or newer, devkitPPC r49 and genuine WUT 1.3.0 headers. No SDK source is included. GCC's standard headers `stdint.h` and `stddef.h`, WUT's coreinit headers and their genuine transitive headers are external dependencies. Pass their roots explicitly:

```text
python tools/build_jwa_terminal_publication_source.py --devkitppc /path/to/devkitPPC --wut /path/to/wut --output ../jwa-terminal-build
```

The output directory must be new and outside the repository. The builder compiles `source/provider.c`, which includes the exact authored adapter and GHS bridge, preserving the original flags and basename. The reviewed output is the PowerPC ELF32 relocatable object with SHA-256 `a62ef15a4f584b2ca216d91645c1ae420cd05dfdf3bee299c61e06fe7b1d6f8a`. It contains the 352-byte initializer and retains 39 unresolved provider roles. The other owned helper sources are backed up alongside it; the builder does not silently combine them or supply stubs for missing roles.

The component adds 76 CODE bytes to its reviewed parent with no new storage, owners or imports. The 42 other provider functions retain their original code and typed expressions. Its source-level terminal-publication interleavings and static code generation were checked separately; the backup builder only reproduces the component.

This does not build a practice game, Setup app or complete Source712 module. That integration requires the other private owned source/integration inputs, real SDK bindings, startup and owner placement. No activation caller is provided. No Wii U behavior, B43 fix, crash fix or lag reduction is established by this component. The source manifest intentionally pins this reviewed snapshot; deliberate source edits require updating its manifest and expected build selection.
