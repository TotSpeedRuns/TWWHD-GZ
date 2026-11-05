# Offline fixed175 journal explanation

Run with Python3.10+ against a local collector JSONL file:

```powershell
python -B explain_fixed175.py C:/path/to/fixed175.jsonl
```

The tool reads at most128KiB, prints JSON to stdout and makes no connection.
It imports neither the collector nor any target/runtime code. It accepts only
the fixed175 collector schema and target hash
`5915c5d4b7a37d2e80d49272000747c3003c0bd88ae453f64583f1e86f780a90`.
This expected target is an input-journal claim, not offline authentication of
the running executable. Setup175 labels alone do not establish Game175 identity.

`layout.json` contains the exact24 names/preferred addresses/byte lengths,
totalling2124 metadata bytes per sample. It is data, pinned by SHA256 in the
explainer. `prepare_layout.py` extracted it from pinned JSON/source without
importing or running the collector. Its `source_pins` records every authority
hash: collector/prepare dependencies, source result descriptions, HL flags,
HX ABI, trace enum/decoder and the authenticated175 RNS72 layout receipt.
Only trusted copied source labels are used. Raw codes/detail fields are kept.

## Validation and partial journals

The complete artifact is validated before any explanation is emitted. The
parser rejects changed target/schema/layout, duplicate keys, unknown fields,
non-uint32 or boolean words, wrong record lengths, unsafe sample flags,
nonsequential indexes, samples beyond the1..8 requested bound, final-event
ordering mistakes, and completion counters outside400 reads/32768 bytes or
below the recorded sample minima. At most10 events are accepted. Completion
requires all requested samples; counter bounds do not authenticate actual I/O.

A valid journal ending after a complete JSON event retains its samples as
`incomplete_EOF`. A `stopped` event retains preceding valid samples as
`incomplete_stopped`, including zero samples when connection failed before
the first sample. A truncated/malformed JSON line rejects the entire file;
the CLI does not print a partial explanation. UNC/URL inputs are refused.
These name checks and final-symlink refusal do not prove that a parent directory
or mapped drive has no filesystem redirection/network backing. The program
itself opens no socket or live service; choose a known local journal path.

## Interpretation limits

GHS exposes retained raw coordinator request/frame/phase/result fields. Zero
never establishes success, including a terminal-looking Save record. A complete
collection is not a completed Save/Restore. Source result descriptions remain
record-code descriptions; no displayed toast is claimed to have been observed.
There is no GHW in this24-record readset, so H is explicitly unavailable.

GHC is attributed only when its supported header and nonzero request/frame
match the supported GHS record. With word29 zero, tags0x7100..0x7103 are shown
as unpublished advisory raw tags/status. With word29 nonzero, words27/28 are
shown as generation/type, never trace tags. Word31 stays opaque detail; it is
not universally a byte count and can contain a narrow-path SP tag.

The authenticated RNS72 layout keeps the original64-byte prefix at offset0:
trace words12..15 are followed by separate config words16/17. Trace attribution
requires nonzero pending request/frame, matching nonzero GHS request/frame,
and trace-frame equality. Stale/mismatched trace labels are ignored. Matching
step/stage names are advisory; raw result and bad/detail stay uninterpreted.
Unknown/torn steps remain raw. Sequential matching words do not prove freshness.

HL flags are sticky observations, not immediate causes or ownership evidence.
HX occupied counts historical nonzero keys, including non-live rows; it is not
a live allocation count. Other fixed records are retained raw. No word value
selects a further file/address read, and no pointer is followed. Nothing here
establishes atomicity, a hold/lease, payload integrity, physical lifetime,
actual menu/SaveRestore correctness or loading performance.

Run `python -B test_explain_fixed175.py` for the host-only tests. Tests cover
stale traces/GHC, zero and nonterminal results, published-field interpretation,
strict target/layout/shape/bool/bounds checks, stopped/EOF/truncated artifacts,
pointer poison without I/O, local-file restriction and CLI rejection before
printing a partial result. No console or target instruction is executed.

Root source review is complete. Its high-bit trace-family recognition correction
is covered by explicit0x12203/0x13203 regressions; all29 tests pass. The frozen
qualification binds this source, layout and provenance. Root's separate host
review reconstructs the readset independently and supplies its own receipt.
