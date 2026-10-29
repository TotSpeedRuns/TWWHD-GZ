# Experimental quiet HEART item retirement

This partial source experiment extends the existing current-item lifetime
consumer from quiet native `ItemNo 1` to the finite quiet `ItemNo 0/1` cohort.
Two bounded Cemu cases passed: one naturally spawned HEART (`0`) and a separate
control with two naturally spawned green rupees (`1`). A mixed-cohort attempt
was inconclusive before Restore. These results do not qualify Wii U, active
effects, arbitrary actors, or full-game save states.

The complete reproducible integrated repository export remains `v126`. These
files are a partial source experiment, not a standalone builder for the current
runtime or a released console update. The baseline is byte-exact to the
previously exported current-item lifetime consumer. The candidate keeps that
source's formatting. The patch changes the consumer only; it retains the native
writer, dispatcher, typed world participants, and their original source, heap,
and job ownership providers.

The private intent gains eight `ItemNo` bytes, one for each of at most eight
current-only actors. Its internal version becomes `2`, and its size changes
from `2320` to `2328` bytes. The seal covers `2324` bytes, including the exact
`ItemNo` tuple. Unused tuple bytes must remain zero. The public `128`-byte
report ABI is unchanged. Accepting `0/1` as a set does not permit a retained
actor to change between those two cases.

Before the newly added `ItemNo` read, the actor must have the exact current
profile `255`, process `255`, `0x7f0` allocation, and a valid full extent. The
existing LIVE process, allocation, SOLID heap, complete cohort, original
transaction interval, and native shape checks remain. The selected archive row
`0` or `1` must still identify `Always`. The original four optional emitter and
follow fields must all remain zero. Active effects are not admitted by this
extension.

The exact `ItemNo` is compared again under the original per-frame native writer
before the existing native delete request is issued once. An eight-byte private
snapshot is compared after the request returns. From that return onward, the
scheduled actor's body, callback, heap, and emitter pointers are not followed.
Progress uses the existing process, collider, and SOLID heap completion receipts
for the exact same generation. The original owner, source, frame, queue, and
timeout guards remain. Unknown or changed receipts stop the experiment rather
than clear it.

## Bounded native results

Both passed cases used the ordinary controller UI to capture a new SOURCE with
Save status `14`, then natural grass cuts to create the current-only items.
The first ordinary Restore returned UI refusal `28` with internal CLEAN result
`40`, after completing the typed inverse and arming a finite retirement intent.
The original native schedule and callback path completed retirement. A later,
separate ordinary manual Restore reacquired CURRENT and committed with status
`15`. Retirement completion alone did not restore the saved world. This is a
two-request flow with a manual retry, not one-action Restore or automatic retry.

| Cemu case | Natural current-only cohort | Observed result |
| --- | --- | --- |
| HEART | One `ItemNo 0` actor and one collider | Inverse/CLEAN `40`, native retirement, then separate manual Restore `15` |
| Green control | Two `ItemNo 1` actors and two colliders | Inverse/CLEAN `40`, native retirement, then separate manual Restore `15` |
| Mixed attempt | No complete mixed cohort qualified | SOURCE Save `14` succeeded; after two cuts, the current-item observer check did not stabilize, before any Restore |

The passed cases checked the sealed `ItemNo` tuple, the exact original process,
collider, and SOLID heap generations, the actual native schedule return, and
released writer ownership. All `11` checked private cleanup blocks were zero,
and owner, heap gate, and job holds were released. The saved health value was
restored. Six original and disposable save files retained their exact hashes.
The executed drivers recorded writes only to the three ordinary controller
input spans. No host actor writes, supplied native results, or SDK budget
fixture was used.

Ordinary Restore confirmation release remained enabled. Its before/after host
observations are non-atomic, and retirement could begin during that release.
There is no claim that the entire pending native interval received zero input.
The HEART case also had a later read-only check of all `1500` restored grass
rows' `64`-byte numeric spans against SOURCE; those spans matched. Current live
linkage words were retained rather than copied from historical pointers.

The mixed attempt establishes neither mixed success nor a collection or
lifecycle failure. It never reached Restore or a qualified lifetime intent.
It needs a new, naturally formed and stable mixed cohort before that case can
be assessed.

The independent review fingerprints are:

- HEART finite outcome review:
  `9dc92dbeb9cf5aff577b4e7af4b81c6c37dcd9dbacbf312bd5204b6aadb048c4`.
- Green outcome and mixed-scope review:
  `9eeb7c71017b520d8474b737dfcf5fdae0cf3715b11356ba974e22de109fe7f7`.

## Inputs and limitations

The exact inputs reviewed privately are recorded as SHA-256 fingerprints only:

- Parent BOMB object:
  `9ca3ce7ccd2d3c378c74e2e778ffb80e689f650801b6cc870e2eb7580190bd0a`.
- Candidate BOMB object:
  `ef2a6eaa491afa030c3914dd1a8c77f665ed95d97be4cd83b8c4592d54db3e7b`.
- Integrated private image:
  `b7bc7bf89b937cef19ae69826a14f1e44a29d166b1ca6e324474943115feb51d`.

The parent whole object was reproduced before the candidate. Emitted review
retained initialized DATA and rodata, all other BSS, imports and exports, and
the existing cleanup call. Only private LRR BSS grows by eight bytes. The
integrated private main retained `79` other roles and `803` checkpoint
payloads. These static checks and the two finite Cemu cases do not establish
general native reliability or Wii U coverage.

Unsupported cases include active item effects, arbitrary profiles and
resources, broad actor reconstruction, missing ownership providers, cross-area
or general cutscene replay, device and particle closure, and full-game save
states. This source retains the existing smoke and effect coverage refusal.
The parent clean-abort cleanup change is separate and is retained by the
private candidate lineage. No general actor deletion grant is added.

The five source files here contain authored C include files, their exact patch,
and readable metadata only. They include no compiled objects, game executables,
artwork, fonts, resources, SDK binaries, saves, captured state, personal
profiles, host memory clients, or native RAM address dumps. Native
anchor-relative constants retained in the authored guard code are not captured
actor addresses. The repository's root MIT license applies to this authored
source.

Before publication, the maintainer must retain these scoped results and
limitations, verify the explicit file hashes, and run the repository audit and
portable publication tests. The first inverse and refusal, retirement interval,
and separate manual Restore must remain distinct in any result description.
