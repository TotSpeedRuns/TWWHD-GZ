/* Included by the real OIS owner after its original allocation/pin helpers.
 * All allocations still use the original fixed Cafe SDK allocator. Reserve
 * and final release occur outside the producer's genuine native read hold. */
#include "owned_heap_native.h"
#include "owned_heap_cut_policy.h"
enum {
    OIS_CUT_EMPTY = 0,
    OIS_CUT_RESERVED = 1,
    OIS_CUT_PUBLISHED = 2,
    OIS_CUT_LEASED = 3,
    OIS_CUT_RETIRING = 4,
    OIS_CUT_RETAINED = 5
};

typedef struct {
    U phase, request, frame, thread, slot, used, generation, token;
    OisLease saved;
    Region current, work;
    OisLease reader;
} OisCurrentCut;
typedef char OisCurrentCut128[sizeof(OisCurrentCut) == 128 ? 1 : -1];
static OisCurrentCut ois_cut;

static U ois_cut_pending(void)
{
    return ois_cut.phase != OIS_CUT_EMPTY;
}

static const Region *ois_cut_region(void)
{
    return &ois_cut.current;
}

static const Region *ois_cut_work_region(void)
{
    return &ois_cut.work;
}

static U ois_cut_zero(const void *pointer, U bytes)
{
    const B *p = pointer;
    U i;
    for (i = 0; i < bytes; ++i) {
        if (p[i]) return 0;
    }
    return 1;
}

static U ois_cut_external(U pointer, U bytes)
{
    return apart(pointer, bytes, (U)&_gz_ois_held_record, sizeof(_gz_ois_held_record)) &&
        apart(pointer, bytes, (U)&ois_cut, sizeof(ois_cut)) &&
        (!ois_cut.current.pointer || apart(pointer, bytes,
            ois_cut.current.pointer, ois_cut.current.capacity)) &&
        (!ois_cut.work.pointer || apart(pointer, bytes,
            ois_cut.work.pointer, ois_cut.work.capacity));
}

static U ois_cut_protected_pin(const OisLease *lease)
{
    return ois_cut.phase != OIS_CUT_EMPTY &&
        same(lease, &ois_cut.saved, sizeof(*lease));
}

static U ois_cut_reader_valid(void)
{
    const OisLease *l = &ois_cut.reader;
    return l->allocation == ois_cut.current.pointer &&
        l->archive == ois_cut.current.pointer &&
        l->capacity == ois_cut.current.capacity &&
        l->bytes == ois_cut.used &&
        l->identity == ois_cut.current.identity &&
        l->generation == ois_cut.generation && l->pin && !l->reserved;
}

static U ois_cut_header(void)
{
    U i;
    /* The owner uses the same original clear-before-free SDK sequence.
     * During that serialized window no read/write acquisition is allowed,
     * and physical_apart still refuses every caller while entered. */
    if (ois_cut.phase == OIS_CUT_RETIRING) {
        return state.entered == 1 && state.thread;
    }
    if (ois_cut.phase == OIS_CUT_EMPTY) {
        /* The monotonic current generation survives ordinary retirement. */
        return !ois_cut.request && !ois_cut.frame && !ois_cut.thread &&
            !ois_cut.slot && !ois_cut.used && !ois_cut.token &&
            ois_cut_zero(&ois_cut.saved, sizeof(ois_cut.saved)) &&
            ois_cut_zero(&ois_cut.current, sizeof(ois_cut.current)) &&
            ois_cut_zero(&ois_cut.work, sizeof(ois_cut.work)) &&
            ois_cut_zero(&ois_cut.reader, sizeof(ois_cut.reader));
    }
    if (ois_cut.phase > OIS_CUT_LEASED || !ois_cut.request || !ois_cut.frame ||
        !ois_cut.thread || ois_cut.slot >= OIS_SLOTS ||
        state.tx.phase ||
        !pin_valid(&ois_cut.saved) ||
        state.slots[ois_cut.slot].type != GHC_PROCESS_VERSION ||
        state.slots[ois_cut.slot].region.pointer != ois_cut.saved.archive) {
        return 0;
    }
    if (ois_cut.phase == OIS_CUT_RESERVED) {
        return !ois_cut.used && !ois_cut.token &&
            ois_cut_zero(&ois_cut.current, sizeof(ois_cut.current)) &&
            ois_cut_zero(&ois_cut.work, sizeof(ois_cut.work)) &&
            ois_cut_zero(&ois_cut.reader, sizeof(ois_cut.reader));
    }
    if (!region_ok(&ois_cut.current) || !ois_cut.current.pointer ||
        !ois_cut.generation || !ois_cut.token || ois_cut.used < 192 ||
        ois_cut.used > ois_cut.current.capacity || (ois_cut.used & 3) ||
        state.staging.pointer ||
        !apart(ois_cut.current.pointer, ois_cut.current.capacity,
            (U)&state, sizeof(state)) ||
        !apart(ois_cut.current.pointer, ois_cut.current.capacity,
            (U)&ois_cut, sizeof(ois_cut))) {
        return 0;
    }
    for (i = 0; i < OIS_SLOTS; ++i) {
        if (!region_ok(&state.slots[i].region)) return 0;
        if (state.slots[i].region.pointer &&
            !apart(ois_cut.current.pointer, ois_cut.current.capacity,
                state.slots[i].region.pointer, state.slots[i].region.capacity)) return 0;
    }
    if (!region_ok(&ois_cut.work)) return 0;
    if (ois_cut.work.pointer) {
        if (ois_cut.phase != OIS_CUT_LEASED || ois_cut.work.capacity > OIS_CUT_WORK_MAX ||
            !apart(ois_cut.work.pointer, ois_cut.work.capacity,
                ois_cut.current.pointer, ois_cut.current.capacity) ||
            !apart(ois_cut.work.pointer, ois_cut.work.capacity, (U)&state, sizeof(state)) ||
            !apart(ois_cut.work.pointer, ois_cut.work.capacity, (U)&ois_cut, sizeof(ois_cut))) return 0;
        for (i = 0; i < OIS_SLOTS; ++i) {
            if (state.slots[i].region.pointer &&
                !apart(ois_cut.work.pointer, ois_cut.work.capacity,
                    state.slots[i].region.pointer, state.slots[i].region.capacity)) return 0;
        }
    }
    return ois_cut.phase == OIS_CUT_LEASED ? ois_cut_reader_valid() :
        ois_cut_zero(&ois_cut.reader, sizeof(ois_cut.reader));
}

