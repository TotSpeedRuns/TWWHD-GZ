# Runtime integration contract

The eight seams target the qualified R801 map source, SHA256
`0e7dc9e1380a061b7827b2eafc5f17d9e89cc0b00726752a0544775e843a96b4`.
That source inherits the normal R791/v145 UI lineage and the independently
qualified static map board. The complete private baseline and its inherited
link inputs are not included. These edits do not apply automatically to the
repository's integrated v126 template or to a later diagnostic branch.

Apply each exact JSON seam once to a compatible map runtime, assemble `hierarchy.s`
as a separate object, and link that object once with the existing runtime and
its unchanged modules. Do not also append `hierarchy.s` to the runtime source.
For example, the standalone module can be assembled with:

```sh
powerpc-eabi-gcc -mcpu=750 -c -x assembler hierarchy.s -o hierarchy.o
```

Unresolved existing label/controller symbols are expected at this stage.
Compiling this object does not produce a complete game image. The inherited
board module and assets remain separate dependencies and are not supplied.

| Existing caller state | Required meaning |
| --- | --- |
| `r29` | Existing menu-state base; cursor is the word at `r29 + 8` |
| `_gz_page` | 13 for the map grid, 14 for a destination/folder list |
| `_gz_map_sector` | Zero-based sector index, 0 through 48 |
| `_gz_mask` | Existing input-exit tail target |
| Existing label symbols | Every label referenced by the new row tables |

`_gz_mah_descriptor` returns the active descriptor pointer in `r12`. It uses
`r9`, `r12` and CR0 as volatile scratch and returns through LR. Invalid folder
state falls back to the sector root; invalid sector state is bounded to zero.

`_gz_mah_open_folder` expects a high-bit-tagged folder value in `r10` and the
existing state base in `r29`. It bounds the decoded ID below 12, records the
parent cursor, selects the first folder row and branches to `_gz_mask`.
It uses `r9`, `r10` and CR0. `_gz_mah_back` uses `r9` through `r11` and CR0;
it restores the parent row, returns from a sector list to the grid, or returns
from the grid to menu root. These two helpers are custom tail paths, not C ABI
functions. None changes `r29`, `r31`, SP or nonvolatile GPRs.

Each descriptor is three 32-bit words: title pointer, first row index, row
count. Each row is two words: label pointer and value. A clear high bit means
an unchanged original warp ID. A set high bit means `0x80000000 | folder_id`.
Only folder IDs 0 through 11 are valid. The selection seam intercepts that
tag before the original warp-choice/pending publication; a folder value must
never reach the warp dispatcher.

There are 49 root descriptors, 12 folder descriptors and 230 rows: 218 original
leaves plus 12 folder rows. Folder lists contain leaves only. New mutable state
is exactly eight initialized DATA bytes at `_gz_mah_state`: active folder
`0xffffffff` at the sector root and the parent cursor. No slot scratch, BSS,
heap allocation, native hook or import is added. The three helper bodies total
216 bytes; after the replaced instruction spans, net TEXT grows by 184 bytes,
RO by 2,956 bytes and DATA by eight bytes.

The eight edits replace the list descriptor lookup, distinguish folder/leaf
selection, reset folder state on sector entry and map open, implement parent
Back, draw the active area title, draw the new row table, and update the action
help text. Existing button binding, repeat, root access and drawing helpers
remain the caller's responsibility. The original empty-sector refusal remains.

Preserve the original warp records, old map/folder tables, Favorites IDs and
route payloads. Retain all inherited input/draw/instrumentation integrations
when rebasing to another runtime. Replacing a newer whole runtime with this
older baseline would discard those unrelated integrations.

The final linker/packer must emit and audit all label ADDR32 fields, HA/LO
address halves and branch ranges. In the tested builder, const input sections
are packed into nonexecutable CafeDATA with flags 3; this is not an MMU
read-only claim. All new tables and assets must stay inside the existing
private-image exclusion, and the unchanged 168-byte runtime BSS must still be
initialized. The tested final image retained 74 roles, 298 hooks, 803 routes,
241 native import sites and all 80 user-GQR corrections.

The formatted export changes comments/whitespace only. Its allocated section
bytes, symbol offsets and relocations were checked against the pinned private
module before staging. No published source or release artifact is modified by
this proposal.
