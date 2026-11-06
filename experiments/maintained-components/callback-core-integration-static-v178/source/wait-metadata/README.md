# OSWaitCond metadata successor

This separate, non-installable ELF-format artifact adds one genuine named SDK
function import to the static initializer candidate `2eadd54f…`. It preserves
that parent's existing code and four entry changes exactly. It has not been
loaded, installed, or executed.

The current callback Source defines `OSWaitCond` as FUNC18 at offset 8256 in
the genuine 8264-byte `.fimport_coreinit`. The Native parent contains the exact
same import payload and an unused eight-byte slot at `0xc000a8c0`. The new
symbol is Native table 28, index 5025, name offset 9983, size 0, FUNC18,
`st_other=0`, section 25. Its record names that genuine slot and adds no fake
function body. Prior `source711-sdk-binding-r1/report.json` retains the actual
named coreinit export and PPC32 ABI evidence. The saved official WUT generator
at commit `b22de1a3c0c8950c4ad4b4df170c89c05d3196aa` documents the global function
symbol and two zero words used by function imports.

The parent symbol table has 48 zero bytes available after its 5025 records;
the additional 16-byte record fits there. The old string table has only one
byte before the next stored section, so its new 9994-byte payload is explicitly
moved to the old EOF, offset 30,174,912. The old string bytes remain intact as
unreferenced file bytes. The new file has 54 trailing zero bytes and totals
30,184,960 bytes. All other section file offsets stay fixed.

The enlarged SYM runtime span shifts SHSTR from `0xc001e690` to `0xc001e6a0`.
FileInfo `loadSize` rises minimally from `0x1e984` to `0x1e994`, using the actual
136-byte pad and four-byte load alignment. All metadata and real CODE/DATA/BSS
spans remain disjoint. CRC records 28, 29 and 50 change; every other CRC remains
exact. FileInfo TEXT, DATA, temporary, trampoline, pad and flag fields remain
exact. The serializer does not invoke the encoder or its automatic 4096-byte
TEXT adjustment.

`materialize.py` independently checks exact parent hashes, stored/compressed
payload extents, both no-op roundtrips, all output CRCs, all 5025 old symbol
records and typed relocations, all twelve real Native spans, old string bytes,
and an explicit raw edit recipe. It changed 28 existing bytes and appended
10,048 bytes. `diff.json` gives the complete existing-byte runs, header fields,
appended string layout, loader bound and unchanged payload hashes.

This is metadata serialization evidence. It grants no SDK scheduler, held-lock,
module, cold-object lifetime, callback, read/save/lifetime service, or hardware
loader authority. It adds no call rows or trampoline budget. Further callback
integration must separately account for its real SDK calls and physical owner
projection. Existing stopped runtime suites remain stopped. No console,
network, Source, repo, Setup, release version, or continuation checkpoint changed.