static void ois_cut_clear(void)
{
    U generation = ois_cut.generation;
    zero(&ois_cut, sizeof(ois_cut));
    ois_cut.generation = generation;
}

/* Only this private owner releases its reserved saved pin. Exposed old OIS
 * read-release calls are refused for this exact protected pin. */
static U ois_cut_release_saved(void)
{
    U i = pin_find(&ois_cut.saved);
    Pin *pin;
    Slot *slot;
    if (i == OIS_PINS || !pin_valid(&ois_cut.saved)) return OIS_CORRUPT;
    pin = state.pins + i;
    if (pin->slot != ois_cut.slot || pin->slot >= OIS_SLOTS) return OIS_CORRUPT;
    slot = state.slots + pin->slot;
    if (!slot->readers) return OIS_CORRUPT;
    --slot->readers;
    zero(pin, sizeof(*pin));
    return OIS_OK;
}

static U ois_cut_begin(U request, U frame, U target)
{
    U result;
    const GhcSaveProducer *saved_producer;
    if (ois_cut_pending() || target >= OIS_SLOTS || !slot_ok(target) ||
        state.slots[target].type != GHC_PROCESS_VERSION) return OIS_TYPE;
    if (ois_cut.generation == 0xffffffffu) return OIS_EXHAUSTED;
    result = pin_acquire(target, &ois_cut.saved);
    if (result) return result;
    ois_cut.request = request;
    ois_cut.frame = frame;
    ois_cut.thread = state.thread;
    ois_cut.slot = target;
    ois_cut.phase = OIS_CUT_RESERVED;
    result = _gz_ghc_validate((const void *)ois_cut.saved.archive,
        ois_cut.saved.bytes, 1);
    if (!result) {
        saved_producer = (const GhcSaveProducer *)(ois_cut.saved.archive + 128u);
        /* A saved user cut came from a genuine Save, never a current cut.
         * Legacy HSPv1 was Save-only. All new HSPv2 cuts carry command1/2. */
        if (!ois_cut_saved_policy(saved_producer, request, frame, target))
            result = GHC_CHANGED;
    }
    if (!result) return OIS_OK;
    ois_cut.phase = OIS_CUT_RETIRING;
    if (ois_cut_release_saved()) {
        ois_cut.phase = OIS_CUT_RETAINED;
        return GHC_RETAINED;
    }
    ois_cut_clear();
    return result;
}

static U ois_cut_abort(void)
{
    U result;
    if (ois_cut.phase != OIS_CUT_RESERVED) return GHC_RETAINED;
    ois_cut.phase = OIS_CUT_RETIRING;
    result = ois_cut_release_saved();
    if (result) {
        ois_cut.phase = OIS_CUT_RETAINED;
        return GHC_RETAINED;
    }
    ois_cut_clear();
    return OIS_OK;
}

static U ois_cut_publish(U request, U frame, U used)
{
    U token, result;
    if (ois_cut.phase != OIS_CUT_RESERVED || ois_cut.request != request ||
        ois_cut.frame != frame || ois_cut.thread != state.thread ||
        !ois_cut_header() || !region_ok(&state.staging) || !state.staging.pointer ||
        used < 192 || used > state.staging.capacity || (used & 3) ||
        ois_cut.generation == 0xffffffffu) return GHC_RETAINED;
    result = issue(&token);
    if (result) return result;
    copy(&ois_cut.current, &state.staging, sizeof(Region));
    zero(&state.staging, sizeof(state.staging));
    ois_cut.used = used;
    ++ois_cut.generation;
    ois_cut.token = token;
    __sync_synchronize();
    ois_cut.phase = OIS_CUT_PUBLISHED;
    return OIS_OK;
}

