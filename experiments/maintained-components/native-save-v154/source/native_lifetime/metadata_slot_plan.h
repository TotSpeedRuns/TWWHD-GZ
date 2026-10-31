#ifndef GZ_METADATA_SLOT_PLAN_H
#define GZ_METADATA_SLOT_PLAN_H

typedef unsigned int HmpU;
enum {
    HMP_OK = 0, HMP_ARGUMENT = 1, HMP_SPACE = 2, HMP_FORMAT = 3,
    HMP_EXHAUSTED = 4, HMP_CHANGED = 5, HMP_SCOPE = 6,
    HMP_RT_OK = 7, HMP_RT_RECOVERED = 8, HMP_RT_DIRTY = 9, HMP_CPU_COMMITTED = 10,
    HMP_SLOT_PROTECTED = 0, HMP_SLOT_REUSABLE = 1, HMP_SLOT_CURRENT = 2
};

typedef struct {
    HmpU slots, current_count, saved_count, serial, stride, capacity;
} HmpSelection;

/* One-based private table index and saved ordinal. A zero saved ordinal
 * retires a CURRENT-only row. Generation proposals are not issued tokens.
 * Neither an ordinal nor an old snapshot index supplies native authority.
 */
typedef struct { HmpU index, source, generation; } HmpSlot;
typedef char HmpSlot12[sizeof(HmpSlot) == 12 ? 1 : -1];

/* Pure selection over an explicitly classified private table. CURRENT rows
 * are reused before inactive rows; live outside owners are always protected.
 * All input/space/exhaustion checks finish before the first plan-row store.
 * This function never reads a native actor, heap, allocator or process.
 */
HmpU _gz_hmp_slots(const HmpSelection *selection, const unsigned char *classes,
                   void *output, HmpU *written);
/* Preserve a genuinely CURRENT root's table slot/generation. This only
 * permutes already selected private indices; it cannot add a new index.
 */
HmpU _gz_hmp_pin(const HmpSelection *selection, void *output, HmpU count,
                 HmpU source_ordinal, HmpU current_index, HmpU current_generation);

/* Genuine GHW/OIS held providers, implemented in the actual lifetime TU.
 * These capture a complete before-image and prepare generic adoption slots.
 * They do not yet mutate or adopt the lifetime/process tables.
 */
HmpU _gz_hmp_plan_held(HmpU request, HmpU frame);
HmpU _gz_hmp_verify_held(HmpU request, HmpU frame);
/* Actual table apply/verify/inverse under both locks; never publishes
 * a historical world. Permission cursors remain monotonic on inverse. */
HmpU _gz_hmp_roundtrip_held(HmpU request, HmpU frame);
/* Private same-scene CPU-retention experiment only. All original
 * owner/shape checks and failure inverse remain. Missing255 stays. */
HmpU _gz_hmp_commit_held(HmpU request, HmpU frame);

#endif
