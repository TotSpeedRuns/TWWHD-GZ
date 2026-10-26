# Location warp tables and Hyrule browser

This is an authored table-generator component and a factual destination list,
not a complete current-release build export. The repository's complete
reproducible runtime remains **v126**. No game executable, artwork, room data,
raw capture, native process identifier or compiled object is included here.

## Reference and attribution

Destination names and the initial stage/entrance suggestions came from
[Teotia444/WWHD-Hacker, `aroma.yaml`](https://github.com/Teotia444/WWHD-Hacker/blob/main/WWHDHacker/Resources/aroma.yaml),
Git blob `3aa4fc3a192e51c78c2ec9cf992d11e890202c96` (6,990 bytes).
Its original C# `ListTeleportHandler` parses room numbers as decimal and
spawn/layer values as hexadecimal. A stage-only line inherits mutable values
from that application's UI; it is not a complete entrance tuple. Both source
lines named Ghost Ship are retained as separate factual records.

`reference-plan.json` records all 174 source lines and their location groups.
`decisions.json` provides normalized selections and short explanations:
76 match existing records, 86 are supported additions/selections, nine are
excluded non-gameplay or explicitly unused scenes, and three are contextual.
There are 14 corrections to supplied explicit tuples and three corrections
to old authored warp records. These facts were checked privately against an
owned game's entrance data. That game data and the private evidence reports
are excluded; this export does not itself prove every destination's gameplay.

Authored generator/build logic is by the TWWHD-GZ project. The upstream YAML
and C# implementation are not vendored. Normalized functional names and
tuples are retained with attribution rather than copying game resources.

## Behavior and factual inputs

`normalized-tables.json` is a readable expected table result: 396 physical
records, 220 unique tuples, 218 chart destinations, 63 folders and 21 entries
in the new Hyrule text group. Physical aliases preserve the existing table
layout and IDs. Visible chart/text lists deduplicate by the actual tuple.

The generator retains the first 136 IDs and flags, with only these repairs:

* ID 78 becomes `TF_01`, room 0, spawn 0, layer FF, labelled Triforce Cave.
  This is the cave entrance, not a claimed chest landing; it aliases ID 79.
* ID 101 becomes `M_Dai`, room 7, spawn 08, layer FF.
* ID 105 becomes `kindan`, room 12, spawn 00, layer FF.

Stage-global entrance points use their authored room. Room-local points use
their containing room, even if reused records carry a different room value.
The ordinary Savage Labyrinth exit chain supports the supplied decimal room
15 floor-10/20/40 selections and room-20 floor-30 selection; no silent hex
reinterpretation is used. The original tuples remain in `decisions.json`.

Hyrule is a third text-browser root. Interiors are grouped with their named
entrance locations; the mobile Ghost Ship remains in the text Dungeons list
without an invented fixed chart sector. These are raw warp destinations.
The existing 803 route/memfile payloads are not expanded or replaced.

## Source and build compatibility

`build_candidate.py` and `compile_candidate.py` preserve the exact authored
historical driver logic, with UTF-8/LF normalization only. Their local
workspace paths and revision names are part of that recorded dependency
contract, not a portable replacement for the full project build.

The drivers require the private qualified R755 Personal assembly base and
its exact build report, bottle/Inventory-stick/heart-piece objects, native
aliases, devkitPPC tools and the RPX parser. The base assembly SHA256 is
`6664b1a9f6bca42dc80192600f1004fb6a5e6dc0c64da1edfa98e72cfa27f6b2`.
Those full private build inputs are not supplied by this component snapshot.
Do not substitute the repository's v126 template or an older maintained C
snapshot and claim the resulting image reproduces the current release.

For an already qualified private workspace, place the two drivers and these
clean `reference-plan.json`/`decisions.json` files in a new isolated
`work/teotia-warps-20261026` directory under that workspace. The original
private base is expected at `work/quarter-heart-pieces-candidate-20261026`.
The historical driver refuses an existing `staging.json` or build report.

```sh
python build_candidate.py --decisions decisions.json
python compile_candidate.py
```

The historical compiler first reproduces the exact R755 baseline. The
reviewed result changes two existing TEXT immediates (warp and browser-root
counts), adds 8,864 bytes of DATA, and retains BSS, route data, imports and
hooks. Personal/Public differ only in the Creator Code default word. The
recorded component hashes are in `source-pins.json`; no objects ship here.

Generated full assembly, object files and local staging/build reports remain
private outputs. They are not source-publication inputs. The clean factual
evidence strings are explanatory metadata, not an issued game/source lease.

The factual inputs can be checked without any game, compiler or private base:

```sh
python validate_facts.py
```

That check validates data consistency only. Source/object checks, native warp
behavior, installer use and Wii U acceptance are different scopes. Consult
the selected release's notes for actual runtime testing; this component does
not establish universal map safety, save-state restoration or durability.