static U ois_cut_lock(void)
{
    if (ois_heap_pending()) return OIS_BUSY;
    if (!header()) return OIS_CORRUPT;
    if (!__sync_bool_compare_and_swap(&state.entered, 0, 1)) return OIS_BUSY;
    state.thread = (U)OSGetCurrentThread();
    if (!state.thread) {
        state.entered = 0;
        return OIS_CORRUPT;
    }
    return OIS_OK;
}

static void ois_cut_describe(OisCutPair *pair)
{
    zero(pair, sizeof(*pair));
    pair->magic = OIS_CUT_MAGIC;
    pair->bytes = sizeof(*pair);
    pair->version = OIS_CUT_VERSION;
    pair->valid = 1;
    pair->request = ois_cut.request;
    pair->frame = ois_cut.frame;
    pair->generation = ois_cut.generation;
    pair->token = ois_cut.token;
    pair->missing = OIS_CUT_MISSING_ALL;
    pair->slot = ois_cut.slot;
    copy(&pair->saved, &ois_cut.saved, sizeof(pair->saved));
    copy(&pair->current, &ois_cut.reader, sizeof(pair->current));
}

static U ois_cut_pair_matches(const OisCutPair *pair)
{
    OisCutPair expected;
    if (ois_cut.phase != OIS_CUT_LEASED || !ois_cut_header()) return 0;
    ois_cut_describe(&expected);
    return same(pair, &expected, sizeof(expected));
}

U _gz_ois_cut_occupied(U target)
{
    /* Nonzero keeps the generic route; a corrupt/busy owner must not fall
     * through to unrelated old GZS Restore. This hint grants no authority. */
    if (target >= OIS_SLOTS || !header() || state.entered ||
        ois_heap_pending() || ois_cut_pending() || !slot_ok(target)) return 2;
    return state.slots[target].region.pointer &&
        state.slots[target].type == GHC_PROCESS_VERSION ? 1u : 0u;
}

U _gz_ois_cut_acquire(U request, U frame, OisCutPair *output)
{
    U result = ois_cut_lock();
    if (result) return result;
    if (!external((U)output, sizeof(*output))) return done(OIS_ARGUMENT);
    if (ois_cut.phase != OIS_CUT_PUBLISHED || ois_cut.request != request ||
        ois_cut.frame != frame) return done(OIS_CHANGED);
    result = issue(&ois_cut.reader.pin);
    if (result) return done(result);
    ois_cut.reader.allocation = ois_cut.reader.archive = ois_cut.current.pointer;
    ois_cut.reader.capacity = ois_cut.current.capacity;
    ois_cut.reader.bytes = ois_cut.used;
    ois_cut.reader.identity = ois_cut.current.identity;
    ois_cut.reader.generation = ois_cut.generation;
    __sync_synchronize();
    ois_cut.phase = OIS_CUT_LEASED;
    ois_cut_describe(output);
    return done(OIS_OK);
}

U _gz_ois_cut_validate(const OisCutPair *input)
{
    OisCutPair pair;
    U result = ois_cut_lock();
    if (result) return result;
    if (!external((U)input, sizeof(*input))) return done(OIS_ARGUMENT);
    copy(&pair, input, sizeof(pair));
    if (!ois_cut_pair_matches(&pair)) return done(OIS_CHANGED);
    result = _gz_ghc_validate((const void *)pair.saved.archive, pair.saved.bytes, 1);
    if (!result) result = _gz_ghc_validate((const void *)pair.current.archive,
        pair.current.bytes, 1);
    if (!result && !ois_cut_pair_matches(&pair)) result = OIS_CHANGED;
    return done(result);
}

U _gz_ois_cut_release(const OisCutPair *input)
{
    OisCutPair pair;
    U result = ois_cut_lock();
    if (result) return result;
    if (!external((U)input, sizeof(*input))) return done(OIS_ARGUMENT);
    copy(&pair, input, sizeof(pair));
    if (!ois_cut_pair_matches(&pair)) return done(OIS_CHANGED);
    ois_cut.phase = OIS_CUT_RETIRING;
    result = ois_cut_release_saved();
    if (result) {
        ois_cut.phase = OIS_CUT_RETAINED;
        return done(GHC_RETAINED);
    }
    /* Real exact saved pin is gone; the user archive remains unchanged.
     * Only the independently allocated CURRENT region is retired. */
    release_region(&ois_cut.work);
    release_region(&ois_cut.current);
    ois_cut_clear();
    return done(OIS_OK);
}

U _gz_ois_cut_drop(U request, U frame)
{
    U result = ois_cut_lock();
    if (result) return result;
    if (ois_cut.phase != OIS_CUT_PUBLISHED || ois_cut.request != request ||
        ois_cut.frame != frame) return done(OIS_CHANGED);
    ois_cut.phase = OIS_CUT_RETIRING;
    result = ois_cut_release_saved();
    if (result) {
        ois_cut.phase = OIS_CUT_RETAINED;
        return done(GHC_RETAINED);
    }
    release_region(&ois_cut.work);
    release_region(&ois_cut.current);
    ois_cut_clear();
    return done(OIS_OK);
}

#include "owned_heap_cut_work.inc"
